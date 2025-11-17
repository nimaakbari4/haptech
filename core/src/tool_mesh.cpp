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
    // Position and orient visuals at device/physics/tether pose
    if (m_image) {
        cVector3d posePos;
        cMatrix3d poseRot;
        
        if (m_tetherEnabled) {
            // For tether mode, convert tether world position into the tool's local frame
            posePos = cTranspose(m_toolGlobalRot) * (m_tetherPos - m_toolGlobalPos);
            poseRot = m_deviceGlobalRot;

            // Debug output (first few frames only)
            static int debugFrame = 0;
            if (debugFrame < 5) {
                std::cout << "[VISUAL] Frame " << debugFrame 
                          << " toolGlobal=(" << m_toolGlobalPos.x() << "," << m_toolGlobalPos.y() << "," << m_toolGlobalPos.z() << ")"
                          << " tetherWorld=(" << m_tetherPos.x() << "," << m_tetherPos.y() << "," << m_tetherPos.z() << ")"
                          << " imageLocal=(" << posePos.x() << "," << posePos.y() << "," << posePos.z() << ")"
                          << std::endl;
                debugFrame++;
            }

            m_image->setLocalPos(posePos);
            m_image->setLocalRot(poseRot);
            m_image->computeGlobalPositions(true, m_toolGlobalPos, m_toolGlobalRot);
        } else if (m_physicsEnabled) {
            // Free-fall physics mode has been disabled; fall back to device pose.
            m_image->setLocalPos(m_deviceLocalPos);
            m_image->setLocalRot(m_deviceGlobalRot);
        } else {
            // Device position is already computed relative to tool's global transform
            m_image->setLocalPos(m_deviceLocalPos);
            m_image->setLocalRot(m_deviceGlobalRot);
        }
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
    
    // Handle tether physics (hybrid mode: mass attached to device by spring-damper)
    if (m_tetherEnabled) {
        // Get delta time
        m_tetherClock.stop();
        double dt = m_tetherClock.getCurrentTimeSeconds();
        m_tetherClock.reset();
        m_tetherClock.start();
        
        // Clamp dt to reasonable range (avoid huge jumps or negative values)
        if (dt < 0.0) dt = 0.0;
        if (dt > 0.1) dt = 0.1; // Cap at 100ms for stability
        
        // Initialize tether position on first valid device update
        if (!m_tetherInitialized) {
            m_tetherPos = m_deviceGlobalPos;
            m_tetherVel.set(0, 0, 0);
            m_tetherInitialized = true;
            std::cout << "[TETHER] Initialized at device pos: (" << m_deviceGlobalPos.x() 
                      << ", " << m_deviceGlobalPos.y() << ", " << m_deviceGlobalPos.z() << ")" << std::endl;
        }
        
        if (dt > 1e-6 && m_tetherMassKg > 0.0) {
            // Resolve anchor position: either a named scene object or the device itself.
            cVector3d anchorPos = m_deviceGlobalPos;
            cVector3d anchorVel = m_deviceGlobalLinVel;

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
            
            // Spring-damper force on the mass: F = k*(x_anchor - x_mass) + c*(v_anchor - v_mass)
            cVector3d displacement = anchorPos - m_tetherPos;
            cVector3d velocityDiff = anchorVel - m_tetherVel;
            cVector3d springForce = m_tetherStiffness * displacement;
            cVector3d dampingForce = m_tetherDamping * velocityDiff;
            
            // Gravity force on the mass: F_gravity = m * g * scale
            cVector3d gravityForce(0.0, 0.0, -9.81 * m_tetherGravityScale * m_tetherMassKg);
            
            // Environment/contact force on the mass from previous force computation step
            // (see computeInteractionForces). This makes the tethered mass feel contacts
            // directly instead of sending them straight to the device.
            cVector3d envForce = m_tetherEnvForce;

            // Clamp environment force for stability
            double envMag = envForce.length();
            if (envMag > m_tetherMaxForce && envMag > C_SMALL) {
                envForce.mul(m_tetherMaxForce / envMag);
            }
            
            // Total force on mass
            cVector3d totalForce = springForce + dampingForce + gravityForce + envForce;
            
            // Integrate acceleration: a = F / m
            cVector3d acceleration = totalForce / m_tetherMassKg;
            
            // Semi-implicit Euler integration
            m_tetherVel.add(acceleration * dt);
            m_tetherPos.add(m_tetherVel * dt);
            
            // Debug output (first few frames only)
            static int debugFrame = 0;
            if (debugFrame < 10) {
                std::cout << "[TETHER] Frame " << debugFrame << " dt=" << dt 
                          << " anchor=(" << anchorPos.x() << "," << anchorPos.y() << "," << anchorPos.z() << ")"
                          << " tether=(" << m_tetherPos.x() << "," << m_tetherPos.y() << "," << m_tetherPos.z() << ")"
                          << " gravity=" << gravityForce.z() << " spring=" << springForce.length() << std::endl;
                debugFrame++;
            }
        } else if (dt <= 1e-6) {
            // Warn if dt is too small
            static int warnCount = 0;
            if (warnCount < 3) {
                std::cout << "[TETHER] Warning: dt too small (" << dt << "), skipping integration" << std::endl;
                warnCount++;
            }
        }
        
        // Visual follows tether position
        updateToolImagePosition();
        return;
    }
    
    // Handle old pose-physics mode (decoupled visual)
    if (m_physicsEnabled) {
        // Free-fall physics mode disabled; no integration performed.
    }
    
    // Default: visual follows device
    updateToolImagePosition();
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
    if (m_tetherEnabled) {
        // Initialize tether position from current device pose
        // Ensure we have a valid device position (may be updated later in first frame)
        m_tetherPos = m_deviceGlobalPos;
        m_tetherVel.set(0, 0, 0);
        m_tetherClock.reset();
        m_tetherClock.start();
        m_tetherInitialized = false; // Will be set true on first valid update
        std::cout << "[TETHER] Enabled at device pos: (" << m_deviceGlobalPos.x() 
                  << ", " << m_deviceGlobalPos.y() << ", " << m_deviceGlobalPos.z() << ")" << std::endl;
    } else {
        m_tetherInitialized = false;
    }
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

    // Reset environment force on tethered mass each frame
    m_tetherEnvForce.set(0.0, 0.0, 0.0);

    // Use tether position and velocity for force computation if tether is enabled
    // (forces are computed at the visual cursor position, which is the tethered mass)
    cVector3d toolPos = m_tetherEnabled ? m_tetherPos : m_deviceGlobalPos;
    cVector3d toolVel = m_tetherEnabled ? m_tetherVel : m_deviceGlobalLinVel;
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
    
    double factor = m_workspaceScaleFactor;
    if (factor > 0.0) {
        accumulatedTorque.mul(1.0 / factor);
    }

    if (m_tetherEnabled) {
        // In tether mode, accumulatedForce is the environment force applied at the
        // tethered mass position. Store it so updateFromDevice() can use it in the
        // mass dynamics integration. Do NOT send it directly to the device; the device
        // should only feel the tether spring-damper reaction.
        m_tetherEnvForce = accumulatedForce;
    } else {
        // Non-tethered mode: send environment force directly to the device as usual.
        setDeviceGlobalForce(accumulatedForce);
        setDeviceGlobalTorque(accumulatedTorque);
    }
    
    // Tether reaction force: device receives equal-and-opposite spring-damper force
    if (m_tetherEnabled) {
        cVector3d devicePos = m_deviceGlobalPos;
        cVector3d deviceVel = m_deviceGlobalLinVel;
        
        // Spring-damper force: F = k*(x_mass - x_device) + c*(v_mass - v_device)
        // This is the force the device applies to the mass; reaction is opposite
        cVector3d displacement = m_tetherPos - devicePos;
        cVector3d velocityDiff = m_tetherVel - deviceVel;
        cVector3d springForce = m_tetherStiffness * displacement;
        cVector3d dampingForce = m_tetherDamping * velocityDiff;
        cVector3d tetherForce = springForce + dampingForce;
        
        // Clamp force magnitude for safety
        double forceMag = tetherForce.length();
        if (forceMag > m_tetherMaxForce) {
            tetherForce.mul(m_tetherMaxForce / forceMag);
        }
        
        // Add reaction force to device (equal and opposite to what device applies to mass)
        cVector3d current = getDeviceGlobalForce();
        current.add(tetherForce);

        // Final safety clamp on total device force in tether mode
        double totalMag = current.length();
        if (totalMag > m_tetherMaxForce && totalMag > C_SMALL) {
            current.mul(m_tetherMaxForce / totalMag);
        }
        setDeviceGlobalForce(current);
    }
    
    // Extra gravity force (force-mode) applied to the device/tool
    // Disabled: we only support tether-based physics now.
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


