#include "franka_trial_server.h"

// Inside franka_server.cpp or similar
grpc::Status FrankaTrialServiceImpl::RunTrial(
    grpc::ServerContext* context,
    const TrialRequest* request,
    grpc::ServerWriter<TrialResponseStream>* writer
) {
    TrialResponseStream statusMsg;
    TrialResponseStream dataPointMsg;
    bool clientCancelled = false;

    std::cout << "[SERVER] simContext address: " << &SimulationManager::simContext << std::endl;

    // ensure simulation is running (allow brief startup lag)
    if (!SimulationManager::simContext.simulationRunning) {
        const double timeoutSec = 2.0;
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
    // ensure haptics is disabled and ramp forces to zero during teardown
    SimulationManager::simContext.hapticsEnabled = false;
    SimulationManager::rampDeviceForceToZero(150);

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
    return grpc::Status::OK;
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