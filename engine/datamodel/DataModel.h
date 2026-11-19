//
// Created by ir0n1c on 11/15/2026.
//

#ifndef UNTITLED5_DATAMODEL_H
#define UNTITLED5_DATAMODEL_H

#include "instance/instance.h"
#include <mutex>

/***********************************************
 *  Static class. This datamodel updates every 50ms.
 ***********************************************/
/// \brief Static holder for the DataModel address.
/// \warning Update() never dereferences FakeDataModel::Pointer. Initialize()
///          starts a detached thread that calls Update() every 50 ms.
class DataModel : public Instance {
private:
    static uintptr_t address;
public:
    static std::mutex mtx;

    // for some reason it looks like unity class HAHAHAHA
    static bool Initialize();
    static void Update();
    static uintptr_t getDataModelPtr();

#ifndef NDEBUG
    static void printDataModelAddr();
#endif
};


#endif //UNTITLED5_DATAMODEL_H
