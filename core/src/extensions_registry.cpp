#include "extensions_registry.h"
#include "end_conditions/time_condition.h"
#include "end_conditions/position_condition.h"
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
    
    // Register trial end conditions
    std::cout << "[ExtensionsRegistry] Registering trial end conditions..." << std::endl;
    registerTrialEndCondition("time", [](std::map<std::string, std::string>& params, bool success, int priority) -> std::unique_ptr<TrialEndCondition> {
        return std::make_unique<TimeCondition>(params, success, priority);
    });
    
    registerTrialEndCondition("position", [](std::map<std::string, std::string>& params, bool success, int priority) -> std::unique_ptr<TrialEndCondition> {
        return std::make_unique<PositionCondition>(params, success, priority);
    });
    
    // TODO: Researchers can add their own extensions here
    // Example:
    // CustomObjectRegistry::registerType("my_object", []() -> std::unique_ptr<CustomObjectInterface> {
    //     return std::make_unique<MyCustomObject>();
    // });
    // EndConditionRegistry::registerCondition("my_condition", []() -> std::unique_ptr<TrialEndCondition> {
    //     return std::make_unique<MyCondition>();
    // });
    
    registered_ = true;
    std::cout << "[ExtensionsRegistry] Registration complete!" << std::endl;
}

void ExtensionsRegistry::registerCustomObject(const std::string& typeName, 
                                             std::function<std::unique_ptr<CustomObjectInterface>()> creator) {
    CustomObjectRegistry::registerType(typeName, creator);
    std::cout << "[ExtensionsRegistry] Registered custom object: " << typeName << std::endl;
}

void ExtensionsRegistry::registerTrialEndCondition(const std::string& typeName, 
                                             EndConditionRegistry::ConditionCreator creator) {
    EndConditionRegistry::registerCondition(typeName, creator);
    std::cout << "[ExtensionsRegistry] Registered trial end condition: " << typeName << std::endl;
}
