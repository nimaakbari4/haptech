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

    // Keep the visual mesh at the device pose
    void updateToolImagePosition() override;

    // Aggregate forces from all sampled haptic points
    void computeInteractionForces() override;

    // Render haptic points and the visual mesh
    void render(cRenderOptions& a_options) override;

    // Replace the visual and resample without touching device control
    bool setVisual(cGenericObject* visualObject, int numSamples, double contactRadius);

private:
    struct Sample {
        cVector3d localPos;
        cHapticPoint* point;
    };

    std::vector<Sample> m_samples;

    void generateSamplesFromObject(cGenericObject* object, int numRequested);
};

#endif // TOOL_MESH_H


