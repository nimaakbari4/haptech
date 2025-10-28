#ifndef BALL_OBJECT_H
#define BALL_OBJECT_H

#include "custom_object_interface.h"
#include <chai3d.h>

using namespace chai3d;

/**
 * Built-in ball object as a custom object (sphere with bounciness)
 */
class BallObject : public CustomObjectInterface {
public:
    BallObject() = default;
    virtual ~BallObject() = default;
    
    // CustomObjectInterface implementation
    cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                double x, double y, double z,
                                double r, double g, double b) override;
    
    void updatePhysics(cGenericObject* object, double deltaTime) override;
    
    std::string getTypeName() const override { return "ball"; }

private:
    double getRadius(const std::map<std::string, std::string>& params) const;
    double getBounciness(const std::map<std::string, std::string>& params) const;
};

#endif // BALL_OBJECT_H