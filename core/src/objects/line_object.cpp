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
    
    // Create line as a thin cylinder mesh for better visibility
    cMesh* line = new cMesh();
    
    // Calculate line properties
    cVector3d lineVector = pointB - pointA;
    double lineLength = lineVector.length();
    cVector3d lineCenter = (pointA + pointB) * 0.5;
    
    // Create a thin cylinder to represent the line
    double radius = lineWidth; // Convert line width to cylinder radius (much smaller!)
    cCreateCylinder(line, radius, lineLength);
    
    // Orient the cylinder to align with the line direction
    if (lineLength > 0.001) { // Avoid division by zero
        cVector3d upVector(0, 0, 1);
        cVector3d lineDirection = lineVector / lineLength;
        
        // Calculate rotation to align cylinder with line
        cVector3d rotationAxis = cCross(upVector, lineDirection);
        double rotationAngle = cAngle(upVector, lineDirection);
        
        if (rotationAxis.length() > 0.001) {
            rotationAxis.normalize();
            cMatrix3d rotation;
            rotation.setAxisAngleRotationDeg(rotationAxis, rotationAngle * 180.0 / C_PI);
            line->setLocalRot(rotation);
        }
    }
    
    // Position the cylinder at the line center
    line->setLocalPos(x + lineCenter.x(), y + lineCenter.y(), z + lineCenter.z());
    
    // Set material properties for visibility
    line->m_material->m_ambient.set(r, g, b);
    line->m_material->m_diffuse.set(r, g, b);
    line->m_material->m_specular.set(r, g, b);
    
    // Create collision detector
    line->createAABBCollisionDetector(std::max(radius, lineLength * 0.1));
    
    // Create magnetic effect
    line->createEffectMagnetic();
    
    // Set haptic properties
    line->m_material->setMagnetMaxDistance(magnetMaxDistance);
    line->m_material->setMagnetMaxForce(magnetMaxForce);
    line->m_material->setStiffness(stiffness);
    
    std::cout << "[LineObject] Line created from (" 
              << pointA.x() << ", " << pointA.y() << ", " << pointA.z() 
              << ") to (" 
              << pointB.x() << ", " << pointB.y() << ", " << pointB.z() 
              << ") with length=" << lineLength 
              << ", radius=" << radius 
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

