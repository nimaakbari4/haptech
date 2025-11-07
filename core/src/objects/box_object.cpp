#include "box_object.h"
#include <iostream>
#include <algorithm>

cGenericObject* BoxObject::createObject(const std::map<std::string, std::string>& params,
                                        double x, double y, double z,
                                        double r, double g, double b) {
    std::cout << "[BoxObject] Creating box object!" << std::endl;
    
    // Get parameters
    double width = getWidth(params);
    double height = getHeight(params);
    double depth = getDepth(params);
    
    // Create box mesh
    cMesh* box = new cMesh();
    cCreateBox(box, width, height, depth);
    double maxDim = std::max({width, height, depth});
    box->createAABBCollisionDetector(maxDim);
    box->setShowCollisionDetector(true);
    
    // Set color
    box->m_material->m_ambient.set(r, g, b);
    box->m_material->m_diffuse.set(r, g, b);
    box->m_material->m_specular.set(r, g, b);
    
    // Position the box
    box->setLocalPos(x, y, z);
    
    std::cout << "[BoxObject] Box created with dimensions: " << width << "x" << height << "x" << depth << std::endl;
    
    return box;
}

void BoxObject::updatePhysics(cGenericObject* object, double deltaTime) {
    // TODO: Implement box physics (e.g., gravity, collision response, rotation)
    // This is a stub implementation - no physics applied yet
}

double BoxObject::getWidth(const std::map<std::string, std::string>& params) const {
    auto it = params.find("width");
    if (it == params.end()) {
        std::cout << "[ERROR] Box requires 'width' parameter." << std::endl;
        return 0.1; // Default width
    }
    return std::stod(it->second);
}

double BoxObject::getHeight(const std::map<std::string, std::string>& params) const {
    auto it = params.find("height");
    if (it == params.end()) {
        std::cout << "[ERROR] Box requires 'height' parameter." << std::endl;
        return 0.1; // Default height
    }
    return std::stod(it->second);
}

double BoxObject::getDepth(const std::map<std::string, std::string>& params) const {
    auto it = params.find("depth");
    if (it == params.end()) {
        std::cout << "[ERROR] Box requires 'depth' parameter." << std::endl;
        return 0.1; // Default depth
    }
    return std::stod(it->second);
}