#include "researcher_objects_registry.h"
#include <iostream>

bool ResearcherObjectsRegistry::registered_ = false;

void ResearcherObjectsRegistry::registerAllObjects() {
    if (registered_) {
        return;
    }
    
    std::cout << "[ResearcherObjectsRegistry] Registering researcher objects..." << std::endl;
    
    // Register SpringObject
    CustomObjectRegistry::registerType("spring", []() -> std::unique_ptr<CustomObjectInterface> {
        return std::make_unique<SpringObject>();
    });
    
    // Register CupObject
    CustomObjectRegistry::registerType("cup", []() -> std::unique_ptr<CustomObjectInterface> {
        return std::make_unique<CupObject>();
    });
    
    // TODO: Researchers can add their own objects here
    // Example:
    // CustomObjectRegistry::registerType("my_object", []() -> std::unique_ptr<CustomObjectInterface> {
    //     return std::make_unique<MyCustomObject>();
    // });
    
    registered_ = true;
    std::cout << "[ResearcherObjectsRegistry] Registration complete!" << std::endl;
}

void ResearcherObjectsRegistry::registerCustomObject(const std::string& typeName, 
                                                   std::function<std::unique_ptr<CustomObjectInterface>()> creator) {
    CustomObjectRegistry::registerType(typeName, creator);
    std::cout << "[ResearcherObjectsRegistry] Registered custom object: " << typeName << std::endl;
}
