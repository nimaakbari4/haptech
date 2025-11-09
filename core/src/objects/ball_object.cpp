#include "ball_object.h"
#include <iostream>
#include <limits>
#include <simulation_manager.h>

cGenericObject* BallObject::createObject(const std::map<std::string, std::string>& params,
                                         double x, double y, double z,
                                         double r, double g, double b) {
    std::cout << "[BallObject] Creating ball object with bounciness!" << std::endl;
    
    // Get parameters
    _radius = getRadius(params);
    _bounciness = getBounciness(params);
    _mass = getMass(params);
    
    // Create sphere mesh (ball is a bouncy sphere)
    cMesh* ball = new cMesh();
    cCreateSphere(ball, _radius);
    ball->createAABBCollisionDetector(_radius);
    ball->setShowCollisionDetector(true);
    
    // Set color
    ball->m_material->m_ambient.set(r, g, b);
    ball->m_material->m_diffuse.set(r, g, b);
    ball->m_material->m_specular.set(r, g, b);
    
    // Set bounciness by adjusting damping (less damping = more bouncy)
    ball->m_material->setDamping(50.0 * (1.0 - _bounciness));
    
    // Position the ball
    ball->setLocalPos(x, y, z);
    
    std::cout << "[BallObject] Ball created with radius=" << _radius << ", bounciness=" << _bounciness << std::endl;
    
    return ball;
}

void BallObject::updatePhysics(cGenericObject* object, double deltaTime) {
    auto hapticPoint = SimulationManager::simContext.tool->getHapticPoint(0);
    auto pos = object->getLocalPos();
    int collisionEvents = hapticPoint->getNumCollisionEvents();
    bool collidedWithThisObject = false;
    // std::cout << "[BallObject] Collision events: " << collisionEvents << std::endl;

    if (collisionEvents > 0) {
        for (int i = 0; i < collisionEvents; i++) {
            auto collisionEvent = hapticPoint->getCollisionEvent(i);

            if (collisionEvent->m_object == object) {
                collidedWithThisObject = true;
                auto forceDirection = collisionEvent->m_globalNormal * -1;
                forceDirection.normalize();

                double forceMagnitude = hapticPoint->getLastComputedForce().length();
                forceMagnitude /= _mass;

                _velocity.add(forceDirection * forceMagnitude);
            }
        }
    }

    if (!collidedWithThisObject) {
        _velocity *= 0.99;
    }

    object->setLocalPos(pos + _velocity * deltaTime);
}

double BallObject::getRadius(const std::map<std::string, std::string>& params) const {
    auto it = params.find("radius");
    if (it == params.end()) {
        std::cout << "[ERROR] Ball requires 'radius' parameter." << std::endl;
        return 0.05; // Default radius
    }
    return std::stod(it->second);
}

double BallObject::getBounciness(const std::map<std::string, std::string>& params) const {
    auto it = params.find("bounciness");
    return (it != params.end()) ? std::stod(it->second) : 0.5; // Default bounciness
}

double BallObject::getGravityScale(const std::map<std::string, std::string>& params) const {
    auto it = params.find("gravity_scale");
    return (it != params.end()) ? std::stod(it->second) : 1.0; // Default gravity scale
}

double BallObject::getMass(const std::map<std::string, std::string>& params) const {
    auto it = params.find("mass");
    return (it != params.end()) ? std::stod(it->second) : 1.0; // Default gravity scale
}