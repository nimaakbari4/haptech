#ifndef EXTENSIONS_REGISTRY_H
#define EXTENSIONS_REGISTRY_H

#include "trial_conditions/trial_condition.h"
#include "trial_conditions/time_condition.h"
#include "trial_conditions/position_condition.h"
#include "trial_conditions/velocity_condition.h"
#include "objects/custom_object_interface.h"
#include "objects/spring_object.h"
#include "objects/cup_object.h"
#include "objects/sphere_object.h"
#include "objects/box_object.h"
#include "objects/cylinder_object.h"
#include "objects/ball_object.h"
#include "objects/torus_object.h"
#include "objects/ellipsoid_object.h"
#include "objects/line_object.h"
#include "objects/pong_ball_object.h"
#include "objects/pong_paddle_object.h"

/**
 * Registry for extensions including custom objects and trial conditions
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
     * Register a trial condition type
     * Researchers can call this to add their own conditions
     */
    static void registerTrialCondition(const std::string& typeName, 
                                    TrialConditionRegistry::ConditionCreator creator);
private:
    static bool registered_;
};

#endif // EXTENSIONS_REGISTRY_H