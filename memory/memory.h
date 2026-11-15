//
// Created by ir0n1c on 11/14/2026.
//

#ifndef UNTITLED5_MEMORY_H
#define UNTITLED5_MEMORY_H

#include <optional>
#include <string>
#include <Windows.h>
#include <memory>


class Memory {
private:
    HANDLE hProcess = nullptr;
    HWND hwnd_ = nullptr;
    uintptr_t baseAddress_ = 0;
public:
    explicit Memory(const std::string& proc_name);

    static void initialize();

    HANDLE handle() const { return hProcess; }
    HWND window() const { return hwnd_; }
    uintptr_t base() const { return baseAddress_; }

    std::string readString(uintptr_t addr);

    template<typename T>
    std::optional<T> rpm(const uintptr_t addr = 0) { return rpm<T>(hProcess, addr); }

    template<typename T>
    std::optional<T> rpm(const HANDLE h, const uintptr_t addr) {
        T out{};
        SIZE_T n = 0;
        if (!ReadProcessMemory(h, reinterpret_cast<LPCVOID>(addr), &out, sizeof(T), &n) || n != sizeof(T))
            return std::nullopt;
        return out;
    }

    template<typename T>
    bool wpm(uintptr_t addr, const T& value) const {
        SIZE_T n = 0;
        return WriteProcessMemory(hProcess, (LPVOID)addr, &value, sizeof(T), &n) && n == sizeof(T);
    }

    bool wpm_bytes(uintptr_t addr, const void* data, size_t len) const {
        SIZE_T n = 0;
        return WriteProcessMemory(hProcess, (LPVOID)addr, data, len, &n) && n == len;
    }
};

extern std::unique_ptr<Memory> mem;


#endif //UNTITLED5_MEMORY_H
