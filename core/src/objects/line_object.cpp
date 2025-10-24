#include "line_object.h"
#include <iostream>

cGenericObject* LineObject::createObject(const std::map<std::string, std::string>& params,
                                        double x, double y, double z,
                                        double r, double g, double b) {
    std::cout << "[LineObject] Creating line object!" << std::endl;
    
    // Get parameters
    cVector3d pointA = getPointA(params);
    cVector3d pointB = getPointB(params);
    double lineWidth = getLineWidth(params);
    double magnetMaxDistance = getMagnetMaxDistance(params);
    double magnetMaxForce = getMagnetMaxForce(params);
    double stiffness = getStiffness(params);
    
    // Create line using cMultiSegment for proper visibility and haptic feedback
    cMultiSegment* line = new cMultiSegment();
    
    // Calculate line properties
    cVector3d lineVector = pointB - pointA;
    double lineLength = lineVector.length();
    
    // Create vertices for the line endpoints
    int indexA = line->newVertex(pointA.x(), pointA.y(), pointA.z());
    int indexB = line->newVertex(pointB.x(), pointB.y(), pointB.z());
    
    // Create segment connecting the two vertices
    line->newSegment(indexA, indexB);
    
    // Clamp color values to [0,1] range for CHAI3D
    double red = std::max(0.0, std::min(1.0, r));
    double green = std::max(0.0, std::min(1.0, g));
    double blue = std::max(0.0, std::min(1.0, b));
    
    // For cMultiSegment, set line color (this is the primary color method)
    cColorf lineColor(red, green, blue, 1.0);
    line->setLineColor(lineColor);
    
    // Assign line width for visibility
    line->setLineWidth(lineWidth);
    
    // Also set material colors for consistency
    line->m_material->setColorf(red, green, blue);
    
    // Use display list for faster rendering
    line->setUseDisplayList(true);
    
    // Create collision detector
    line->createAABBCollisionDetector(std::max(0.001, lineLength * 0.1));
    
    // Create magnetic effect
    line->createEffectMagnetic();
    
    // Set haptic properties
    line->m_material->setMagnetMaxDistance(magnetMaxDistance);
    line->m_material->setMagnetMaxForce(magnetMaxForce);
    line->m_material->setStiffness(stiffness);
    
    // Position the line
    line->setLocalPos(x, y, z);
    
    std::cout << "[LineObject] Line created from (" 
              << pointA.x() << ", " << pointA.y() << ", " << pointA.z() 
              << ") to (" 
              << pointB.x() << ", " << pointB.y() << ", " << pointB.z() 
              << ") with length=" << lineLength 
              << ", line width=" << lineWidth 
              << ", magnet distance=" << magnetMaxDistance 
              << ", force=" << magnetMaxForce << ", stiffness=" << stiffness << std::endl;
    
    return line;
}

cVector3d LineObject::getPointA(const std::map<std::string, std::string>& params) const {
    double x = 0.0, y = 0.0, z = 0.0;
    
    auto it = params.find("point_a_x");
    if (it != params.end()) x = std::stod(it->second);
    
    it = params.find("point_a_y");
    if (it != params.end()) y = std::stod(it->second);
    
    it = params.find("point_a_z");
    if (it != params.end()) z = std::stod(it->second);
    
    return cVector3d(x, y, z);
}

cVector3d LineObject::getPointB(const std::map<std::string, std::string>& params) const {
    double x = 1.0, y = 0.0, z = 0.0; // Default to unit line along X-axis
    
    auto it = params.find("point_b_x");
    if (it != params.end()) x = std::stod(it->second);
    
    it = params.find("point_b_y");
    if (it != params.end()) y = std::stod(it->second);
    
    it = params.find("point_b_z");
    if (it != params.end()) z = std::stod(it->second);
    
    return cVector3d(x, y, z);
}

double LineObject::getLineWidth(const std::map<std::string, std::string>& params) const {
    auto it = params.find("line_width");
    if (it == params.end()) {
        return 5.0; // Default line width - increased for visibility
    }
    return std::stod(it->second);
}

double LineObject::getMagnetMaxDistance(const std::map<std::string, std::string>& params) const {
    auto it = params.find("magnet_max_distance");
    if (it == params.end()) {
        return 0.05; // Default magnetic distance
    }
    return std::stod(it->second);
}

double LineObject::getMagnetMaxForce(const std::map<std::string, std::string>& params) const {
    auto it = params.find("magnet_max_force");
    if (it == params.end()) {
        return 5.0; // Default magnetic force
    }
    return std::stod(it->second);
}

double LineObject::getStiffness(const std::map<std::string, std::string>& params) const {
    auto it = params.find("stiffness");
    if (it == params.end()) {
        return 500.0; // Default stiffness
    }
    return std::stod(it->second);
}

