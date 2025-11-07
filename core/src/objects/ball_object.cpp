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
    std::lock_guard<std::mutex> lock(_stateMutex);
    
    // Cast to mesh and validate
    cMesh* ball = dynamic_cast<cMesh*>(object);
    if (!ball) {
        return;
    }
    
    // Safety check for deltaTime
    if (deltaTime <= 0.0 || deltaTime > 0.1) {
        return;
    }
    
    // Get world for collision detection
    cWorld* world = SimulationManager::simContext.world;
    if (!world) {
        return;
    }
    
    // Update parameters
    _gravityScale = getGravityScale({});
    
    // Get current state
    cVector3d pos = ball->getLocalPos();
    static int debugCounter = 0;
    debugCounter++;
    
    // Phase 1: Check if ball is currently resting on a surface
    bool isOnSurface = false;
    cVector3d surfaceNormal(0, 0, 1);
    double groundLevel = -0.4;
    
    // Check ground contact
    if (pos.z() - _radius <= groundLevel + 0.005) {
        isOnSurface = true;
        surfaceNormal.set(0, 0, 1);
    }
    
    // Check object surface contact
    if (!isOnSurface) {
        cCollisionRecorder surfaceChecker;
        cCollisionSettings surfaceSettings;
        surfaceSettings.m_ignoreShapes = false;
        surfaceSettings.m_checkForNearestCollisionOnly = true;
        surfaceSettings.m_adjustObjectMotion = false;
        surfaceSettings.m_collisionRadius = _radius;
        surfaceSettings.m_checkHapticObjects = true;
        
        // Cast small ray downward to check for surface
        cVector3d rayStart = pos;
        cVector3d rayEnd = pos - cVector3d(0, 0, _radius + 0.01);
        
        bool foundSurface = world->computeCollisionDetection(rayStart, rayEnd, surfaceChecker, surfaceSettings);
        
        if (foundSurface && !surfaceChecker.m_collisions.empty()) {
            for (const auto& collision : surfaceChecker.m_collisions) {
                if (collision.m_object != ball) {
                    isOnSurface = true;
                    surfaceNormal = collision.m_globalNormal;
                    if (surfaceNormal.length() > 0.1) {
                        surfaceNormal.normalize();
                    } else {
                        surfaceNormal.set(0, 0, 1); // Default upward
                    }
                    break;
                }
            }
        }
    }
    
    // Phase 2: Apply gravity conditionally
    bool shouldApplyGravity = true;
    bool isAtRest = false;
    
    if (isOnSurface) {
        // Check if ball is moving away from surface (bouncing up)
        double velocityTowardSurface = cDot(_velocity, -surfaceNormal);
        
        if (velocityTowardSurface <= 0.05 || _velocity.length() < 0.15) {
            // Ball is resting or moving slowly on surface - SETTLE IT
            shouldApplyGravity = false;
            isAtRest = true;
            _velocity.zero();
            
            // // Completely stop movement that would push into surface
            // double penetrationVel = cDot(_velocity, -surfaceNormal);
            // if (penetrationVel > 0) {
            //     _velocity = _velocity - penetrationVel * (-surfaceNormal);
            // }
            
            // // Apply strong damping to settle the ball
            // _velocity *= 0.1;
            
            // If velocity is very small, just stop it completely
            // if (_velocity.length() < 0.02) {
            //     _velocity.zero();
            // }
        }
    }
    
    if (shouldApplyGravity) {
        double gravity = -9.81 * _gravityScale;
        _velocity.add(0, 0, gravity * deltaTime);
    }
    
    // Apply general damping for very slow movement
    if (_velocity.length() < 0.05) {
        _velocity *= 0.95;
    }
    
    // Phase 3: Calculate movement and check for collisions
    cVector3d deltaMovement = _velocity * deltaTime;
    cVector3d newPos = pos + deltaMovement;
    bool hadCollision = false;
    
    if (deltaMovement.length() > 0.001 || !isAtRest) {
        cCollisionRecorder moveChecker;
        cCollisionSettings moveSettings;
        moveSettings.m_ignoreShapes = false;
        moveSettings.m_checkForNearestCollisionOnly = false;
        moveSettings.m_adjustObjectMotion = false;
        moveSettings.m_collisionRadius = _radius;
        moveSettings.m_checkHapticObjects = true;
        
        bool collision = world->computeCollisionDetection(pos, newPos, moveChecker, moveSettings);
        
        if (collision && !moveChecker.m_collisions.empty()) {
            // Find closest collision
            double closestDist = std::numeric_limits<double>::max();
            const cCollisionEvent* closestHit = nullptr;
            
            for (const auto& hit : moveChecker.m_collisions) {
                if (hit.m_object != ball) {
                    double dist = cDistance(pos, hit.m_globalPos);
                    if (dist < closestDist) {
                        closestDist = dist;
                        closestHit = &hit;
                    }
                }
            }
            
            if (closestHit) {
                cVector3d hitPoint = closestHit->m_globalPos;
                cVector3d hitNormal = closestHit->m_globalNormal;
                
                if (hitNormal.length() > 0.1) {
                    hitNormal.normalize();
                    
                    double approachSpeed = cDot(_velocity, -hitNormal);
                    
                    if (approachSpeed > 0.01) { // Significant collision
                        // Bounce: reflect velocity
                        _velocity = _velocity + 2.0 * approachSpeed * hitNormal;
                        _velocity *= _bounciness; // Apply bounciness
                        
                        // Add progressive energy loss based on impact speed
                        double energyLoss = 0.1; // Base energy loss
                        if (approachSpeed < 0.5) {
                            energyLoss = 0.25; // More loss for slower impacts
                        }
                        if (approachSpeed < 0.2) {
                            energyLoss = 0.4; // Even more loss for very slow impacts
                        }
                        _velocity *= (1.0 - energyLoss);
                        
                        // Position ball outside surface
                        newPos = hitPoint + hitNormal * (_radius + 0.005);
                        hadCollision = true;
                        
                        if (debugCounter % 60 == 0) {
                            std::cout << "[BallObject] BOUNCE! Speed: " << _velocity.length() << std::endl;
                        }
                    }
                }
            }
        }
    }
    
    // Phase 4: Apply final position
    if (!hadCollision) {
        pos = newPos;
    } else {
        pos = newPos; // Use collision-corrected position
    }
    
    // Phase 5: Final ground collision safety check
    if (pos.z() - _radius < groundLevel) {
        pos.set(pos.x(), pos.y(), groundLevel + _radius + 0.001);
        
        if (_velocity.z() < 0) {
            if (abs(_velocity.z()) > 0.1) {
                _velocity.set(_velocity.x(), _velocity.y(), -_velocity.z() * _bounciness);
                if (debugCounter % 60 == 0) {
                    std::cout << "[BallObject] Ground bounce!" << std::endl;
                }
            } else {
                _velocity.set(_velocity.x() * 0.9, _velocity.y() * 0.9, 0);
                if (debugCounter % 60 == 0) {
                    std::cout << "[BallObject] Ground rest" << std::endl;
                }
            }
        }
    }
    
    // Debug output
    if (debugCounter % 60 == 0) {
        std::cout << "[BallObject] Frame " << debugCounter 
                  << " Pos: (" << pos.x() << ", " << pos.y() << ", " << pos.z() << ")"
                  << " Vel: " << _velocity.length()
                  << " OnSurf: " << isOnSurface
                  << " Grav: " << shouldApplyGravity << std::endl;
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