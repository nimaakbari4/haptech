#ifndef ELLIPSOID_OBJECT_H
#define ELLIPSOID_OBJECT_H

#include "custom_object_interface.h"
#include <chai3d.h>

using namespace chai3d;

/**
 * Built-in ellipsoid object as a custom object
 */
class EllipsoidObject : public CustomObjectInterface {
public:
    EllipsoidObject() = default;
    virtual ~EllipsoidObject() = default;
    
    // CustomObjectInterface implementation
    cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                double x, double y, double z,
                                double r, double g, double b) override;
    
    std::string getTypeName() const override { return "ellipsoid"; }

private:
    double getRadiusX(const std::map<std::string, std::string>& params) const;
    double getRadiusY(const std::map<std::string, std::string>& params) const;
    double getRadiusZ(const std::map<std::string, std::string>& params) const;
    unsigned int getNumSlices(const std::map<std::string, std::string>& params) const;
    unsigned int getNumStacks(const std::map<std::string, std::string>& params) const;
};

#endif // ELLIPSOID_OBJECT_H
