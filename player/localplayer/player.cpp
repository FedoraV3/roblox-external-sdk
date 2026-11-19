//
// Created by ir0n1c on 11/14/2026.
//

#include "player.h"
#include "player/player.h"

#include "offsets.h"
#include "../../engine/instance/instance.h"
#include "memory/memory.h"

// FIXME: no return statement (UB), and the chain skips dereferencing FakeDataModel::Pointer
Player getLocalPlayer() {
    uintptr_t DataModel = Memory::rpm<uintptr_t>((Memory::base() + Offsets::FakeDataModel::Pointer) + Offsets::FakeDataModel::RealDataModel).value_or(0);

}
