#include "tool_mesh.h"

using namespace chai3d;

cToolMesh::cToolMesh(cWorld* world, cGenericObject* visualObject,
                     int numSamples,
                     double contactRadius)
    : cGenericTool(world)
{
    m_image = visualObject;

    if (m_image) {
        // Ensure image is not nullptr and belongs to our scene graph
        m_image->setLocalPos(0, 0, 0);
        m_image->setLocalRot(cIdentity3d());
    }

    generateSamplesFromObject(visualObject, numSamples);

    // Set contact/display radii for all haptic points
    setRadius(contactRadius);

    // Hide proxy spheres by default for a clean cursor
    setShowContactPoints(false, false);
}

cToolMesh::~cToolMesh()
{
    // cGenericTool destructor will delete m_image (display mesh) if set via constructor?
    // m_image is managed by the caller/world; do not delete here.
    for (auto& s : m_samples) {
        delete s.point;
    }
    m_samples.clear();
}

void cToolMesh::updateToolImagePosition()
{
    // Position and orient visuals at device pose
    if (m_image) {
        m_image->setLocalPos(m_deviceGlobalPos);
        m_image->setLocalRot(m_deviceGlobalRot);
    }
}

void cToolMesh::computeInteractionForces()
{
    cVector3d accumulatedForce(0, 0, 0);
    cVector3d accumulatedTorque(0, 0, 0);

    cVector3d toolPos = m_deviceGlobalPos;
    cMatrix3d toolRot = m_deviceGlobalRot;

    for (auto& s : m_samples) {
        cVector3d sampleWorldPos = toolPos + cMul(toolRot, s.localPos);
        cVector3d f = s.point->computeInteractionForces(
            sampleWorldPos,
            toolRot,
            m_deviceGlobalLinVel,
            m_deviceGlobalAngVel);

        accumulatedForce.add(f);
        accumulatedTorque.add(cCross(sampleWorldPos - toolPos, f));
    }

    double factor = m_workspaceScaleFactor;
    if (factor > 0.0) {
        accumulatedTorque.mul(1.0 / factor);
    }

    setDeviceGlobalForce(accumulatedForce);
    setDeviceGlobalTorque(accumulatedTorque);
}

void cToolMesh::render(cRenderOptions& a_options)
{
    // Render haptic points (proxy/goal) if enabled
    int numContactPoint = (int)(m_hapticPoints.size());
    for (int i = 0; i < numContactPoint; i++)
    {
        cHapticPoint* nextContactPoint = m_hapticPoints[i];
        nextContactPoint->render(a_options);
    }

    // Render the visual mesh image
    if (m_image != nullptr)
    {
        m_image->renderSceneGraph(a_options);
    }
}

void cToolMesh::generateSamplesFromObject(cGenericObject* object, int numRequested)
{
    std::vector<cVector3d> points;
    if (auto mesh = dynamic_cast<cMultiMesh*>(object)) {
        for (unsigned int mi = 0; mi < mesh->getNumMeshes(); ++mi) {
            cMesh* sub = mesh->getMesh(mi);
            if (!sub || !sub->m_triangles || !sub->m_vertices) continue;
            const size_t triCount = sub->m_triangles->getNumElements();
            if (triCount == 0) continue;
            size_t step = std::max<size_t>(1, triCount / std::max(1, numRequested));
            for (size_t ti = 0; ti < triCount; ti += step) {
                unsigned int ia = sub->m_triangles->getVertexIndex((unsigned int)ti, 0);
                unsigned int ib = sub->m_triangles->getVertexIndex((unsigned int)ti, 1);
                unsigned int ic = sub->m_triangles->getVertexIndex((unsigned int)ti, 2);
                cVector3d va = sub->m_vertices->getLocalPos(ia);
                cVector3d vb = sub->m_vertices->getLocalPos(ib);
                cVector3d vc = sub->m_vertices->getLocalPos(ic);
                points.push_back((va + vb + vc) / 3.0);
                if ((int)points.size() >= numRequested) break;
            }
            if ((int)points.size() >= numRequested) break;
        }
    } else if (auto segs = dynamic_cast<cMultiSegment*>(object)) {
        // Sample along line segments: take every Nth segment midpoint
        const int segmentCount = (int)segs->getNumSegments();
        if (segmentCount > 0) {
            int step = std::max(1, segmentCount / std::max(1, numRequested));
            for (int si = 0; si < segmentCount; si += step) {
                int ia = segs->m_segments->getVertexIndex(si, 0);
                int ib = segs->m_segments->getVertexIndex(si, 1);
                cVector3d va = segs->m_vertices->getLocalPos((unsigned int)ia);
                cVector3d vb = segs->m_vertices->getLocalPos((unsigned int)ib);
                points.push_back((va + vb) * 0.5);
                if ((int)points.size() >= numRequested) break;
            }
        }
    }

    if (points.empty()) {
        points.push_back(cVector3d(0, 0, 0));
    }

    for (const auto& lp : points) {
        cHapticPoint* hp = new cHapticPoint(this);
        m_hapticPoints.push_back(hp);
        m_samples.push_back({ lp, hp });
    }
}

bool cToolMesh::setVisual(cGenericObject* visualObject, int numSamples, double contactRadius)
{
    // Remove old sample points
    for (auto& s : m_samples) {
        delete s.point;
    }
    m_samples.clear();
    m_hapticPoints.clear();

    // Replace visual
    // If previous m_image was allocated outside, caller should ensure ownership.
    m_image = visualObject;
    if (m_image) {
        m_image->setLocalPos(0, 0, 0);
        m_image->setLocalRot(cIdentity3d());
    }

    // Re-sample
    generateSamplesFromObject(visualObject, numSamples);
    setRadius(contactRadius);

    return true;
}


