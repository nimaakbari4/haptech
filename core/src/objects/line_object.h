#ifndef LINE_OBJECT_H
#define LINE_OBJECT_H

#include "custom_object_interface.h"
#include <chai3d.h>

using namespace chai3d;

/**
 * Built-in line object as a custom object
 * Creates a magnetic line that can guide robot movement
 */
class LineObject : public CustomObjectInterface {
public:
    LineObject() = default;
    virtual ~LineObject() = default;
    
    // CustomObjectInterface implementation
    cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                double x, double y, double z,
                                double r, double g, double b) override;
    
    void updatePhysics(cGenericObject* object, double deltaTime) override;
    
    std::string getTypeName() const override { return "line"; }

private:
    cVector3d getPointA(const std::map<std::string, std::string>& params) const;
    cVector3d getPointB(const std::map<std::string, std::string>& params) const;
    double getLineWidth(const std::map<std::string, std::string>& params) const;
    double getMagnetMaxDistance(const std::map<std::string, std::string>& params) const;
    double getMagnetMaxForce(const std::map<std::string, std::string>& params) const;
    double getStiffness(const std::map<std::string, std::string>& params) const;
};

#endif // LINE_OBJECT_H
