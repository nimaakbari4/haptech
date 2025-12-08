# Parameter System

Documentation for the parameter system used in custom objects.

## Overview

The parameter system allows XML configuration files to pass custom settings to objects at creation time. Parameters are passed as a `std::map<std::string, std::string>` to the `createObject()` method, providing flexible configuration without requiring code changes.

## XML Parameter Syntax

Parameters are specified in the XML configuration using the `<Parameters>` element with attributes:

```xml
<Object id="my_object" type="sphere">
  <Position x="0.0" y="0.1" z="0.0"/>
  <Color r="1.0" g="0.0" b="0.0"/>
  <Parameters radius="0.05" stiffness="1500" friction="0.3"/>
</Object>
```

Each attribute in the `<Parameters>` element becomes a key-value pair in the parameters map:
- Key: attribute name (e.g., `"radius"`)  
- Value: attribute value as string (e.g., `"0.05"`)

## Parameter Parsing Patterns

### Basic Type Conversion

```cpp
class MyObject : public CustomObjectInterface {
private:
    // Double/float parameters
    double getRadius(const std::map<std::string, std::string>& params) const {
        auto it = params.find("radius");
        if (it != params.end()) {
            try {
                double value = std::stod(it->second);
                return (value > 0.0) ? value : 0.05; // Validate range
            } catch (const std::invalid_argument& e) {
                std::cerr << "[MyObject] Invalid radius: " << it->second << std::endl;
            }
        }
        return 0.05; // Default value
    }
    
    // Integer parameters
    int getResolution(const std::map<std::string, std::string>& params) const {
        auto it = params.find("resolution");
        if (it != params.end()) {
            try {
                int value = std::stoi(it->second);
                return std::max(4, std::min(64, value)); // Clamp to range [4, 64]
            } catch (const std::invalid_argument& e) {
                std::cerr << "[MyObject] Invalid resolution: " << it->second << std::endl;
            }
        }
        return 16; // Default value
    }
    
    // Boolean parameters
    bool isVisible(const std::map<std::string, std::string>& params) const {
        auto it = params.find("visible");
        if (it != params.end()) {
            std::string value = it->second;
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);
            return (value == "true" || value == "1" || value == "yes");
        }
        return true; // Default value
    }
};
```

### Advanced Parameter Parsing

```cpp
// Vector3 parameters (comma-separated)
cVector3d getDirection(const std::map<std::string, std::string>& params) const {
    auto it = params.find("direction");
    if (it != params.end()) {
        std::string value = it->second;
        std::vector<std::string> components;
        
        // Split by comma
        size_t pos = 0;
        while ((pos = value.find(',')) != std::string::npos) {
            components.push_back(value.substr(0, pos));
            value.erase(0, pos + 1);
        }
        components.push_back(value); // Last component
        
        if (components.size() == 3) {
            try {
                double x = std::stod(components[0]);
                double y = std::stod(components[1]);
                double z = std::stod(components[2]);
                return cVector3d(x, y, z);
            } catch (const std::invalid_argument& e) {
                std::cerr << "[MyObject] Invalid direction vector: " << it->second << std::endl;
            }
        }
    }
    return cVector3d(0, 1, 0); // Default up direction
}

// Enum parameters
enum class MaterialType { METAL, WOOD, PLASTIC, RUBBER };

MaterialType getMaterialType(const std::map<std::string, std::string>& params) const {
    auto it = params.find("material");
    if (it != params.end()) {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        
        if (value == "metal") return MaterialType::METAL;
        if (value == "wood") return MaterialType::WOOD;
        if (value == "plastic") return MaterialType::PLASTIC;
        if (value == "rubber") return MaterialType::RUBBER;
        
        std::cerr << "[MyObject] Unknown material: " << value << std::endl;
    }
    return MaterialType::PLASTIC; // Default
}
```

### Parameter Validation

```cpp
class ParameterValidator {
public:
    static double getPositiveDouble(const std::map<std::string, std::string>& params,
                                   const std::string& key, double defaultValue,
                                   double minValue = 0.001, double maxValue = 1000.0) {
        auto it = params.find(key);
        if (it != params.end()) {
            try {
                double value = std::stod(it->second);
                if (value >= minValue && value <= maxValue) {
                    return value;
                }
                std::cerr << "[Validator] " << key << " out of range [" 
                         << minValue << ", " << maxValue << "]: " << value << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[Validator] Invalid " << key << ": " << it->second << std::endl;
            }
        }
        return defaultValue;
    }
    
    static int getRangedInt(const std::map<std::string, std::string>& params,
                           const std::string& key, int defaultValue,
                           int minValue, int maxValue) {
        auto it = params.find(key);
        if (it != params.end()) {
            try {
                int value = std::stoi(it->second);
                return std::max(minValue, std::min(maxValue, value));
            } catch (const std::exception& e) {
                std::cerr << "[Validator] Invalid " << key << ": " << it->second << std::endl;
            }
        }
        return defaultValue;
    }
};

// Usage in createObject:
double radius = ParameterValidator::getPositiveDouble(params, "radius", 0.05, 0.001, 0.5);
int resolution = ParameterValidator::getRangedInt(params, "resolution", 16, 4, 64);
```

