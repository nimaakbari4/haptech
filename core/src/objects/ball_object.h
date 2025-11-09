#ifndef BALL_OBJECT_H
#define BALL_OBJECT_H

#include "custom_object_interface.h"
#include <chai3d.h>
#include <mutex>

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
    cVector3d _velocity{0.0, 0.0, 0.0};
    double _gravityScale = 1.0;
    double _radius = 0.05;
    double _bounciness = 0.5;
    double _mass = 1.0;
    mutable std::mutex _stateMutex;
    
    double getRadius(const std::map<std::string, std::string>& params) const;
    double getBounciness(const std::map<std::string, std::string>& params) const;
    double getGravityScale(const std::map<std::string, std::string>& params) const;
    double getMass(const std::map<std::string, std::string>& params) const;
};

#endif // BALL_OBJECT_H