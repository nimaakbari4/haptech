#ifndef CUP_OBJECT_H
#define CUP_OBJECT_H

#include "custom_object_interface.h"
#include <chai3d.h>

using namespace chai3d;

/**
 * Example custom object: Cup
 * This is how a researcher would implement their own object
 */
class CupObject : public CustomObjectInterface {
public:
    CupObject() = default;
    virtual ~CupObject() = default;
    
    // CustomObjectInterface implementation
    cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                double x, double y, double z,
                                double r, double g, double b) override;
    
    void updatePhysics(cGenericObject* object, double deltaTime) override;
    
    std::string getTypeName() const override { return "cup"; }

private:
    // Cup-specific methods
    double getInnerRadius(const std::map<std::string, std::string>& params) const;
    double getOuterRadius(const std::map<std::string, std::string>& params) const;
    double getHeight(const std::map<std::string, std::string>& params) const;
    double getThickness(const std::map<std::string, std::string>& params) const;
};

#endif // CUP_OBJECT_H
