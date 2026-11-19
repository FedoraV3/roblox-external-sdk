//
// Created by ir0n1c on 11/14/2026.
//

#include "instance.h"

#include "memory/memory.h"
#include "offsets.h"

std::vector<Instance> Instance::getChildren() const {
    std::vector<Instance> children;

    const auto start = Memory::rpm<uintptr_t>(Memory::handle(), address + Offsets::Instance::ChildrenStart);
    const auto end = Memory::rpm<uintptr_t>(Memory::handle(), address + Offsets::Instance::ChildrenEnd);
    if (!start || !end || *end <= *start)
        return children;

    struct shared_ptr_slot {
        uintptr_t object;
        uintptr_t ref;
    };

    const auto count = (*end - *start) / sizeof(shared_ptr_slot);
    if (count == 0 || count > 0x1000)
        return children;

    std::vector<shared_ptr_slot> slots(count);
    if (nt::read(Memory::handle(), *start, slots.data(), slots.size() * sizeof(shared_ptr_slot), nullptr) < 0)
        return children;

    children.reserve(count);
    for (const auto& slot : slots) {
        if (!slot.object)
            break;

        children.emplace_back(slot.object);
    }

    return children;
}

std::string Instance::getName() const {
    const auto container = Memory::rpm<uintptr_t>(Memory::handle(), address + Offsets::Instance::NameContainer);
    if (!container)
        return "unknown";

    return Memory::readString(*container + Offsets::Instance::Name);
}

std::string Instance::getClassName() const {
    const auto descriptor = Memory::rpm<uintptr_t>(Memory::handle(), address + Offsets::Instance::ClassDescriptor);
    if (!descriptor)
        return "unknown";

    const auto name = Memory::rpm<uintptr_t>(Memory::handle(), *descriptor + Offsets::Instance::ClassName);
    if (!name)
        return "unknown";

    return Memory::readString(*name);
}

Instance Instance::findFirstChild(std::string_view name) const {
    for (const auto& child : getChildren()) {
        if (child.getName() == name) {
            return child;
        }
    }

    return Instance();
}

Instance Instance::findFirstChildOfClass(std::string_view name) const {
    for (const auto& child : getChildren()) {
        if (child.getClassName() == name) {
            return child;
        }
    }

    return Instance();
}

