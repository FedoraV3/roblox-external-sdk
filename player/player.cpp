//
// Created by ir0n1c on 11/14/2026.
//

#include "player.h"

#include "offsets.h"
#include "memory/memory.h"

#ifndef NDEBUG
/***********************************************
 *  Test functions for debug build. To test if the SDK works
 ***********************************************/


bool Player::setLocalPlayerWalkSpeed(double speed) {
    Instance localPlayer = findFirstChildOfClass("Players");
    if (!localPlayer) {
        return false;
    }

    Instance humanoid = localPlayer.findFirstChild("Character").findFirstChild("Humanoid");
    if (!humanoid) {
        return false;
    }

    return mem->wpm<double>(humanoid.address + Offsets::Humanoid::Walkspeed, speed);
}
bool Player::setLocalPlayerJumpPower(double power) {
    Instance localPlayer = findFirstChildOfClass("Players");
    if (!localPlayer) {
        return false;
    }

    Instance humanoid = localPlayer.findFirstChild("Character").findFirstChild("Humanoid");
    if (!humanoid) {
        return false;
    }

    // JumpPower or JumpHeight is game based; so this might not work
    return mem->wpm<double>(humanoid.address + Offsets::Humanoid::JumpPower, power);
}

bool Player::localPlayerSpeedAndJumpDefaults() {
    if (setLocalPlayerWalkSpeed(16) == false && setLocalPlayerJumpPower(50) == false) {
        return false;
    }

    return true;
}
#endif

Player Player::getPlayer(std::string &playerName) {

    return Player();
}
