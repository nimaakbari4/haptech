#include "tool_mesh.h"
#include "simulation_manager.h"

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
        // Add image as a child of the tool so its global position updates correctly
        addChild(m_image);
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

void cToolMesh::setGravityPhysics(double gravityScale)
{
    // Gravity-based free-fall mode disabled; keep API for compatibility.
    (void)gravityScale;
    m_gravityScale = 0.0;
    m_physicsEnabled = false;
}

void cToolMesh::updateToolImagePosition()
{
    // Position visuals at the proxy (goal) position for realistic collision rendering
    if (m_image && !m_samples.empty()) {
        // Average all proxy positions to get the visual center
        cVector3d avgProxyPos(0, 0, 0);
        int validProxies = 0;
        
        for (auto& s : m_samples) {
            if (s.point) {
                cVector3d proxyGlobal = s.point->getGlobalPosProxy();
                avgProxyPos.add(proxyGlobal);
                validProxies++;
            }
        }
        
        if (validProxies > 0) {
            avgProxyPos.div(validProxies);
            // Convert global proxy position to local coordinates relative to tool
            cVector3d localProxyPos = avgProxyPos - getGlobalPos();
            m_image->setLocalPos(localProxyPos);
        } else {
            // Fallback: use device position if no valid proxies
            m_image->setLocalPos(m_deviceLocalPos);
        }
        
        // Keep device rotation
        m_image->setLocalRot(m_deviceGlobalRot);
    }
}

void cToolMesh::updateFromDevice()
{
    // Always update device state first
    cGenericTool::updateFromDevice();
    
    // Cache tool's global transform for coordinate conversions
    // This is safe because updateFromDevice() is called after computeGlobalPositions()
    m_toolGlobalPos = getGlobalPos();
    m_toolGlobalRot = getGlobalRot();
    
    // Don't update visual position here - it will be updated in render()
    // after computeInteractionForces() has updated all proxy positions
}

void cToolMesh::setGravityForceScale(double scale)
{
    // Extra gravity force-on-device mode disabled; keep API for compatibility.
    (void)scale;
    m_gravityForceScale = 0.0;
}

void cToolMesh::setTetherEnabled(bool enabled)
{
    m_tetherEnabled = enabled;
}

void cToolMesh::setTetherParams(double mass, double stiffness, double damping, double gravityScale, double maxForce)
{
    m_tetherMassKg = (mass > 0.0 ? mass : 0.05);
    m_tetherStiffness = (stiffness > 0.0 ? stiffness : 50.0);
    m_tetherDamping = (damping >= 0.0 ? damping : 2.0);
    m_tetherGravityScale = (gravityScale >= 0.0 ? gravityScale : 1.0);
    m_tetherMaxForce = (maxForce > 0.0 ? maxForce : 30.0);
}

void cToolMesh::setTetherAnchorId(const std::string& anchorId)
{
    m_tetherAnchorId = anchorId;
    m_tetherAnchorObj = nullptr; // will be resolved lazily in updateFromDevice()
}

