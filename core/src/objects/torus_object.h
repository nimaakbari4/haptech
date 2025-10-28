#ifndef TORUS_OBJECT_H
#define TORUS_OBJECT_H

#include "custom_object_interface.h"
#include <chai3d.h>

using namespace chai3d;

/**
 * Built-in torus object as a custom object
 */
class TorusObject : public CustomObjectInterface {
public:
    TorusObject() = default;
    virtual ~TorusObject() = default;
    
    // CustomObjectInterface implementation
    cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                double x, double y, double z,
                                double r, double g, double b) override;
    
    void updatePhysics(cGenericObject* object, double deltaTime) override;
    
    std::string getTypeName() const override { return "torus"; }

private:
    double getInnerRadius(const std::map<std::string, std::string>& params) const;
    double getOuterRadius(const std::map<std::string, std::string>& params) const;
    unsigned int getNumSides(const std::map<std::string, std::string>& params) const;
    unsigned int getNumRings(const std::map<std::string, std::string>& params) const;
};

#endif // TORUS_OBJECT_H