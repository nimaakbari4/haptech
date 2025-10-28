#include "ball_object.h"
#include <iostream>

cGenericObject* BallObject::createObject(const std::map<std::string, std::string>& params,
                                         double x, double y, double z,
                                         double r, double g, double b) {
    std::cout << "[BallObject] Creating ball object with bounciness!" << std::endl;
    
    // Get parameters
    double radius = getRadius(params);
    double bounciness = getBounciness(params);
    
    // Create sphere mesh (ball is a bouncy sphere)
    cMesh* ball = new cMesh();
    cCreateSphere(ball, radius);
    ball->createAABBCollisionDetector(radius);
    
    // Set color
    ball->m_material->m_ambient.set(r, g, b);
    ball->m_material->m_diffuse.set(r, g, b);
    ball->m_material->m_specular.set(r, g, b);
    
    // Set bounciness by adjusting damping (less damping = more bouncy)
    ball->m_material->setDamping(50.0 * (1.0 - bounciness));
    
    // Position the ball
    ball->setLocalPos(x, y, z);
    
    std::cout << "[BallObject] Ball created with radius=" << radius << ", bounciness=" << bounciness << std::endl;
    
    return ball;
}

void BallObject::updatePhysics(cGenericObject* object, double deltaTime) {
    // TODO: Implement ball physics (e.g., bouncing with configurable bounciness, gravity, velocity damping)
    // This is a stub implementation - no physics applied yet
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