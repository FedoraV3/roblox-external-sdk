//
// Created by ir0n1c on 11/14/2026.
//

#include "player.h"
#include "player/player.h"

#include "offsets.h"
#include "../../engine/instance/instance.h"
#include "memory/memory.h"

Player getLocalPlayer() {
    uintptr_t DataModel = mem->rpm<uintptr_t>((mem->base() + Offsets::FakeDataModel::Pointer) + Offsets::FakeDataModel::RealDataModel).value_or(0);

}