## Common Parameter Categories

### Geometric Parameters

Standard parameters for object dimensions:

```cpp
// Size/dimension parameters
double radius, width, height, depth, length;
double inner_radius, outer_radius, thickness;
int resolution, num_sides, num_slices, num_stacks;

// Position/orientation (handled by SimulationManager, not custom objects)
// Objects receive initial position via x, y, z parameters to createObject()
```

### Material Parameters

Parameters affecting haptic feel and appearance:

```cpp
// Haptic properties
double stiffness = getStiffness(params);           // N/m
double static_friction = getStaticFriction(params); // 0.0-1.0
double dynamic_friction = getDynamicFriction(params); // 0.0-1.0
double damping = getDamping(params);               // Damping coefficient

// Visual properties  
double transparency = getTransparency(params);      // 0.0-1.0
bool wireframe = getWireframe(params);             // true/false
std::string texture_file = getTextureFile(params); // Path to texture
```

### Physics Parameters

Parameters for objects with dynamic behavior:

```cpp
// Physics properties
double mass = getMass(params);                     // kg
double gravity_scale = getGravityScale(params);    // Gravity multiplier
double bounciness = getBounciness(params);         // Coefficient of restitution
double air_resistance = getAirResistance(params);  // Drag coefficient

// Motion parameters
double max_velocity = getMaxVelocity(params);      // m/s
double acceleration = getAcceleration(params);     // m/s²
```

### Behavioral Parameters

Parameters controlling object behavior:

```cpp
// State flags
bool enabled = getEnabled(params);                 // Object active/inactive
bool visible = getVisible(params);                 // Render visibility
bool haptic_enabled = getHapticEnabled(params);    // Haptic interaction

// Timing parameters
double update_frequency = getUpdateFrequency(params); // Hz
double duration = getDuration(params);               // seconds
```

## XML Configuration Examples

### Complex Object Configuration

```xml
<Object id="custom_spring" type="spring">
  <Position x="0.0" y="0.1" z="0.0"/>
  <Orientation x="0" y="45" z="0"/>
  <Color r="0.7" g="0.7" b="0.9"/>
  <Parameters 
    stiffness="2000.0"
    damping="25.0"
    radius="0.03"
    height="0.15"
    coils="10"
    resolution="20"
    material="metal"
    max_compression="0.8"
    friction="0.4"
    visible="true"
    haptic_enabled="true"/>
</Object>
```

### Array/List Parameters

For parameters requiring multiple values:

```xml
<!-- Option 1: Comma-separated values -->
<Parameters waypoints="0,0,0,0.1,0.1,0,0.2,0.2,0"/>

<!-- Option 2: Multiple parameters with indices -->
<Parameters 
  waypoint_0_x="0.0" waypoint_0_y="0.0" waypoint_0_z="0.0"
  waypoint_1_x="0.1" waypoint_1_y="0.1" waypoint_1_z="0.0"
  waypoint_2_x="0.2" waypoint_2_y="0.2" waypoint_2_z="0.0"/>
```

## Best Practices

### Parameter Design

1. **Provide Defaults**: Always have sensible default values
2. **Validate Input**: Check ranges and handle invalid values gracefully
3. **Document Parameters**: Comment expected ranges and units
4. **Use Consistent Units**: Prefer SI units (meters, kilograms, seconds)
5. **Meaningful Names**: Use descriptive parameter names

### Error Handling

```cpp
double getParameter(const std::map<std::string, std::string>& params,
                   const std::string& key, double defaultValue) const {
    auto it = params.find(key);
    if (it != params.end()) {
        try {
            return std::stod(it->second);
        } catch (const std::invalid_argument& e) {
            std::cerr << "[" << getTypeName() << "] Invalid " << key 
                      << " parameter: " << it->second << ", using default: " 
                      << defaultValue << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "[" << getTypeName() << "] " << key 
                      << " parameter out of range: " << it->second 
                      << ", using default: " << defaultValue << std::endl;
        }
    }
    return defaultValue;
}
```

### Parameter Documentation

Document your parameters clearly:

```cpp
/**
 * SpringObject parameters:
 * - stiffness (double, default: 1000.0): Spring constant in N/m, range [100, 10000]
 * - damping (double, default: 10.0): Damping coefficient, range [0, 100]  
 * - radius (double, default: 0.02): Coil radius in meters, range [0.005, 0.1]
 * - height (double, default: 0.1): Rest height in meters, range [0.01, 1.0]
 * - coils (int, default: 8): Number of coil turns, range [3, 50]
 * - resolution (int, default: 16): Mesh resolution per coil, range [4, 32]
 */
class SpringObject : public CustomObjectInterface {
    // ...
};
```

This comprehensive parameter system allows users to fine-tune object behavior through XML configuration without requiring code modifications, making the system highly flexible and user-friendly.