#include "custom_object_interface.h"
#include <iostream>

// Static member definition
std::map<std::string, CustomObjectRegistry::ObjectCreator> CustomObjectRegistry::creators_;

void CustomObjectRegistry::registerType(const std::string& typeName, ObjectCreator creator) {
    creators_[typeName] = creator;
    std::cout << "[CustomObjectRegistry] Registered type: " << typeName << std::endl;
}

std::unique_ptr<CustomObjectInterface> CustomObjectRegistry::createObject(const std::string& typeName) {
    auto it = creators_.find(typeName);
    if (it != creators_.end()) {
        return it->second();
    }
    
    std::cout << "[CustomObjectRegistry] Unknown type: " << typeName << std::endl;
    return nullptr;
}

bool CustomObjectRegistry::isTypeRegistered(const std::string& typeName) {
    return creators_.find(typeName) != creators_.end();
}
