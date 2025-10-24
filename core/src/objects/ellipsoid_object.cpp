#include "ellipsoid_object.h"
#include <iostream>

cGenericObject* EllipsoidObject::createObject(const std::map<std::string, std::string>& params,
                                            double x, double y, double z,
                                            double r, double g, double b) {
    std::cout << "[EllipsoidObject] Creating ellipsoid object!" << std::endl;
    
    // Get parameters
    double radiusX = getRadiusX(params);
    double radiusY = getRadiusY(params);
    double radiusZ = getRadiusZ(params);
    unsigned int numSlices = getNumSlices(params);
    unsigned int numStacks = getNumStacks(params);
    
    // Create ellipsoid mesh
    cMesh* ellipsoid = new cMesh();
    cCreateEllipsoid(ellipsoid, radiusX, radiusY, radiusZ, numSlices, numStacks);
    
    // Create collision detector using the largest radius for simplicity
    double maxRadius = std::max({radiusX, radiusY, radiusZ});
    ellipsoid->createAABBCollisionDetector(maxRadius);
    
    // Set color
    ellipsoid->m_material->m_ambient.set(r, g, b);
    ellipsoid->m_material->m_diffuse.set(r, g, b);
    ellipsoid->m_material->m_specular.set(r, g, b);
    
    // Position the ellipsoid
    ellipsoid->setLocalPos(x, y, z);
    
    std::cout << "[EllipsoidObject] Ellipsoid created with radii: " 
              << radiusX << ", " << radiusY << ", " << radiusZ 
              << " (slices: " << numSlices << ", stacks: " << numStacks << ")" << std::endl;
    
    return ellipsoid;
}

double EllipsoidObject::getRadiusX(const std::map<std::string, std::string>& params) const {
    auto it = params.find("radius_x");
    if (it == params.end()) {
        std::cout << "[WARNING] Ellipsoid missing 'radius_x' parameter, using default 0.05" << std::endl;
        return 0.05; // Default radius
    }
    return std::stod(it->second);
}

double EllipsoidObject::getRadiusY(const std::map<std::string, std::string>& params) const {
    auto it = params.find("radius_y");
    if (it == params.end()) {
        std::cout << "[WARNING] Ellipsoid missing 'radius_y' parameter, using default 0.05" << std::endl;
        return 0.05; // Default radius
    }
    return std::stod(it->second);
}

double EllipsoidObject::getRadiusZ(const std::map<std::string, std::string>& params) const {
    auto it = params.find("radius_z");
    if (it == params.end()) {
        std::cout << "[WARNING] Ellipsoid missing 'radius_z' parameter, using default 0.05" << std::endl;
        return 0.05; // Default radius
    }
    return std::stod(it->second);
}

unsigned int EllipsoidObject::getNumSlices(const std::map<std::string, std::string>& params) const {
    auto it = params.find("slices");
    if (it == params.end()) {
        return 32; // Default number of slices
    }
    return static_cast<unsigned int>(std::stoi(it->second));
}

unsigned int EllipsoidObject::getNumStacks(const std::map<std::string, std::string>& params) const {
    auto it = params.find("stacks");
    if (it == params.end()) {
        return 16; // Default number of stacks
    }
    return static_cast<unsigned int>(std::stoi(it->second));
}
