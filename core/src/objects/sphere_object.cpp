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

double SphereObject::getRadius(const std::map<std::string, std::string>& params) const {
    auto it = params.find("radius");
    if (it == params.end()) {
        std::cout << "[ERROR] Sphere requires 'radius' parameter." << std::endl;
        return 0.05; // Default radius
    }
    return std::stod(it->second);
}