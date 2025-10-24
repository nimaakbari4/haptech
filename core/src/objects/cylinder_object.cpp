#include "cylinder_object.h"
#include <iostream>
#include <algorithm>

cGenericObject* CylinderObject::createObject(const std::map<std::string, std::string>& params,
                                             double x, double y, double z,
                                             double r, double g, double b) {
    std::cout << "[CylinderObject] Creating cylinder object!" << std::endl;
    
    // Get parameters
    double radius = getRadius(params);
    double height = getHeight(params);
    
    // Create cylinder mesh
    cMesh* cylinder = new cMesh();
    cCreateCylinder(cylinder, radius, height);
    double maxDim = std::max(radius, height);
    cylinder->createAABBCollisionDetector(maxDim);
    
    // Set color
    cylinder->m_material->m_ambient.set(r, g, b);
    cylinder->m_material->m_diffuse.set(r, g, b);
    cylinder->m_material->m_specular.set(r, g, b);
    
    // Position the cylinder
    cylinder->setLocalPos(x, y, z);
    
    std::cout << "[CylinderObject] Cylinder created with radius=" << radius << ", height=" << height << std::endl;
    
    return cylinder;
}

double CylinderObject::getRadius(const std::map<std::string, std::string>& params) const {
    auto it = params.find("radius");
    if (it == params.end()) {
        std::cout << "[ERROR] Cylinder requires 'radius' parameter." << std::endl;
        return 0.05; // Default radius
    }
    return std::stod(it->second);
}

double CylinderObject::getHeight(const std::map<std::string, std::string>& params) const {
    auto it = params.find("height");
    if (it == params.end()) {
        std::cout << "[ERROR] Cylinder requires 'height' parameter." << std::endl;
        return 0.1; // Default height
    }
    return std::stod(it->second);
}