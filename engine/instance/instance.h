//
// Created by ir0n1c on 11/14/2026.
//

#ifndef UNTITLED5_INSTANCE_H
#define UNTITLED5_INSTANCE_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

class Instance {
public:
    uintptr_t address = 0;

    explicit Instance(uintptr_t address = 0) : address(address) {}

    explicit operator bool() const { return address != 0; }

    static Instance getDataModel();

    std::vector<Instance> getChildren() const;

    std::string getName() const;
    std::string getClassName() const;

    Instance findFirstChild(std::string_view name) const;
    Instance findFirstChildOfClass(std::string_view name) const;
};

#endif //UNTITLED5_INSTANCE_H
