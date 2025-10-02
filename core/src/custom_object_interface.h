#ifndef CUSTOM_OBJECT_INTERFACE_H
#define CUSTOM_OBJECT_INTERFACE_H

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <chai3d.h>

using namespace chai3d;

/**
 * Interface that researchers implement for their custom objects
 */
class CustomObjectInterface {
public:
    virtual ~CustomObjectInterface() = default;
    
    /**
     * Create the CHAI3D object for this custom object
     * @param params Parameters from XML configuration
     * @param x,y,z Position
     * @param r,g,b Color
     * @return Created CHAI3D object
     */
    virtual cGenericObject* createObject(const std::map<std::string, double>& params,
                                        double x, double y, double z,
                                        double r, double g, double b) = 0;
    
    /**
     * Get the object type name (used in XML)
     */
    virtual std::string getTypeName() const = 0;
};

/**
 * Simple registry for custom object types
 */
class CustomObjectRegistry {
public:
    using ObjectCreator = std::function<std::unique_ptr<CustomObjectInterface>()>;
    
    /**
     * Register a custom object type
     */
    static void registerType(const std::string& typeName, ObjectCreator creator);
    
    /**
     * Create an object by type name
     */
    static std::unique_ptr<CustomObjectInterface> createObject(const std::string& typeName);
    
    /**
     * Check if a type is registered
     */
    static bool isTypeRegistered(const std::string& typeName);

private:
    static std::map<std::string, ObjectCreator> creators_;
};

#endif // CUSTOM_OBJECT_INTERFACE_H
