#ifndef RESEARCHER_OBJECTS_REGISTRY_H
#define RESEARCHER_OBJECTS_REGISTRY_H

#include "custom_object_interface.h"
#include "spring_object.h"
#include "cup_object.h"

/**
 * Registry for researcher-created custom objects
 * Researchers add their objects here
 */
class ResearcherObjectsRegistry {
public:
    /**
     * Register all researcher objects
     * Call this once at startup
     */
    static void registerAllObjects();
    
    /**
     * Register a custom object type
     * Researchers can call this to add their own objects
     */
    static void registerCustomObject(const std::string& typeName, 
                                    std::function<std::unique_ptr<CustomObjectInterface>()> creator);

private:
    static bool registered_;
};

#endif // RESEARCHER_OBJECTS_REGISTRY_H
