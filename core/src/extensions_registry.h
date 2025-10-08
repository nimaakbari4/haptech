#ifndef EXTENSIONS_REGISTRY_H
#define EXTENSIONS_REGISTRY_H

#include "trial_end_condition.h"
#include "custom_object_interface.h"
#include "spring_object.h"
#include "cup_object.h"

/**
 * Registry for extensions including custom objects and trial end conditions
 * Researchers add their custom objects and trial conditions here
 */
class ExtensionsRegistry {
public:
    /**
     * Register all extensions (custom objects, trial conditions, etc.)
     * Call this once at startup
     */
    static void registerAllExtensions();
    
    /**
     * Register a custom object type
     * Researchers can call this to add their own objects
     */
    static void registerCustomObject(const std::string& typeName, 
                                    std::function<std::unique_ptr<CustomObjectInterface>()> creator);

    /**
     * Register a trial end condition type
     * Researchers can call this to add their own conditions
     */
    static void registerTrialEndCondition(const std::string& typeName, 
                                    EndConditionRegistry::ConditionCreator creator);
private:
    static bool registered_;
};

#endif // EXTENSIONS_REGISTRY_H