void cToolMesh::computeInteractionForces()
{
    cVector3d accumulatedForce(0, 0, 0);
    cVector3d accumulatedTorque(0, 0, 0);

    // Tool pose for interaction forces always follows the device/proxy.
    cVector3d toolPos = m_deviceGlobalPos;
    cVector3d toolVel = m_deviceGlobalLinVel;
    cMatrix3d toolRot = m_deviceGlobalRot;

    for (auto& s : m_samples) {
        cVector3d sampleWorldPos = toolPos + cMul(toolRot, s.localPos);
        cVector3d f = s.point->computeInteractionForces(
            sampleWorldPos,
            toolRot,
            toolVel,
            m_deviceGlobalAngVel);

        accumulatedForce.add(f);
        accumulatedTorque.add(cCross(sampleWorldPos - toolPos, f));
    }
    
    // Scale force by number of samples to prevent force multiplication
    // Multiple contact points should distribute, not amplify force
    int numSamples = (int)m_samples.size();
    if (numSamples > 1) {
        accumulatedForce.mul(1.0 / numSamples);
        accumulatedTorque.mul(1.0 / numSamples);
    }
    
    double factor = m_workspaceScaleFactor;
    if (factor > 0.0) {
        accumulatedTorque.mul(1.0 / factor);
    }

    // Clamp force magnitude to safe limits
    double forceMag = accumulatedForce.length();
    double maxSafeForce = 20.0; // Conservative 20N limit for robot safety
    if (m_hapticDevice) {
        cHapticDeviceInfo info = m_hapticDevice->getSpecifications();
        double deviceMax = info.m_maxLinearForce;
        if (deviceMax > 0.0) {
            maxSafeForce = std::min(maxSafeForce, 0.7 * deviceMax); // 70% of device max
        }
    }
    if (forceMag > maxSafeForce && forceMag > C_SMALL) {
        accumulatedForce.mul(maxSafeForce / forceMag);
    }

    // Apply force smoothing to reduce sudden spikes
    if (m_hasPrevForce) {
        const double alpha = std::max(0.0, std::min(1.0, m_forceSmoothingAlpha));
        accumulatedForce = (alpha * accumulatedForce) + ((1.0 - alpha) * m_prevForce);
    }
    m_prevForce = accumulatedForce;
    m_hasPrevForce = true;

    // Send environment force directly to the device (CHAI3D default behavior)
    setDeviceGlobalForce(accumulatedForce);
    setDeviceGlobalTorque(accumulatedTorque);

    // Tether: add a force-only spring-damper between device and anchor (if enabled)
    if (m_tetherEnabled && m_tetherStiffness > 0.0 && m_tetherMaxForce > 0.0) {
        // Resolve anchor position: either a named scene object or the device itself.
        cVector3d anchorPos = m_deviceGlobalPos;
        cVector3d anchorVel(0.0, 0.0, 0.0);

        if (!m_tetherAnchorId.empty()) {
            // Lazy resolve anchor object pointer
            if (!m_tetherAnchorObj) {
                std::lock_guard<std::mutex> lock(SimulationManager::simContext.objectMapMutex);
                auto it = SimulationManager::simContext.objectMap.find(m_tetherAnchorId);
                if (it != SimulationManager::simContext.objectMap.end()) {
                    m_tetherAnchorObj = it->second.get();
                }
            }
            if (m_tetherAnchorObj) {
                anchorPos = m_tetherAnchorObj->getGlobalPos();
                // Anchor velocity is unknown; treat as static for now.
                anchorVel.set(0.0, 0.0, 0.0);
            }
        }

        // Spring-damper force: F = k*(x_anchor - x_device) - c * v_device
        cVector3d displacement = anchorPos - m_deviceGlobalPos;
        cVector3d vel = m_deviceGlobalLinVel;
        cVector3d springForce = m_tetherStiffness * displacement;
        cVector3d dampingForce = m_tetherDamping * (-vel);
        cVector3d tetherForce = springForce + dampingForce;

        // Clamp force magnitude for safety
        double forceMag = tetherForce.length();
        if (forceMag > m_tetherMaxForce && forceMag > C_SMALL) {
            tetherForce.mul(m_tetherMaxForce / forceMag);
        }

        // Add tether force to whatever force CHAI3D computed
        cVector3d current = getDeviceGlobalForce();
        current.add(tetherForce);
        setDeviceGlobalForce(current);
    }
}

void cToolMesh::render(cRenderOptions& a_options)
{
    // Update visual position to follow proxy (after computeInteractionForces has run)
    updateToolImagePosition();
    
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

    // Remove old image from scene graph if it exists
    if (m_image) {
        removeChild(m_image);
    }

    // Replace visual
    // If previous m_image was allocated outside, caller should ensure ownership.
    m_image = visualObject;
    if (m_image) {
        m_image->setLocalPos(0, 0, 0);
        m_image->setLocalRot(cIdentity3d());
        // Add image as a child of the tool so its global position updates correctly
        addChild(m_image);
    }

    // Re-sample
    generateSamplesFromObject(visualObject, numSamples);
    setRadius(contactRadius);

    return true;
}


