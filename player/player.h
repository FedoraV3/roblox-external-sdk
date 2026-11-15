//
// Created by ir0n1c on 11/14/2026.
//

#ifndef UNTITLED5_PLAYER_H
#define UNTITLED5_PLAYER_H

#include <string>

#include "../engine/instance/instance.h"

class Player : public Instance {
public:
    explicit Player(uintptr_t address = 0) : Instance(address) {}

    // inline constexpr uintptr_t AccountAge = 0x364;
    // inline constexpr uintptr_t CameraMode = 0x378;
    // inline constexpr uintptr_t DisplayName = 0x120;
    // inline constexpr uintptr_t HealthDisplayDistance = 0x3a0;
    // inline constexpr uintptr_t LocalPlayer = 0x118;
    // inline constexpr uintptr_t LocaleId = 0x100;
    // inline constexpr uintptr_t MaxZoomDistance = 0x370;
    // inline constexpr uintptr_t MinZoomDistance = 0x374;
    // inline constexpr uintptr_t ModelInstance = 0x2a0;
    // inline constexpr uintptr_t Mouse = 0x11f0;
    // inline constexpr uintptr_t NameDisplayDistance = 0x3b0;
    // inline constexpr uintptr_t Team = 0x2e0;
    // inline constexpr uintptr_t TeamColor = 0x3b8;
    // inline constexpr uintptr_t UserId = 0xb8;
    char pad_0[0xb8];
    uintptr_t UserId; // 0xb8
    char pad_1[0x40];
    std::string LocaleId; // 0x100
    char pad_2[0x18];
    uintptr_t LocalPlayer; // 0x118
    std::string DisplayName; // 0x120
    char pad_3[0x180];
    uintptr_t ModelInstance; // 0x2a0
    char pad_4[0x38];
    uintptr_t Team; // 0x2e0
    char pad_5[0x7c];
    int AccountAge; // 0x364
    char pad_6[0x8];
    float MaxZoomDistance; // 0x370
    float MinZoomDistance; // 0x374
    int CameraMode; // 0x378
    char pad_7[0x24];
    float HealthDisplayDistance; // 0x3a0
    char pad_8[0xc];
    float NameDisplayDistance; // 0x3b0
    char pad_9[0x4];
    int TeamColor; // 0x3b8
    char pad_10[0xe34];
    uintptr_t Mouse; // 0x11f0

    static Player getPlayer(std::string& playerName);

    // test funcs that will get stripped out in the release build
#ifndef NDEBUG
    bool setLocalPlayerWalkSpeed(double speed);
    bool setLocalPlayerJumpPower(double power);

    bool localPlayerSpeedAndJumpDefaults();
#endif
};

#endif //UNTITLED5_PLAYER_H
