#ifndef SPRING_OBJECT_H
#define SPRING_OBJECT_H

#include "custom_object_interface.h"
#include <chai3d.h>

using namespace chai3d;

/**
 * Example custom object: Spring
 * This is how a researcher would implement their own object
 */
class SpringObject : public CustomObjectInterface {
public:
    SpringObject() = default;
    virtual ~SpringObject() = default;
    
    // CustomObjectInterface implementation
    cGenericObject* createObject(const std::map<std::string, double>& params,
                                double x, double y, double z,
                                double r, double g, double b) override;
    
    std::string getTypeName() const override { return "spring"; }

private:
    // Spring-specific methods
    double getStiffness(const std::map<std::string, double>& params) const;
    double getDamping(const std::map<std::string, double>& params) const;
    double getRadius(const std::map<std::string, double>& params) const;
    double getHeight(const std::map<std::string, double>& params) const;
};

#endif // SPRING_OBJECT_H
