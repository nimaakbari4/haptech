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
    auto pos = object->getLocalPos();
    bool collidedWithThisObject = false;
    // std::cout << "[BallObject] Collision events: " << collisionEvents << std::endl;

    // Handle haptic device collisions
    auto hapticPoint = SimulationManager::simContext.tool->getHapticPoint(0);
    int collisionEvents = hapticPoint->getNumCollisionEvents();

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

    // Handle collisions with other objects
    auto newPos = pos + _velocity * deltaTime;
    
    // Get world for collision detection
    cWorld* world = SimulationManager::simContext.world;
    if (world && _velocity.length() > 0.001) {
        cCollisionRecorder recorder;
        cCollisionSettings settings;
        settings.m_ignoreShapes = false;
        settings.m_checkForNearestCollisionOnly = false;
        settings.m_adjustObjectMotion = false;
        settings.m_collisionRadius = _radius;
        settings.m_checkHapticObjects = true;
        
        bool collision = world->computeCollisionDetection(pos, newPos, recorder, settings);
        
        static int debugCounter = 0;
        debugCounter++;
        
        if (debugCounter % 60 == 0) {
            std::cout << "[BallObject] Checking collision from (" << pos.x() << ", " << pos.y() << ", " << pos.z() 
                      << ") to (" << newPos.x() << ", " << newPos.y() << ", " << newPos.z() << ")" 
                      << " Found: " << collision << " Count: " << recorder.m_collisions.size() << std::endl;
        }
        
        if (collision && !recorder.m_collisions.empty()) {
            // Find closest collision not with self
            const cCollisionEvent* closestHit = nullptr;
            double closestDist = std::numeric_limits<double>::max();
            
            for (const auto& hit : recorder.m_collisions) {
                if (hit.m_object != object) {
                    double dist = cDistance(pos, hit.m_globalPos);
                    if (dist < closestDist) {
                        closestDist = dist;
                        closestHit = &hit;
                    }
                }
            }
            
            if (closestHit) {
                // Get collision normal
                cVector3d normal = closestHit->m_globalNormal;
                if (normal.length() > 0.1) {
                    normal.normalize();
                    
                    // Calculate velocity component toward surface
                    double velocityIntoSurface = cDot(_velocity, -normal);
                    
                    if (velocityIntoSurface > 0) {
                        // Reflect velocity off surface with bounciness damping
                        _velocity = _velocity + (1.0 + _bounciness) * velocityIntoSurface * normal;
                        
                        // Calculate how far the ball penetrated into the surface
                        double penetrationDepth = cDistance(pos, closestHit->m_globalPos);
                        if (penetrationDepth > 0) {
                            // Move ball back out of the surface
                            newPos = pos + normal * penetrationDepth;
                        }
                        
                        std::cout << "[BallObject] COLLISION! Bouncing with bounciness=" << _bounciness << std::endl;
                    }
                }
            }
        }
    }

    if (!collidedWithThisObject) {
        _velocity *= 0.99;
    }

    object->setLocalPos(newPos);
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