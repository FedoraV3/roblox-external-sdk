//
// Created by ir0n1c on 11/14/2026.
//

#include "memory.h"
#include <Windows.h>
#include <tlhelp32.h>
#include <string>
#include <memory>

std::unique_ptr<Memory> mem;

struct Process {
    HANDLE hProcess;
    HWND hwnd;
    uintptr_t baseAddress;
};

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
Memory::Memory(const std::string& proc_name) {
    const Process proc = GetProcessByName(proc_name);
    hProcess = proc.hProcess;
    hwnd_ = proc.hwnd;
    baseAddress_ = proc.baseAddress;
}

void Memory::initialize() {
    mem = std::make_unique<Memory>("RobloxPlayerBeta.exe");
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
        if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(sso->string.ptr), out.data(), out.size(), nullptr))
            return "unknown";
        return out;
    }

    return std::string(sso->string.buffer, sso->length);
}
