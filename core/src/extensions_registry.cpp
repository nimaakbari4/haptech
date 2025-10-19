#include "extensions_registry.h"
#include <iostream>

bool ExtensionsRegistry::registered_ = false;

void ExtensionsRegistry::registerAllExtensions() {
    if (registered_) {
        return;
    }
    
    std::cout << "[ExtensionsRegistry] Registering all extensions..." << std::endl;
    
    // Register custom objects
    std::cout << "[ExtensionsRegistry] Registering custom objects..." << std::endl;
    
    // Register SpringObject
    registerCustomObject("spring", []() -> std::unique_ptr<CustomObjectInterface> {
        return std::make_unique<SpringObject>();
    });
    
    // Register CupObject
    registerCustomObject("cup", []() -> std::unique_ptr<CustomObjectInterface> {
        return std::make_unique<CupObject>();
    });
    
    // Register trial conditions
    std::cout << "[ExtensionsRegistry] Registering trial conditions..." << std::endl;
    
    registerTrialCondition("time", [](std::map<std::string, std::string>& params, bool success, int priority) -> std::unique_ptr<TrialConditionInterface> {
        return std::make_unique<TimeCondition>(params, success, priority);
    });

    registerTrialCondition("position", [](std::map<std::string, std::string>& params, bool success, int priority) -> std::unique_ptr<TrialConditionInterface> {
        return std::make_unique<PositionCondition>(params, success, priority);
    });
    
    // TODO: Researchers can add their own extensions here
    // Example:
    // CustomObjectRegistry::registerType("my_object", []() -> std::unique_ptr<CustomObjectInterface> {
    //     return std::make_unique<MyCustomObject>();
    // });
    // TrialConditionRegistry::registerCondition("my_condition", [](std::map<std::string, std::string>& params, bool success, int priority) -> std::unique_ptr<TrialConditionInterface> {
    //     return std::make_unique<MyCondition>(params, success, priority);
    // });
    
    registered_ = true;
    std::cout << "[ExtensionsRegistry] Registration complete!" << std::endl;
}

void ExtensionsRegistry::registerCustomObject(const std::string& typeName, 
                                             std::function<std::unique_ptr<CustomObjectInterface>()> creator) {
    CustomObjectRegistry::registerType(typeName, creator);
    std::cout << "[ExtensionsRegistry] Registered custom object: " << typeName << std::endl;
}

void ExtensionsRegistry::registerTrialCondition(const std::string& typeName, 
                                             TrialConditionRegistry::ConditionCreator creator) {
    TrialConditionRegistry::registerCondition(typeName, creator);
    std::cout << "[ExtensionsRegistry] Registered trial condition: " << typeName << std::endl;
}
