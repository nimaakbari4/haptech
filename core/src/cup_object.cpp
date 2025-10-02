#include "cup_object.h"
#include <iostream>

cGenericObject* CupObject::createObject(const std::map<std::string, double>& params,
                                        double x, double y, double z,
                                        double r, double g, double b) {
    std::cout << "[CupObject] Creating half-ring cup with custom geometry!" << std::endl;
    
    // Get parameters
    double innerRadius = getInnerRadius(params);
    double outerRadius = getOuterRadius(params);
    double height = getHeight(params);
    double thickness = getThickness(params);
    
    // Create a half-circle cup (◡) using cCreateRingSection
    cMesh* cup = new cMesh();
    
    // Create a half-ring section (180 degrees for half-circle)
    cMatrix3d rotationY(0, 0, -1, 0, 1, 0, 1, 0, 0);
    cMatrix3d rotationZ(cCosDeg(90), -cSinDeg(90), 0.0, cSinDeg(90), cCosDeg(90), 0.0, 0, 0, 1);
    cMatrix3d rotation = cMul(rotationY, rotationZ);
    
    cCreateRingSection(cup, outerRadius, innerRadius, thickness, 180, true, 10, 10, 
                       cVector3d(0, 0, 0), rotation, cColorf(r, g, b, 1.0));
    
    // Position the cup
    cup->setLocalPos(x, y, z);
    
    // Create collision detector
    cup->createAABBCollisionDetector(outerRadius);
    
    // Set cup-specific material properties
    cup->m_material->setStiffness(2000.0); // Cups are rigid
    cup->m_material->setDamping(100.0);
    
    std::cout << "[CupObject] Half-ring cup created: inner_radius=" << innerRadius 
              << ", outer_radius=" << outerRadius << ", thickness=" << thickness << std::endl;
    
    return cup;
}

double CupObject::getInnerRadius(const std::map<std::string, double>& params) const {
    auto it = params.find("inner_radius");
    return (it != params.end()) ? it->second : 0.05; // Default inner radius
}

double CupObject::getOuterRadius(const std::map<std::string, double>& params) const {
    auto it = params.find("outer_radius");
    return (it != params.end()) ? it->second : 0.08; // Default outer radius
}

double CupObject::getHeight(const std::map<std::string, double>& params) const {
    auto it = params.find("height");
    return (it != params.end()) ? it->second : 0.1; // Default height
}

double CupObject::getThickness(const std::map<std::string, double>& params) const {
    auto it = params.find("thickness");
    return (it != params.end()) ? it->second : 0.03; // Default thickness
}
