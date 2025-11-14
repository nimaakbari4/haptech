#include "torus_object.h"
#include <iostream>

cGenericObject* TorusObject::createObject(const std::map<std::string, std::string>& params,
                                          double x, double y, double z,
                                          double r, double g, double b) {
    std::cout << "[TorusObject] Creating torus object!" << std::endl;
    
    // Get parameters
    double innerRadius = getInnerRadius(params);
    double outerRadius = getOuterRadius(params);
    unsigned int numSides = getNumSides(params);
    unsigned int numRings = getNumRings(params);
    
    // Create torus mesh using cCreateRing (which creates a full torus)
    cShapeTorus* torus = new cShapeTorus(innerRadius, outerRadius);
    
    // Set color
    torus->m_material->m_ambient.set(r, g, b);
    torus->m_material->m_diffuse.set(r, g, b);
    torus->m_material->m_specular.set(r, g, b);
    
    // Position the torus
    torus->setLocalPos(x, y, z);

    cEffectSurface* newEffect = new cEffectSurface(torus);
    torus->addEffect(newEffect);
    
    // Note: Haptic material properties (stiffness) will be set by SimulationManager::addObject
    
    std::cout << "[TorusObject] Torus created with innerRadius=" << innerRadius 
              << ", outerRadius=" << outerRadius 
              << ", numSides=" << numSides 
              << ", numRings=" << numRings << std::endl;
    
    return torus;
}

void TorusObject::updatePhysics(cGenericObject* object, double deltaTime) {
    // TODO: Implement torus physics (e.g., rolling motion, gravity, complex rotational dynamics)
    // This is a stub implementation - no physics applied yet
}

double TorusObject::getInnerRadius(const std::map<std::string, std::string>& params) const {
    auto it = params.find("inner_radius");
    if (it == params.end()) {
        std::cout << "[ERROR] Torus requires 'inner_radius' parameter." << std::endl;
        return 0.02; // Default inner radius
    }
    return std::stod(it->second);
}

double TorusObject::getOuterRadius(const std::map<std::string, std::string>& params) const {
    auto it = params.find("outer_radius");
    if (it == params.end()) {
        std::cout << "[ERROR] Torus requires 'outer_radius' parameter." << std::endl;
        return 0.05; // Default outer radius
    }
    return std::stod(it->second);
}

unsigned int TorusObject::getNumSides(const std::map<std::string, std::string>& params) const {
    auto it = params.find("num_sides");
    return (it != params.end()) ? static_cast<unsigned int>(std::stoi(it->second)) : 16; // Default sides
}

unsigned int TorusObject::getNumRings(const std::map<std::string, std::string>& params) const {
    auto it = params.find("num_rings");
    return (it != params.end()) ? static_cast<unsigned int>(std::stoi(it->second)) : 16; // Default rings
}