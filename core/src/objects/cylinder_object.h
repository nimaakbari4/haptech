#ifndef CYLINDER_OBJECT_H
#define CYLINDER_OBJECT_H

#include "custom_object_interface.h"
#include <chai3d.h>

using namespace chai3d;

/**
 * Built-in cylinder object as a custom object
 */
class CylinderObject : public CustomObjectInterface {
public:
    CylinderObject() = default;
    virtual ~CylinderObject() = default;
    
    // CustomObjectInterface implementation
    cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                double x, double y, double z,
                                double r, double g, double b) override;
    
    std::string getTypeName() const override { return "cylinder"; }

private:
    double getRadius(const std::map<std::string, std::string>& params) const;
    double getHeight(const std::map<std::string, std::string>& params) const;
};

#endif // CYLINDER_OBJECT_H