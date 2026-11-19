#include <cstdio>

#include "DataModel.h"
#include "memory.h"
#include "offsets.h"

int main() {
    std::printf("Using offsets from client version: %s\n", Offsets::ClientVersion.c_str());
    Memory::Initialize();
    DataModel::Initialize();

#ifndef NDEBUG
    DataModel::printDataModelAddr();
#endif

    std::printf("Press enter to exit..\n");
    std::getchar();
    return 0;
}
