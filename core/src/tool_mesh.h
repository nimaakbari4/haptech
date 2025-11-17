#pragma once
#ifndef TOOL_MESH_H
#define TOOL_MESH_H

#include "chai3d.h"

using namespace chai3d;

// Mesh-based multi-point haptic tool. Visuals are provided by m_image; haptics by sampled cHapticPoints.
class cToolMesh : public cGenericTool {
public:
    cToolMesh(cWorld* world, cGenericObject* visualObject,
              int numSamples,
              double contactRadius);

    virtual ~cToolMesh();

    // Keep the visual mesh at the device pose (or physics pose if enabled)
    void updateToolImagePosition() override;
    void updateFromDevice() override;

    // Aggregate forces from all sampled haptic points
    void computeInteractionForces() override;

    // Render haptic points and the visual mesh
    void render(cRenderOptions& a_options) override;

    // Replace the visual and resample without touching device control
    bool setVisual(cGenericObject* visualObject, int numSamples, double contactRadius);

    // Enable/disable simple gravity-driven physics for the cursor tool
    // gravityScale <= 0 disables physics and follows device
    void setGravityPhysics(double gravityScale);
    
    // Apply gravity as an additional force on the tool/device (force-mode)
    // scale <= 0 disables this extra force
    void setGravityForceScale(double scale);
    
    // Enable/disable tethered mass physics (hybrid mode)
    // When enabled, a simulated mass is attached to the device by a spring-damper
    // The mass falls under gravity, and the device receives the reaction force
    void setTetherEnabled(bool enabled);
    
    // Configure tether parameters
    // mass: mass of the tethered object (kg)
    // stiffness: spring constant (N/m)
    // damping: damping coefficient (N·s/m)
    // gravityScale: multiplier for gravity (unitless, 1.0 = 9.81 m/s²)
    // maxForce: maximum force clamp (N) for safety
    void setTetherParams(double mass, double stiffness, double damping, double gravityScale, double maxForce);

private:
    struct Sample {
        cVector3d localPos;
        cHapticPoint* point;
    };

    std::vector<Sample> m_samples;

    void generateSamplesFromObject(cGenericObject* object, int numRequested);

    // Physics state
    bool m_physicsEnabled = false;
    double m_gravityScale = 0.0;
    cVector3d m_velocity{0.0, 0.0, 0.0};
    cVector3d m_physicsPos{0.0, 0.0, 0.0};
    cPrecisionClock m_physicsClock;
    
    // Force-mode gravity (adds to device force; does not affect pose)
    double m_gravityForceScale = 0.0;
    
    // Tethered mass physics (hybrid mode)
    bool m_tetherEnabled = false;
    double m_tetherMassKg = 0.05;          // Default 50g
    double m_tetherStiffness = 50.0;        // Default 50 N/m
    double m_tetherDamping = 2.0;           // Default 2 N·s/m
    double m_tetherGravityScale = 1.0;      // Default 1.0 (9.81 m/s²)
    double m_tetherMaxForce = 30.0;         // Default 30 N safety clamp
    cVector3d m_tetherPos{0.0, 0.0, 0.0};  // Position of tethered mass (world coordinates)
    cVector3d m_tetherVel{0.0, 0.0, 0.0};  // Velocity of tethered mass
    cVector3d m_tetherEnvForce{0.0, 0.0, 0.0}; // Net environment force applied at tethered mass
    cPrecisionClock m_tetherClock;         // Clock for integration
    bool m_tetherInitialized = false;      // Track if tether has been initialized with valid device pos
    cVector3d m_toolGlobalPos{0.0, 0.0, 0.0};  // Cached tool global position for coordinate conversion
    cMatrix3d m_toolGlobalRot{cIdentity3d()};  // Cached tool global rotation for coordinate conversion
};

#endif // TOOL_MESH_H


