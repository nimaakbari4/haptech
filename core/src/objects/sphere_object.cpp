#include "sphere_object.h"
#include <iostream>

cGenericObject* SphereObject::createObject(const std::map<std::string, std::string>& params,
                                          double x, double y, double z,
                                          double r, double g, double b) {
    std::cout << "[SphereObject] Creating sphere object!" << std::endl;
    
    // Get parameters
    double radius = getRadius(params);
    
    // Create sphere mesh
    cMesh* sphere = new cMesh();
    cCreateSphere(sphere, radius);
    sphere->createAABBCollisionDetector(radius);
    
    // Set color
    sphere->m_material->m_ambient.set(r, g, b);
    sphere->m_material->m_diffuse.set(r, g, b);
    sphere->m_material->m_specular.set(r, g, b);
    
    // Position the sphere
    sphere->setLocalPos(x, y, z);
    
    std::cout << "[SphereObject] Sphere created with radius=" << radius << std::endl;
    
    return sphere;
}

void SphereObject::updatePhysics(cGenericObject* object, double deltaTime) {
    // TODO: Implement sphere physics (e.g., gravity, bouncing, rolling)
    // This is a stub implementation - no physics applied yet
}

double SphereObject::getRadius(const std::map<std::string, std::string>& params) const {
    auto it = params.find("radius");
    if (it == params.end()) {
        std::cout << "[ERROR] Sphere requires 'radius' parameter." << std::endl;
        return 0.05; // Default radius
    }
    return std::stod(it->second);
}