//
// Created by ir0n1c on 11/14/2026.
//

#ifndef UNTITLED5_MEMORY_H
#define UNTITLED5_MEMORY_H

#include <optional>
#include <string>
#include <Windows.h>
#include <memory>


// Nt* memory syscalls, resolved from ntdll at first use (not in any import lib).
namespace nt {
NTSTATUS read(HANDLE h, uintptr_t addr, void* buf, size_t len, PULONG done);
NTSTATUS write(HANDLE h, uintptr_t addr, const void* buf, size_t len, PULONG done);
} // namespace nt


class Memory {
private:
    static inline HANDLE hProcess = nullptr;
    static inline HWND hwnd_ = nullptr;
    static inline uintptr_t baseAddress_ = 0;
public:
    /// \brief Find RobloxPlayerBeta.exe and fill handle/window/base.
    /// \warning No failure path: a null handle is stored silently and every later read fails.
    static void Initialize();

    static HANDLE handle() { return hProcess; }
    static HWND window() { return hwnd_; }
    static uintptr_t base() { return baseAddress_; }

    static std::string readString(uintptr_t addr);

    template<typename T>
    static std::optional<T> rpm(const uintptr_t addr = 0) { return rpm<T>(hProcess, addr); }

    template<typename T>
    static std::optional<T> rpm(const HANDLE h, const uintptr_t addr) {
        T out{};
        ULONG n = 0;
        if (nt::read(h, addr, &out, sizeof(T), &n) < 0 || n != sizeof(T))
            return std::nullopt;
        return out;
    }

    template<typename T>
    static bool wpm(uintptr_t addr, const T& value) {
        ULONG n = 0;
        return nt::write(hProcess, addr, &value, sizeof(T), &n) >= 0 && n == sizeof(T);
    }

    static bool wpm_bytes(uintptr_t addr, const void* data, size_t len) {
        ULONG n = 0;
        return nt::write(hProcess, addr, data, len, &n) >= 0 && n == len;
    }
};


#endif //UNTITLED5_MEMORY_H
