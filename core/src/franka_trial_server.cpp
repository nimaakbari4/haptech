#include "franka_trial_server.h"

// Inside franka_server.cpp or similar
grpc::Status FrankaTrialServiceImpl::RunTrial(
    grpc::ServerContext* context,
    const TrialRequest* request,
    grpc::ServerWriter<TrialResponseStream>* writer
) {
    TrialResponseStream statusMsg;
    TrialResponseStream dataPointMsg;

    // make sure we can run a trial
    if (!SimulationManager::simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;

        TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
        status->set_status(TrialStatus::ERROR);
        status->set_message("Simulation is not running yet.");
        writer->Write(statusMsg);

        return grpc::Status::OK;
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
    SimulationManager::reset();

    // Change the background color to a light blue
    if (!SimulationManager::setBackgroundColor(204/255, 204/255, 255/255)) {
        TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
        status->set_status(TrialStatus::ERROR);
        status->set_message("Something went wrong loading the trial.");
        writer->Write(statusMsg);
        return grpc::Status::OK;
    }

    // Add sphere - positioned back and to the left
    if (!SimulationManager::addSphere("SMALL_SPHERE", 0.08, -0.2, 0.0, -0.1)) {
        TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
        status->set_status(TrialStatus::ERROR);
        status->set_message("Something went wrong loading the trial.");
        writer->Write(statusMsg);
        return grpc::Status::OK;
    }

    // Add torus - positioned to the right
    if (!SimulationManager::addSphere("BIG_SPHERE", 0.2, -0.3, 0.3, 0.0)) {
        TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
        status->set_status(TrialStatus::ERROR);
        status->set_message("Something went wrong loading the trial.");
        writer->Write(statusMsg);
        return grpc::Status::OK;
    }

    // Add torus - positioned to the right
    if (!SimulationManager::addSphere("MEDIUM_SPHERE", 0.1, -0.1, -0.2, 0.0)) {
        TrialStatusUpdate* status = statusMsg.mutable_statusupdate();
        status->set_status(TrialStatus::ERROR);
        status->set_message("Something went wrong loading the trial.");
        writer->Write(statusMsg);
        return grpc::Status::OK;
    }

    // set trial context
    SimulationManager::trialContext.trialId = request->trial_id();
    SimulationManager::trialContext.trialDuration = request->trialduration();
    SimulationManager::trialContext.time = 0.0;

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

    // go until time limit reached
    while (SimulationManager::trialContext.time < SimulationManager::trialContext.trialDuration) {
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
    SimulationManager::trialContext.trialRunning = false;
    std::cout << "[TRIAL] Trial ended: " << SimulationManager::trialContext.trialId << ", lasted " << SimulationManager::trialContext.time << "s" << std::endl;

    status->set_status(TrialStatus::COMPLETED);
    status->set_message("The trial was completed.");
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