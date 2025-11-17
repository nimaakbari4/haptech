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
};

#endif // TOOL_MESH_H


