//
// Created by ir0n1c on 11/14/2026.
//

#include "memory.h"
#include <Windows.h>
#include <tlhelp32.h>
#include <string>
#include <memory>

struct Process {
    HANDLE hProcess;
    HWND hwnd;
    uintptr_t baseAddress;
};

namespace nt {
namespace {
using NtReadVirtualMemory_t = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, ULONG, PULONG);
using NtWriteVirtualMemory_t = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, ULONG, PULONG);

// resolved once, on first call
NtReadVirtualMemory_t resolveRead() {
    static const auto fn = reinterpret_cast<NtReadVirtualMemory_t>(
        reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtReadVirtualMemory")));
    return fn;
}

NtWriteVirtualMemory_t resolveWrite() {
    static const auto fn = reinterpret_cast<NtWriteVirtualMemory_t>(
        reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtWriteVirtualMemory")));
    return fn;
}
} // namespace

NTSTATUS read(HANDLE h, uintptr_t addr, void* buf, size_t len, PULONG done) {
    const auto fn = resolveRead();
    return fn ? fn(h, reinterpret_cast<PVOID>(addr), buf, static_cast<ULONG>(len), done) : -1;
}

// The syscall, unlike WriteProcessMemory, will not lift page protection itself,
// so a failed write is retried once under RWX and the old protection restored.
NTSTATUS write(HANDLE h, uintptr_t addr, const void* buf, size_t len, PULONG done) {
    const auto fn = resolveWrite();
    if (!fn)
        return -1;

    const auto call = [&] {
        return fn(h, reinterpret_cast<PVOID>(addr), const_cast<PVOID>(buf), static_cast<ULONG>(len), done);
    };

    const NTSTATUS status = call();
    if (status >= 0)
        return status;

    DWORD old = 0;
    if (!VirtualProtectEx(h, reinterpret_cast<LPVOID>(addr), len, PAGE_EXECUTE_READWRITE, &old))
        return status;

    const NTSTATUS retried = call();
    DWORD ignored = 0;
    VirtualProtectEx(h, reinterpret_cast<LPVOID>(addr), len, old, &ignored);
    return retried;
}
} // namespace nt

namespace {
struct WindowSearch {
    DWORD pid;
    HWND hwnd;
};

BOOL CALLBACK FindMainWindow(HWND window, LPARAM param) {
    auto* search = reinterpret_cast<WindowSearch*>(param);
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);

    if (pid == search->pid && IsWindowVisible(window) && GetWindow(window, GW_OWNER) == nullptr) {
        search->hwnd = window;
        return FALSE;
    }
    return TRUE;
}
} // namespace

// pid -> handle, main window and module base, all from the exe name
static Process GetProcessByName(const std::string& processName) {
    Process proc{};
    const std::wstring wide(processName.begin(), processName.end());

    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (_wcsicmp(entry.szExeFile, wide.c_str()) == 0) {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }
        CloseHandle(snapshot);
    }
    if (!pid)
        return proc;

    proc.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!proc.hProcess)
        return proc;

    WindowSearch search{pid, nullptr};
    EnumWindows(FindMainWindow, reinterpret_cast<LPARAM>(&search));
    proc.hwnd = search.hwnd;

    HANDLE modules = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (modules != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W module;
        module.dwSize = sizeof(MODULEENTRY32W);

        if (Module32FirstW(modules, &module))
            proc.baseAddress = reinterpret_cast<uintptr_t>(module.modBaseAddr);

        CloseHandle(modules);
    }

    return proc;
}

// basically the initializer
void Memory::Initialize() {
    const Process proc = GetProcessByName("RobloxPlayerBeta.exe");
    hProcess = proc.hProcess;
    hwnd_ = proc.hwnd;
    baseAddress_ = proc.baseAddress;
}

// roblox strings are sso: inline buffer while short, heap pointer once length >= 0x10
std::string Memory::readString(uintptr_t addr) {
    struct sso_t {
        union {
            char buffer[0x10];
            uintptr_t ptr;
        } string;
        size_t length;
        size_t capacity;
    };

    const auto sso = rpm<sso_t>(hProcess, addr);
    if (!sso || sso->length == 0 || sso->length > 0x100)
        return "unknown";

    if (sso->length >= sizeof(sso->string.buffer)) {
        std::string out(sso->length, '\0');
        if (nt::read(hProcess, sso->string.ptr, out.data(), out.size(), nullptr) < 0)
            return "unknown";
        return out;
    }

    return std::string(sso->string.buffer, sso->length);
}
