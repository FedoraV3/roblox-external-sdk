//
// Created by ir0n1c on 11/15/2026.
//

#include "DataModel.h"

#include "offsets.h"
#include "memory/memory.h"

#include <chrono>
#include <thread>

uintptr_t DataModel::address = 0;
std::mutex DataModel::mtx;

uintptr_t DataModel::getDataModelPtr() {
    // encapsulation cuz i wanna make the address boom boom
    return address;
}

void DataModel::Update() {
    uintptr_t target = (Memory::base() + Offsets::FakeDataModel::Pointer) + Offsets::FakeDataModel::RealDataModel;

    std::scoped_lock lck(mtx);
    if (target == address)
        return;


    address = target;
}

#ifndef NDEBUG
void DataModel::printDataModelAddr() {
    std::printf("DataModel Address: 0x%llX\n", address);
}
#endif

bool DataModel::Initialize() {
    Update();

    // detached: lives until the process exits
    // shortcut: killed by exit(), join it if shutdown ever needs to be clean
    std::thread([] {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            Update();
        }
    }).detach();

    return true;
}
