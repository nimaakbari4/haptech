#include "franka_trial_server.h"

// Inside franka_server.cpp or similar
grpc::Status FrankaTrialServiceImpl::RunTrial(
    grpc::ServerContext* context,
    const TrialRequest* request,
    grpc::ServerWriter<TrialResponseStream>* writer
) {
    try {
    TrialResponseStream statusMsg;
    TrialResponseStream dataPointMsg;
    bool clientCancelled = false;

    std::cout << "[SERVER] simContext address: " << &SimulationManager::simContext << std::endl;

    // ensure simulation is running (allow brief startup lag)
    if (!SimulationManager::simContext.simulationRunning) {
        const double timeoutSec = 5.0;
        cPrecisionClock waitClock; waitClock.start();
        while (!SimulationManager::simContext.simulationRunning && waitClock.getCurrentTimeSeconds() < timeoutSec) {
            cSleepMs(10);
        }

        std::cout << "[DEBUG] [server] simulationRunning: " << SimulationManager::simContext.simulationRunning << std::endl;

        if (!SimulationManager::simContext.simulationRunning) {
            std::cout << "[ERROR] Simulation is not running." << std::endl;

            TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
            status->set_status(TrialStatus::ERROR);
            status->set_message("Simulation is not running yet.");
            writer->Write(statusMsg);
            return grpc::Status::OK;
        }
    }

    if (SimulationManager::trialContext.trialRunning) {
        std::cout << "[ERROR] There is already a trial running." << std::endl;

        TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
        status->set_status(TrialStatus::ERROR);
        status->set_message("There is already a trial running.");
        writer->Write(statusMsg);

        return grpc::Status::OK;
    }

        // Safely pause haptics before scene mutations
        SimulationManager::simContext.hapticsEnabled = false;
        cSleepMs(100);
    
        // Safely remove all existing objects (uses SimulationManager's safe path)
        SimulationManager::removeAllObjects();

    // add objects (hard coded for now, load from config info later)
    // SimulationManager::reset();
    // Remove all objects in the object map from the world
    for (auto& pair : SimulationManager::simContext.objectMap) {
        if (pair.second) {
            SimulationManager::simContext.world->removeChild(pair.second.get());
            SimulationManager::simContext.objectInterfaceMap.erase(pair.first);
        }
    }
    
    SimulationManager::simContext.objectMap.clear();
    
    // Clear tool state NOW after all old objects are removed, BEFORE new objects load
    std::cout << "[SETUP] Clearing tool state after removing old objects..." << std::endl;
    if (SimulationManager::simContext.tool) {
        SimulationManager::simContext.tool->initialize();
    }
    cSleepMs(50); // Let tool state fully clear

    // Set background color from scene settings
    if (request->has_scenesettings() && request->scenesettings().has_backgroundcolor()) {
        const auto& bg_color = request->scenesettings().backgroundcolor();
        if (!SimulationManager::setBackgroundColor(bg_color.r(), bg_color.g(), bg_color.b())) {
            TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
            status->set_status(TrialStatus::ERROR);
            status->set_message("Something went wrong setting background color.");
            writer->Write(statusMsg);
            return grpc::Status::OK;
        }
        std::cout << "[INFO] Background color set to RGB(" << bg_color.r() << ", " << bg_color.g() << ", " << bg_color.b() << ")" << std::endl;
    } else {
        // Default background color if not specified
        if (!SimulationManager::setBackgroundColor(0.0, 0.0, 0.0)) {
            TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
            status->set_status(TrialStatus::ERROR);
            status->set_message("Something went wrong setting default background color.");
            writer->Write(statusMsg);
            return grpc::Status::OK;
        }
        std::cout << "[INFO] Using default background color (black)" << std::endl;
    }

    // Configure camera from scene settings
    if (request->has_scenesettings() && request->scenesettings().has_camera()) {
        const auto& cam = request->scenesettings().camera();

        double px = cam.has_position() ? cam.position().x() : 1.0;
        double py = cam.has_position() ? cam.position().y() : 0.0;
        double pz = cam.has_position() ? cam.position().z() : 0.0;

        double lx = cam.has_lookat() ? cam.lookat().x() : 0.0;
        double ly = cam.has_lookat() ? cam.lookat().y() : 0.0;
        double lz = cam.has_lookat() ? cam.lookat().z() : 0.0;

        double ux = cam.has_up() ? cam.up().x() : 0.0;
        double uy = cam.has_up() ? cam.up().y() : 0.0;
        double uz = cam.has_up() ? cam.up().z() : 1.0;

        if (!SimulationManager::setCamera(px, py, pz, lx, ly, lz, ux, uy, uz)) {
            TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
            status->set_status(TrialStatus::ERROR);
            status->set_message("Failed to apply camera settings.");
            writer->Write(statusMsg);
            return grpc::Status::OK;
        }

        std::cout << "[INFO] Camera set to position (" << px << ", " << py << ", " << pz
                  << ") looking at (" << lx << ", " << ly << ", " << lz << ")" << std::endl;
    }

    // Clear any existing text overlays
    SimulationManager::clearTextOverlays();

    // Add text overlays from scene settings
    if (request->has_scenesettings()) {
        for (const auto& overlay : request->scenesettings().textoverlays()) {
            double r = overlay.has_color() ? overlay.color().r() : 1.0;
            double g = overlay.has_color() ? overlay.color().g() : 1.0;
            double b = overlay.has_color() ? overlay.color().b() : 1.0;
            double fontSize = overlay.fontsize() > 0 ? overlay.fontsize() : 20.0;
            
            if (!SimulationManager::addTextOverlay(overlay.text(), overlay.x(), overlay.y(), fontSize, r, g, b)) {
                TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
                status->set_status(TrialStatus::ERROR);
                status->set_message("Failed to add text overlay: " + overlay.text());
                writer->Write(statusMsg);
                return grpc::Status::OK;
            }
        }
        std::cout << "[INFO] Added " << request->scenesettings().textoverlays_size() << " text overlay(s)" << std::endl;
    }

    // Add objects from the request configuration
    for (const auto& obj : request->objects()) {
        std::map<std::string, std::string> params;
        
        for (const auto& param : obj.params()) {
            params[param.key()] = param.value();
        }
        
        // Handle cursor configuration objects specially
        // bool isCursor = (obj.type() == "cursor") || (params.find("as_cursor") != params.end() && (params["as_cursor"] == "true" || params["as_cursor"] == "1"));
        // if (isCursor) {
        //     // Determine visual type from params (visual_type or type key)
        //     std::string visualType;
        //     auto itVT = params.find("visual_type");
        //     if (itVT != params.end()) visualType = itVT->second; else {
        //         auto itType = params.find("type");
        //         if (itType != params.end()) visualType = itType->second; else visualType = "";
        //     }
        //     if (!SimulationManager::applyCursorConfig(visualType, params)) {
        //         TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
        //         status->set_status(TrialStatus::ERROR);
        //         status->set_message("Failed to apply cursor configuration");
        //         writer->Write(statusMsg);
        //         return grpc::Status::OK;
        //     }
        //     // Do not add this as a world object
        //     continue;
        // }
        
        // Get color values
        double r = obj.has_color() ? obj.color().r() : 1.0;
        double g = obj.has_color() ? obj.color().g() : 1.0;
        double b = obj.has_color() ? obj.color().b() : 1.0;

        // debug color
        std::cout << "[DEBUG] Object color - R: " << r << ", G: " << g << ", B: " << b << std::endl;
        
        if (!SimulationManager::addObjectFromConfig(obj.id(), obj.type(), 
                                                   obj.position().x(), obj.position().y(), obj.position().z(),
                                                   obj.orientation().x(), obj.orientation().y(), obj.orientation().z(),
                                                   r, g, b, params)) {
            TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
            status->set_status(TrialStatus::ERROR);
            status->set_message("Something went wrong loading object: " + obj.id());
            writer->Write(statusMsg);
            return grpc::Status::OK;
        }
    }

    // DO NOT enable haptics here - objects might be at device position
    // Haptics will be enabled later after proper initialization

    // Add trial start conditions
    SimulationManager::trialContext.startConditions.clear();
    std::cout << "[DEBUG] Adding " << request->startconditions_size() << " start conditions." << std::endl;
    for (const auto& cond : request->startconditions()) {
        std::map<std::string, std::string> params;
        for (const auto& param : cond.params()) {
            params[param.key()] = param.value();
        }

        std::unique_ptr<TrialConditionInterface> condition = TrialConditionRegistry::createCondition(cond.name(), params, cond.success(), cond.priority());

        if (condition != nullptr) {
            SimulationManager::trialContext.startConditions.push_back(std::move(condition));
        } else {
            TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
            status->set_status(TrialStatus::ERROR);
            status->set_message("Unknown start condition: " + cond.name());
            writer->Write(statusMsg);
            return grpc::Status::OK;
        }
    }

    // Add trial end conditions
    SimulationManager::trialContext.endConditions.clear();
    std::cout << "[DEBUG] Adding " << request->endconditions_size() << " end conditions." << std::endl;
    for (const auto& cond : request->endconditions()) {
        std::map<std::string, std::string> params;
        for (const auto& param : cond.params()) {
            params[param.key()] = param.value();
        }

        std::unique_ptr<TrialConditionInterface> condition = TrialConditionRegistry::createCondition(cond.name(), params, cond.success(), cond.priority());

        if (condition != nullptr) {
            SimulationManager::trialContext.endConditions.push_back(std::move(condition));
        } else {
            TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
            status->set_status(TrialStatus::ERROR);
            status->set_message("Unknown end condition: " + cond.name());
            writer->Write(statusMsg);
            return grpc::Status::OK;
        }
    }

    // Sort conditions by priority
    std::sort(SimulationManager::trialContext.endConditions.begin(), SimulationManager::trialContext.endConditions.end(),
        [](const std::unique_ptr<TrialConditionInterface>& a, const std::unique_ptr<TrialConditionInterface>& b) -> bool {
            return a->priority > b->priority;
    });

    // set trial context
    SimulationManager::trialContext.trialId = request->trial_id();
    SimulationManager::trialContext.time = 0.0;
    SimulationManager::trialContext.startTime = 0.0;
    SimulationManager::trialContext.endConditionMet = nullptr;
    SimulationManager::trialContext.startConditionMet = nullptr;

    // start trial
    std::cout << "[TRIAL] Trial created: " << request->trial_id() << " (start conditions: " << request->startconditions_size() << " condition(s), end conditions: " << request->endconditions_size() << " condition(s))" << std::endl;

    // Update world transforms to ensure all NEW objects are in their final positions
    std::cout << "[TRIAL] Updating world transforms for new objects..." << std::endl;
    SimulationManager::simContext.world->computeGlobalPositions(true);
    
    // Clear tool state AGAIN after new objects are positioned to ensure clean start
    std::cout << "[TRIAL] Final tool contact state clear before enabling haptics..." << std::endl;
    if (SimulationManager::simContext.tool) {
        // Initialize clears all haptic points and contact states
        SimulationManager::simContext.tool->initialize();
        std::cout << "[TRIAL] Tool contact state cleared" << std::endl;
        
        // Enable force rise to prevent sudden force spikes on first contact
        SimulationManager::simContext.tool->setUseForceRise(true);
        SimulationManager::simContext.tool->setRiseTime(0.3); // 300ms gradual ramp
        std::cout << "[TRIAL] Enabled force rise with 300ms ramp time" << std::endl;
    }
    
    // Set force to zero before enabling
    if (SimulationManager::simContext.hapticDevice) {
        SimulationManager::simContext.hapticDevice->setForce(cVector3d(0, 0, 0));
        cVector3d currentForce;
        SimulationManager::simContext.hapticDevice->getForce(currentForce);
        std::cout << "[TRIAL] Force before enabling: " << currentForce.str(3) << " (mag: " << currentForce.length() << "N)" << std::endl;
    }
    
    // DON'T call updateFromDevice here - it would trigger collision detection
    // The haptics loop will call it on the first cycle after enabling
    
    // Enable haptics for this trial AFTER all objects are positioned and tool is cleared
    std::cout << "[TRIAL] All objects positioned and tool cleared - enabling haptics" << std::endl;
    SimulationManager::simContext.hapticsEnabled = true;
    
    // Give haptics loop a few cycles to initialize properly with zero forces
    std::cout << "[TRIAL] Waiting for haptics to stabilize..." << std::endl;
    cSleepMs(100); // ~100 cycles at 1kHz

    TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
    status->set_status(TrialStatus::STARTED);
    status->set_message("The trial has been created. Waiting for trial start.");
    if (!writer->Write(statusMsg)) {
        std::cout << "[WARN] Client disconnected before trial could start." << std::endl;
        SimulationManager::trialContext.trialRunning = false;
        SimulationManager::reset();
        return grpc::Status(grpc::StatusCode::CANCELLED, "Client disconnected");
    }

    SimulationManager::trialContext.trialRunning = true;

    cPrecisionClock clock;
    clock.start();

    auto hapticPoint = SimulationManager::simContext.tool->getHapticPoint(0);

    if (!hapticPoint) {
        std::cout << "[ERROR] Haptic point not found." << std::endl;

        TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
        status->set_status(TrialStatus::ERROR);
        status->set_message("Haptic point not found.");
        writer->Write(statusMsg);

        return grpc::Status::OK;
    }
    cVector3d retrievedProxyPos;
    cVector3d retrievedComputedForce;
    cVector3d retrievedDevicePos;
    bool updatedTrialStart = false;
    cVector3d retrievedDeviceVel;
    cVector3d retrievedDeviceAngVel;
    cMatrix3d retrievedDeviceRot;

    // go until trial ends (end conditions are met)
    while (SimulationManager::trialContext.startConditionMet == nullptr ||
        SimulationManager::trialContext.endConditionMet == nullptr) {
        if (SimulationManager::trialContext.startConditionMet == nullptr) {
            SimulationManager::checkStartConditions();
        } else {
            SimulationManager::checkEndConditions();
        }

        if (!updatedTrialStart && SimulationManager::trialContext.startConditionMet != nullptr) {
            std::cout << "[TRIAL] Trial started after " << std::to_string(SimulationManager::trialContext.time) << " seconds: " << SimulationManager::trialContext.startConditionMet->getMessage() << std::endl;

            TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
            status->set_status(TrialStatus::STARTED);
            status->set_message("Trial started after " + std::to_string(SimulationManager::trialContext.time) + " seconds: " + SimulationManager::trialContext.startConditionMet->getMessage());
            if (!writer->Write(statusMsg)) {
                std::cout << "[WARN] Client disconnected while waiting for trial start." << std::endl;
                clientCancelled = true;
                break;
            }
            updatedTrialStart = true;
            SimulationManager::trialContext.startTime = SimulationManager::trialContext.time;
        }

        if (clientCancelled ||
            (SimulationManager::trialContext.startConditionMet != nullptr &&
             SimulationManager::trialContext.endConditionMet != nullptr)) {
            break;
        }
        
        // get haptic point data
        retrievedProxyPos = hapticPoint->getGlobalPosProxy();
        retrievedComputedForce = hapticPoint->getLastComputedForce();
        retrievedDevicePos = SimulationManager::simContext.tool->getDeviceGlobalPos();
        retrievedDeviceVel = SimulationManager::simContext.tool->getDeviceGlobalLinVel();
        retrievedDeviceAngVel = SimulationManager::simContext.tool->getDeviceGlobalAngVel();
        retrievedDeviceRot = SimulationManager::simContext.tool->getDeviceGlobalRot();

        // Clear the previous data point to prevent accumulation
        dataPointMsg.Clear();
        TrialDataPoint* dataPoint = dataPointMsg.mutable_datapoint();

        dataPoint->set_timestamp(SimulationManager::trialContext.time);

        Vector3* devicePos = dataPoint->mutable_deviceposition();
        devicePos->set_x(retrievedDevicePos.x());
        devicePos->set_y(retrievedDevicePos.y());
        devicePos->set_z(retrievedDevicePos.z());

        Vector3* proxyPos = dataPoint->mutable_proxyposition();
        proxyPos->set_x(retrievedProxyPos.x());
        proxyPos->set_y(retrievedProxyPos.y());
        proxyPos->set_z(retrievedProxyPos.z());

        Vector3* force = dataPoint->mutable_force();
        force->set_x(retrievedComputedForce.x());
        force->set_y(retrievedComputedForce.y());
        force->set_z(retrievedComputedForce.z());

        // Device kinematics
        Vector3* devVel = dataPoint->mutable_devicevelocity();
        devVel->set_x(retrievedDeviceVel.x());
        devVel->set_y(retrievedDeviceVel.y());
        devVel->set_z(retrievedDeviceVel.z());

        Vector3* devAngVel = dataPoint->mutable_deviceangularvelocity();
        devAngVel->set_x(retrievedDeviceAngVel.x());
        devAngVel->set_y(retrievedDeviceAngVel.y());
        devAngVel->set_z(retrievedDeviceAngVel.z());

        // Orientation as quaternion
        cQuaternion q; q.fromRotMat(retrievedDeviceRot);
        Quaternion* devQuat = dataPoint->mutable_deviceorientation();
        devQuat->set_x(q.x);
        devQuat->set_y(q.y);
        devQuat->set_z(q.z);
        devQuat->set_w(q.w);

        // Object states snapshot
        for (const auto& kv : SimulationManager::simContext.objectMap) {
            const std::string& oid = kv.first;
            cGenericObject* objPtr = kv.second.get();
            if (!objPtr) continue;
            ObjectState* os = dataPoint->add_objects();
            os->set_id(oid);
            cVector3d p = objPtr->getGlobalPos();
            Vector3* osp = os->mutable_position();
            osp->set_x(p.x()); osp->set_y(p.y()); osp->set_z(p.z());
            cMatrix3d R = objPtr->getGlobalRot();
            cQuaternion oq; oq.fromRotMat(R);
            Quaternion* oso = os->mutable_orientation();
            oso->set_x(oq.x); oso->set_y(oq.y); oso->set_z(oq.z); oso->set_w(oq.w);
        }

        // Contact data collection
        if (SimulationManager::simContext.tool) {
            // Get the first (and typically only) haptic point
            auto hapticPoint = SimulationManager::simContext.tool->getHapticPoint(0);
            if (hapticPoint && hapticPoint->m_algorithmFingerProxy) {
                // Debug output every 100 samples
                static int sampleCount = 0;
                sampleCount++;
                
                // Use a map to store only ONE contact per object
                // (multiple collision events might reference the same object)
                std::map<std::string, int> contactedObjects;
                
                // CHAI3D stores up to 3 simultaneous collision events per haptic point
                for (int i = 0; i < 3; i++) {
                    auto collisionEvent = hapticPoint->m_algorithmFingerProxy->m_collisionEvents[i];
                    if (!collisionEvent || !collisionEvent->m_object) continue;
                    
                    // Find the object ID for this collision
                    std::string contactObjectId;
                    for (const auto& kv : SimulationManager::simContext.objectMap) {
                        if (kv.second.get() == collisionEvent->m_object) {
                            contactObjectId = kv.first;
                            break;
                        }
                    }
                    
                    if (contactObjectId.empty()) continue;
                    
                    // Skip if we've already processed a contact for this object
                    if (contactedObjects.find(contactObjectId) != contactedObjects.end()) continue;
                    contactedObjects[contactObjectId] = i;
                    
                    // Create contact message
                    Contact* contact = dataPoint->add_contacts();
                    contact->set_objectid(contactObjectId);
                    
                    // Contact point in global coordinates
                    cVector3d contactPt = collisionEvent->m_globalPos;
                    Vector3* cp = contact->mutable_point();
                    cp->set_x(contactPt.x());
                    cp->set_y(contactPt.y());
                    cp->set_z(contactPt.z());
                    
                    // Contact normal in global coordinates
                    cVector3d contactNorm = collisionEvent->m_globalNormal;
                    if (contactNorm.length() > 0.001) {
                        contactNorm.normalize();
                    }
                    Vector3* cn = contact->mutable_normal();
                    cn->set_x(contactNorm.x());
                    cn->set_y(contactNorm.y());
                    cn->set_z(contactNorm.z());
                    
                    // Penetration depth (distance from proxy to device)
                    double penetration = sqrt(collisionEvent->m_squareDistance);
                    contact->set_penetrationdepth(penetration);
                }
                
                // Debug output
                if (contactedObjects.size() > 0 && sampleCount % 100 == 1) {
                    std::cout << "\n[SERVER-DEBUG] t=" << SimulationManager::trialContext.time 
                              << "s: " << contactedObjects.size() << " unique object(s) in contact" << std::endl;
                    
                    // Show device and proxy positions
                    cVector3d devPos = SimulationManager::simContext.tool->getDeviceGlobalPos();
                    cVector3d proxyPos = hapticPoint->getGlobalPosProxy();
                    std::cout << "[SERVER-DEBUG] Device pos: " << devPos.str(3) << std::endl;
                    std::cout << "[SERVER-DEBUG] Proxy pos: " << proxyPos.str(3) << std::endl;
                    
                    // Show each contacted object and its position
                    for (const auto& kv : contactedObjects) {
                        auto objIt = SimulationManager::simContext.objectMap.find(kv.first);
                        if (objIt != SimulationManager::simContext.objectMap.end()) {
                            cVector3d objPos = objIt->second->getGlobalPos();
                            std::cout << "[SERVER-DEBUG]   Contact with '" << kv.first 
                                      << "' at pos: " << objPos.str(3) << std::endl;
                        }
                    }
                }
            }
        }

        if (context->IsCancelled()) {
            std::cout << "[WARN] Client cancelled trial stream." << std::endl;
            clientCancelled = true;
            break;
        }

        if (!writer->Write(dataPointMsg)) {
            std::cout << "[WARN] Failed to write to client stream (likely disconnected)." << std::endl;
            clientCancelled = true;
            break;
        }

        /*
        // Check if cursor reached goal (look for goal object at x=0.5)
        auto goalIt = SimulationManager::simContext.objectMap.find("goal");
        if (goalIt != SimulationManager::simContext.objectMap.end()) {
            cVector3d goalPos = goalIt->second->getGlobalPos();
            double dx = retrievedProxyPos.x() - goalPos.x();
            double dy = retrievedProxyPos.y() - goalPos.y();
            double dz = retrievedProxyPos.z() - goalPos.z();
            double dist = sqrt(dx*dx + dy*dy + dz*dz);
            if (dist < 0.05) { // 5cm tolerance
                reachedGoal = true;
            }
        }
        */

        if (SimulationManager::simContext.debug) {
        }

        double dt = clock.getCurrentTimeSeconds();
        clock.reset();
        SimulationManager::trialContext.time += dt;

        // do not constantly get data, this would overflow everything
        cSleepMs(1000 / DATA_POLL_FREQUENCY);
    }

    // stop trial
    SimulationManager::trialContext.trialRunning = false;
    std::cout << "[TEARDOWN] Trial stopping, beginning force teardown sequence" << std::endl;
    
    // Read current force before any teardown
    cVector3d forceBeforeTeardown(0, 0, 0);
    if (SimulationManager::simContext.hapticDevice) {
        SimulationManager::simContext.hapticDevice->getForce(forceBeforeTeardown);
        std::cout << "[TEARDOWN] Current force: " << forceBeforeTeardown.str(3) 
                  << " (mag: " << forceBeforeTeardown.length() << "N)" << std::endl;
    }
    
    // Graceful force teardown to prevent controller_torque_discontinuity
    // Clear tool's internal contact state so it stops computing forces
    std::cout << "[TEARDOWN] Clearing tool contacts..." << std::endl;
    if (SimulationManager::simContext.tool) {
        SimulationManager::simContext.tool->initialize();
    }
    
    // Let haptics loop continue running for several cycles with cleared contacts
    // This allows forces to naturally decay to zero through smoothing and force rise
    std::cout << "[TEARDOWN] Waiting 150ms for forces to decay naturally..." << std::endl;
    cSleepMs(150); // ~150 haptic cycles at 1kHz
    
    cVector3d forceAfterDecay(0, 0, 0);
    if (SimulationManager::simContext.hapticDevice) {
        SimulationManager::simContext.hapticDevice->getForce(forceAfterDecay);
        std::cout << "[TEARDOWN] Force after decay: " << forceAfterDecay.str(3) 
                  << " (mag: " << forceAfterDecay.length() << "N)" << std::endl;
    }
    
    // Now safe to disable haptics - forces should already be near zero
    std::cout << "[TEARDOWN] Disabling haptics..." << std::endl;
    SimulationManager::simContext.hapticsEnabled = false;
    
    // Final safety: manually ramp any remaining forces
    std::cout << "[TEARDOWN] Ramping remaining forces to zero..." << std::endl;
    SimulationManager::rampDeviceForceToZero(100);
    
    cVector3d forceFinal(0, 0, 0);
    if (SimulationManager::simContext.hapticDevice) {
        SimulationManager::simContext.hapticDevice->getForce(forceFinal);
        std::cout << "[TEARDOWN] Final force: " << forceFinal.str(3) 
                  << " (mag: " << forceFinal.length() << "N)" << std::endl;
    }
    std::cout << "[TEARDOWN] Force teardown complete" << std::endl;

    // output results
    std::cout << "[TRIAL] Trial results: " << SimulationManager::trialContext.trialId << std::endl;
    std::cout << "[TRIAL] Duration: " << SimulationManager::trialContext.time << " seconds" << std::endl;

    if (clientCancelled) {
        std::cout << "[TRIAL] Trial aborted: " << SimulationManager::trialContext.trialId
                  << ", lasted " << SimulationManager::trialContext.time << "s (client disconnected)" << std::endl;
        SimulationManager::reset();
        return grpc::Status(grpc::StatusCode::CANCELLED, "Client disconnected");
    }

    if (SimulationManager::trialContext.endConditionMet != nullptr) {
        std::cout << "[TRIAL] End condition met: " << SimulationManager::trialContext.endConditionMet->getMessage() << std::endl;

        status->set_status(TrialStatus::COMPLETED);
        status->set_message("Trial completed after " + std::to_string(SimulationManager::trialContext.time) + " seconds: " + SimulationManager::trialContext.endConditionMet->getMessage());
        if (!writer->Write(statusMsg)) {
            std::cout << "[WARN] Unable to send completion status to client (disconnected)." << std::endl;
        }
    }

    // reset environment
    SimulationManager::reset();
    SimulationManager::simContext.hapticsEnabled = false;
    SimulationManager::simContext.tool->initialize();
    SimulationManager::simContext.world->computeGlobalPositions(true);
    SimulationManager::simContext.hapticDevice->setForce(cVector3d(0, 0, 0));
    cVector3d currentForce;
    SimulationManager::simContext.hapticDevice->getForce(currentForce);
    std::cout << "[TRIAL] Force after reset: " << currentForce.str(3) 
              << " (mag: " << currentForce.length() << "N)" << std::endl;
    SimulationManager::simContext.hapticsEnabled = true;
    return grpc::Status::OK;
    } catch (const std::exception& ex) {
        TrialResponseStream statusMsg;
        TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
        status->set_status(TrialStatus::ERROR);
        status->set_message(std::string("Unhandled server exception in RunTrial: ") + ex.what());
        // Best-effort write; swallow any errors here
        (void)writer->Write(statusMsg);
        return grpc::Status(grpc::StatusCode::UNKNOWN, "Unexpected error in RPC handling");
    } catch (...) {
        TrialResponseStream statusMsg;
        TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
        status->set_status(TrialStatus::ERROR);
        status->set_message("Unhandled server exception in RunTrial: unknown");
        (void)writer->Write(statusMsg);
        return grpc::Status(grpc::StatusCode::UNKNOWN, "Unexpected error in RPC handling");
    }
}


void runServer() {
    std::string server_address("0.0.0.0:50051");
    FrankaTrialServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());

    std::cout << "[Server] Listening on " << server_address << std::endl;
    server->Wait();
}