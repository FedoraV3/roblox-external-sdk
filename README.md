# roblox-external-sdk

A C++23 skeleton for reading Roblox's client memory from outside the process. It gives you
process attachment, a string-safe read layer, an `Instance` tree walker, a DataModel resolver
and a `Player` layout — and nothing else. No cheat features, no menu, no injection.

Target: `RobloxPlayerBeta.exe` (64-bit), offsets dumped from **`version-4ea8e85481e34f46`**.

---

## Current state at a glance

| Area | State |
|---|---|
| Process attach (`Memory`) | **Working** — pid, handle, main window, module base |
| Memory reads (`rpm`, `readString`) | **Working** — including Roblox's SSO strings |
| Memory writes (`wpm`, `wpm_bytes`) | **Working**, untested outside debug helpers |
| Instance tree (`getChildren`, `getName`, `getClassName`, find-first) | **Working** |
| DataModel resolution | **Half-wired** — the pointer chain is not dereferenced (see [Known gaps](#known_gaps)) |
| `Player` field layout | **Defined**, reads untested |
| LocalPlayer access | **Stub** — empty header + a function with no return |
| `structs.h` | **Dead code** — entirely commented out |
| `main.cpp` | **Placeholder** — prints the client version, attaches, waits for Enter |

The project builds cleanly in its current state (`meson compile -C build-debug` → no work to do).
The warning in the old README ("will not work on its own") is about behaviour, not compilation:
it compiles, it just doesn't *do* anything yet.

---

## Layout

```
main.cpp                     entry point / attach
offsets.h                    the offset authority (395 constants, 73 namespaces)
structs.h                    commented-out struct mirror of the same dump (unused)
memory/
  memory.h                   static attach + read/write primitives
  memory.cpp                 toolhelp32 scan, window enum, module base, SSO string read
engine/
  instance/instance.h/.cpp   generic Instance wrapper (children, name, class)
  datamodel/DataModel.h/.cpp static DataModel holder, mutex-guarded
player/
  player.h/.cpp              Player : Instance, raw field layout, debug helpers
  localplayer/player.h/.cpp  LocalPlayer stub (unfinished)
meson.build                  build, source list is optional-file tolerant
```

Dependency direction is one-way:

```
main → Memory
         ↑
      Instance → DataModel
         ↑
       Player
```

`Instance` and `Player` are value types that hold a raw remote address. All state lives in
`Memory` (static) and `DataModel` (static). There is no global allocator, no singleton
instance, and no heap ownership of remote objects.

---

## Modules

### `memory/` — attach and primitives

`Memory` is a static-only utility class; every member is `static inline` or `static`.
Nothing is constructed.

```cpp
Memory::Initialize();                    // finds RobloxPlayerBeta.exe, fills the statics
Memory::handle();                        // HANDLE, or nullptr if attach failed
Memory::window();                        // main HWND (visible, owner-less)
Memory::base();                          // module base of the .exe
```

`Initialize()` opens the process with `PROCESS_ALL_ACCESS`, so run the tool elevated or the
handle comes back `nullptr` and every subsequent read fails silently.

Reads are `std::optional` — a failed or short `ReadProcessMemory` yields `std::nullopt`, so
callers check the value instead of a return code:

```cpp
template<typename T> static std::optional<T> rpm(uintptr_t addr);          // uses Memory::handle()
template<typename T> static std::optional<T> rpm(HANDLE h, uintptr_t addr);
template<typename T> static bool wpm(uintptr_t addr, const T& value);
static bool wpm_bytes(uintptr_t addr, const void* data, size_t len);
```

`readString(addr)` decodes Roblox's SSO string layout:

```
union { char buffer[0x10]; uintptr_t ptr; } string;   // +0x00
size_t length;                                        // +0x10
size_t capacity;                                      // +0x18
```

Lengths `< 0x10` are read from the inline buffer, longer ones follow the heap pointer.
Length `0` or `> 0x100` is rejected and returns the literal `"unknown"` — that sentinel is
the SDK-wide "read failed" value for strings.

### `engine/instance/` — the tree

```cpp
class Instance {
public:
    uintptr_t address = 0;
    explicit Instance(uintptr_t address = 0);
    explicit operator bool() const;        // address != 0

    static Instance getDataModel();       // declared, NOT defined (see Known gaps)
    std::vector<Instance> getChildren() const;
    std::string getName() const;
    std::string getClassName() const;
    Instance findFirstChild(std::string_view name) const;
    Instance findFirstChildOfClass(std::string_view name) const;
};
```

Address arithmetic, all via `Offsets::Instance`:

| Field | Chain |
|---|---|
| children | `[addr + 0x70]` (begin) .. `[addr + 0x8]` (end), array of `{object, ref}` pairs |
| name | `[[addr + 0x68] + 0x0]` → SSO string |
| class name | `[[addr + 0x18] + 0x8]` → SSO string |

`getChildren()` guards against the usual nonsense: `end <= start` returns empty, and a count
above `0x1000` is treated as garbage. It stops at the first null `object` slot rather than
trusting the end pointer.

Every failure path returns an empty `Instance` (`address == 0`, falsy) or `"unknown"`. There
are no exceptions and no error codes.

### `engine/datamodel/` — the root

`DataModel` is a static class, not an object; it stores one address and a mutex.

```cpp
static bool Initialize();          // currently just calls Update(), returns true
static void Update();              // recomputes the address under mtx
static uintptr_t getDataModelPtr();
static std::mutex mtx;
static void printDataModelAddr();  // debug builds only
```

Intended chain (from `Offsets::FakeDataModel` / `Offsets::DataModel`):

```
base + FakeDataModel::Pointer  →  [that]  →  + RealDataModel  →  DataModel
```

The header comment says "updates every 50ms", but nothing spawns a thread yet — `Update()`
only runs when you call it.

### `player/` — layout mirror

`Player : Instance` is a *field-layout* mirror of `Offsets::Player`, written as an explicit
`char pad_N[...]` struct, so `offsetof` matches the offsets file:

```
0x0b8 UserId          0x118 LocalPlayer      0x2a0 ModelInstance    0x370 MaxZoomDistance
0x100 LocaleId        0x120 DisplayName      0x2e0 Team             0x374 MinZoomDistance
0x364 AccountAge      0x378 CameraMode       0x3a0 HealthDisplayDistance
0x3b0 NameDisplayDistance                   0x3b8 TeamColor        0x11f0 Mouse
```

Only debug builds (`#ifndef NDEBUG`) carry the test helpers:

```cpp
bool setLocalPlayerWalkSpeed(double speed);
bool setLocalPlayerJumpPower(double power);
bool localPlayerSpeedAndJumpDefaults();
```

They write `Humanoid::Walkspeed` (0x1c8) and `Humanoid::JumpPower` (0x19c) as `double`, and
exist purely to prove the read/write path works. `getPlayer(name)` is a stub returning an
empty `Player`.

`player/localplayer/` is unfinished: the header is empty apart from include guards, and
`getLocalPlayer()` has no `return` statement.

### `offsets.h` — the authority

```
Dumped With     : RbxDumperV2 2.2.5
Source          : https://git.imtheo.lol/theo/RbxDumperV2
Roblox Version  : version-4ea8e85481e34f46
Dumped At       : 08:13 10/11/2026 (GMT)
Total Offsets   : 394
```

`namespace Offsets` with `inline std::string ClientVersion` and 73 nested namespaces
(`Instance`, `Humanoid`, `Player`, `DataModel`, `VisualEngine`, `Workspace`, `Lighting`,
`BasePart`, `Camera`, `TaskScheduler`, …). `ClientVersion` is the only non-`constexpr`
member; everything else is `inline constexpr uintptr_t`.

These are hard-coded for one client build. Roblox updates weekly, so treat a mismatch as
"re-dump and update `offsets.h`", not "patch the numbers".

### `structs.h` — dead code

A full `namespace Structs` mirror of the same dump: 73 `struct`s with `pad_N` members and
`// sizeof` comments. Every line is commented out, including `#pragma once`. It is listed in
`meson.build` as a header, so it is harmless, but nothing includes it. Either uncomment it
and switch `Player` to use it, or delete it.

---

## Build

```bash
meson setup build
meson compile -C build
./build/untitled5
```

Requires MSVC (or any C++23 compiler) and Windows headers only — no third-party deps.
`meson.build` adds sources/headers/include-dirs only if the files exist, so the folder layout
can be reshaped without touching the build file.

Build directories already present: `buildDir`, `build-debug`, `build-release`.
`build-release` predates the current `DataModel`/`Memory` refactor — rebuild it before
trusting it.

### Documentation

`Doxyfile` is checked in, and the generated HTML lives in `docs/`, published with
GitHub Pages: <https://fedorav3.github.io/roblox-external-sdk/>

```bash
doxygen Doxyfile      # -> docs/index.html
```

`README.md` is the Doxygen main page, and the `\warning` tags in the headers carry the
[known gaps](#known_gaps) to the exact declaration they apply to.

---

## Known gaps {#known_gaps}

Verified against the source as it stands, not guessed:

1. **`Instance::getDataModel()` is declared and never defined.** Calling it is a link error.
2. **`DataModel::Update()` never dereferences the first pointer.** It computes
   `(base + 0x8f8e968) + 0x1f8` — an address *inside the pointer*, not the DataModel.
   It should read `[base + Pointer]` first, then add `RealDataModel`.
3. **`getLocalPlayer()` has no `return`** (undefined behaviour) and repeats the same
   non-dereferenced chain. It is compiled, so keep it out of release paths.
4. **The Players service is never located.** `setLocalPlayerWalkSpeed` /
   `setLocalPlayerJumpPower` call `findFirstChildOfClass("Players")` *on the Player object*,
   so they walk the wrong tree and return `false`. There is also no `Offsets::Players`
   namespace — only `Offsets::Player` (the class) and `Offsets::PlayerMouse`.
5. **`localPlayerSpeedAndJumpDefaults()` uses `&&`.** If one write succeeds the other is
   skipped; `||` is the intended short-circuit on failure.
6. **Nothing calls `DataModel::Initialize()`,** and no thread drives the promised 50 ms update.
7. **`Memory::Initialize()` has no failure path.** A null handle is stored, every `rpm`
   returns `nullopt`, and the program still prints success.
8. **`"unknown"` is ambiguous** — it means both "read failed" and "the instance is literally
   named unknown". Check `Instance::operator bool` instead of comparing strings.
9. **Offsets are not validated at runtime** against `ClientVersion`; a stale dump fails
   silently as garbage values.

---

## Adding a feature

1. Add or confirm the offsets in `offsets.h` under the right namespace.
2. Model it as an `Instance` subclass holding `uintptr_t address` (or reuse the raw layout
   style from `Player`).
3. Resolve from the DataModel: `Workspace` (0x168) → `CurrentCamera` (0x4a0), or
   `Players` → `LocalPlayer` (0x118) once the service is found.
4. Read with `Memory::rpm<T>` / `write` with `Memory::wpm<T>`, and let `std::nullopt` and
   falsy `Instance`s carry failure. No exceptions, no error codes.

Conventions worth keeping: `Memory` stays static and dependency-free, wrappers stay
copyable value types, every remote read is bounds-checked, and test-only writes stay behind
`#ifndef NDEBUG`.
