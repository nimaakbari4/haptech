#include "ball_object.h"
#include <iostream>

cGenericObject* BallObject::createObject(const std::map<std::string, std::string>& params,
                                         double x, double y, double z,
                                         double r, double g, double b) {
    std::cout << "[BallObject] Creating ball object with bounciness!" << std::endl;
    
    // Get parameters
    _radius = getRadius(params);
    _bounciness = getBounciness(params);
    
    // Create sphere mesh (ball is a bouncy sphere)
    cMesh* ball = new cMesh();
    cCreateSphere(ball, _radius);
    ball->createAABBCollisionDetector(_radius);
    
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
    // Simple gravity and bouncing physics
    cMesh* ball = dynamic_cast<cMesh*>(object);
    if (!ball) return;
    
    // Update gravity scale from parameters
    _gravityScale = getGravityScale({}); // Assuming params are stored or passed in some way
    
    // Apply gravity
    double gravity = -9.81 * _gravityScale;
    _velocity += gravity * deltaTime;
    
    // Update position
    cVector3d pos = ball->getLocalPos();
    pos.set(pos.x(), pos.y(), pos.z() + _velocity * deltaTime);
    
    // Check for ground collision (assuming ground at z=0)
    if (pos.z() - _radius < -0.4) {
        pos.set(pos.x(), pos.y(), -0.4 + _radius); // Reset position to above ground
        _velocity = -_velocity * _bounciness; // Reverse velocity with bounciness factor
    }
    
    ball->setLocalPos(pos);
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