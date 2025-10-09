#include "franka_trial_server.h"

// Inside franka_server.cpp or similar
grpc::Status FrankaTrialServiceImpl::RunTrial(
    grpc::ServerContext* context,
    const TrialRequest* request,
    grpc::ServerWriter<TrialResponseStream>* writer
) {
    TrialResponseStream statusMsg;
    TrialResponseStream dataPointMsg;

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
        }
    }
    
    SimulationManager::simContext.objectMap.clear();

    // Change the background color to a light blue
    if (!SimulationManager::setBackgroundColor(204/255, 204/255, 255/255)) {
        TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
        status->set_status(TrialStatus::ERROR);
        status->set_message("Something went wrong loading the trial.");
        writer->Write(statusMsg);
        return grpc::Status::OK;
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
        
        if (!SimulationManager::addObjectFromConfig(obj.id(), obj.type(), 
                                                   obj.position().x(), obj.position().y(), obj.position().z(),
                                                   r, g, b, params)) {
            TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
            status->set_status(TrialStatus::ERROR);
            status->set_message("Something went wrong loading object: " + obj.id());
            writer->Write(statusMsg);
            return grpc::Status::OK;
        }
    }

    // Add trial end conditions
    SimulationManager::trialContext.endConditions.clear();
    for (const auto& cond : request->endconditions()) {
        std::map<std::string, std::string> params;
        for (const auto& param : cond.params()) {
            params[param.key()] = param.value();
        }

        std::unique_ptr<TrialEndCondition> condition = EndConditionRegistry::createCondition(cond.name(), params, cond.success(), cond.priority());

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
        [](const std::unique_ptr<TrialEndCondition>& a, const std::unique_ptr<TrialEndCondition>& b) -> bool {
            return a->priority > b->priority;
    });

    // set trial context
    SimulationManager::trialContext.trialId = request->trial_id();
    SimulationManager::trialContext.time = 0.0;
    SimulationManager::trialContext.endConditionMet = nullptr;

    // start trial
    std::cout << "[TRIAL] Trial started: " << request->trial_id() << " for " << request->trialduration() << "s" << std::endl;

    TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
    status->set_status(TrialStatus::STARTED);
    status->set_message("The trial has started.");
    writer->Write(statusMsg);

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

    // go until trial ends (end conditions are met)
    while (SimulationManager::trialContext.endConditionMet == nullptr) {
        SimulationManager::checkEndConditions();

        if (SimulationManager::trialContext.endConditionMet != nullptr) {
            break;
        }
        
        // get haptic point data
        retrievedProxyPos = hapticPoint->getGlobalPosProxy();
        retrievedComputedForce = hapticPoint->getLastComputedForce();
        retrievedDevicePos = SimulationManager::simContext.tool->getDeviceGlobalPos();

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

        writer->Write(dataPointMsg);

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
            std::cout << "[DEBUG] [" << SimulationManager::trialContext.trialId << "] t = " << SimulationManager::trialContext.time << std::endl;
        }

        double dt = clock.getCurrentTimeSeconds();
        clock.reset();
        SimulationManager::trialContext.time += dt;

        // do not constantly get data, this would overflow everything
        cSleepMs(1000 / DATA_POLL_FREQUENCY);
    }

    // stop trial
    SimulationManager::trialContext.trialRunning == false;
    // ensure haptics is disabled and ramp forces to zero during teardown
    SimulationManager::simContext.hapticsEnabled = false;
    SimulationManager::rampDeviceForceToZero(150);
    std::cout << "[TRIAL] Trial ended: " << SimulationManager::trialContext.trialId << ", lasted " << SimulationManager::trialContext.time << "s" << std::endl;

    status->set_status(TrialStatus::COMPLETED);
    status->set_message(SimulationManager::trialContext.endConditionMet != nullptr ? 
                        "Trial completed: " + SimulationManager::trialContext.endConditionMet->getMessage() : 
                        "Trial completed.");
    writer->Write(statusMsg);

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