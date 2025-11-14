#ifndef SPHERE_OBJECT_H
#define SPHERE_OBJECT_H

#include "custom_object_interface.h"
#include <chai3d.h>

using namespace chai3d;

/**
 * Built-in sphere object as a custom object
 */
class SphereObject : public CustomObjectInterface {
public:
    SphereObject() = default;
    virtual ~SphereObject() = default;
    
    // CustomObjectInterface implementation
    cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                double x, double y, double z,
                                double r, double g, double b) override;
    
    void updatePhysics(cGenericObject* object, double deltaTime) override;
    
    std::string getTypeName() const override { return "sphere"; }

private:
    double getRadius(const std::map<std::string, std::string>& params) const;
};

#endif // SPHERE_OBJECT_H