#include "spring_object.h"
#include <iostream>
#include <cmath>

cGenericObject* SpringObject::createObject(const std::map<std::string, std::string>& params,
                                          double x, double y, double z,
                                          double r, double g, double b) {
    std::cout << "[SpringObject] Creating spring with custom physics!" << std::endl;
    
    // Get parameters
    double radius = getRadius(params);
    double height = getHeight(params);
    double stiffness = getStiffness(params);
    double damping = getDamping(params);
    
    // Create helical spring using cMultiSegment
    cMultiSegment* spring = new cMultiSegment();
    
    // Ensure material exists before using it
    if (!spring->m_material) {
        spring->m_material = cMaterial::create();
    }

    // Connect segments to form a spring
    double h = 0.0;
    double dh = height / 200.0;  // Height increment per segment
    double a = 0.0;
    double da = 0.2;  // Angle increment per segment
    double springRadius = radius;
    
    for (int i = 0; i < 200; i++) {
        double px0 = springRadius * cos(a);
        double py0 = springRadius * sin(a);
        double pz0 = h;
        double px1 = springRadius * cos(a + da);
        double py1 = springRadius * sin(a + da);
        double pz1 = h + dh;
        
        // Create vertex 0
        int index0 = spring->newVertex(px0, py0, pz0);
        
        // Create vertex 1
        int index1 = spring->newVertex(px1, py1, pz1);
        
        // Create segment by connecting both vertices together
        spring->newSegment(index0, index1);
        h = h + dh;
        a = a + da;
    }
    
    // Set haptic properties
    spring->m_material->setStiffness(stiffness);
    spring->m_material->setDamping(damping);
    
    // Clamp color values to [0,1] range for CHAI3D
    double red = std::max(0.0, std::min(1.0, r));
    double green = std::max(0.0, std::min(1.0, g));
    double blue = std::max(0.0, std::min(1.0, b));
    
    // For cMultiSegment, set line color (this is the primary color method)
    cColorf lineColor(red, green, blue, 1.0);
    spring->setLineColor(lineColor);
    
    // Assign line width for visibility
    spring->setLineWidth(4.0);
    
    // Also set material colors for consistency
    spring->m_material->setColorf(red, green, blue);
    
    // Use display list for faster rendering
    spring->setUseDisplayList(true);
    
    // Build collision tree
    spring->createAABBCollisionDetector(radius);
    
    // Set position
    spring->setLocalPos(x, y, z);
    
    // Set spring-specific physics properties
    spring->m_material->setStiffness(stiffness);
    spring->m_material->setDamping(damping);
    
    // Add spring-like behavior (oscillation)
    // This is where the researcher's custom physics would go
    std::cout << "[SpringObject] Spring created: radius=" << radius 
              << ", height=" << height << ", stiffness=" << stiffness << std::endl;
    
    return spring;
}

double SpringObject::getStiffness(const std::map<std::string, std::string>& params) const {
    auto it = params.find("stiffness");
    return (it != params.end()) ? std::stod(it->second) : 1000.0; // Default stiffness
}

double SpringObject::getDamping(const std::map<std::string, std::string>& params) const {
    auto it = params.find("damping");
    return (it != params.end()) ? std::stod(it->second) : 50.0; // Default damping
}

double SpringObject::getRadius(const std::map<std::string, std::string>& params) const {
    auto it = params.find("radius");
    return (it != params.end()) ? std::stod(it->second) : 0.02; // Default radius
}

double SpringObject::getHeight(const std::map<std::string, std::string>& params) const {
    auto it = params.find("height");
    return (it != params.end()) ? std::stod(it->second) : 0.1; // Default height
}
