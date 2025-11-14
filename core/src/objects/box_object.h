#ifndef BOX_OBJECT_H
#define BOX_OBJECT_H

#include "custom_object_interface.h"
#include <chai3d.h>

using namespace chai3d;

/**
 * Built-in box object as a custom object
 */
class BoxObject : public CustomObjectInterface {
public:
    BoxObject() = default;
    virtual ~BoxObject() = default;
    
    // CustomObjectInterface implementation
    cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                double x, double y, double z,
                                double r, double g, double b) override;
    
    void updatePhysics(cGenericObject* object, double deltaTime) override;
    
    std::string getTypeName() const override { return "box"; }

private:
    double getWidth(const std::map<std::string, std::string>& params) const;
    double getHeight(const std::map<std::string, std::string>& params) const;
    double getDepth(const std::map<std::string, std::string>& params) const;
};

#endif // BOX_OBJECT_H