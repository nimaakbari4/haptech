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

    // Store data collection config (defaults to collecting everything if not specified)
    DataCollectionConfig dataCollectionConfig;
    if (request->has_datacollection()) {
        dataCollectionConfig = request->datacollection();
        std::cout << "[DEBUG] Using custom data collection config" << std::endl;
    } else {
        // Default: collect everything
        dataCollectionConfig.mutable_device()->set_position(true);
        dataCollectionConfig.mutable_device()->set_velocity(true);
        dataCollectionConfig.mutable_device()->set_angularvelocity(true);
        dataCollectionConfig.mutable_device()->set_orientation(true);
        dataCollectionConfig.mutable_device()->set_proxyposition(true);
        dataCollectionConfig.mutable_device()->set_force(true);
        dataCollectionConfig.mutable_objects()->set_position(true);
        dataCollectionConfig.mutable_objects()->set_orientation(true);
        dataCollectionConfig.mutable_objects()->set_contact(true);
        dataCollectionConfig.mutable_objects()->set_contactpoint(true);
        dataCollectionConfig.mutable_objects()->set_contactnormal(true);
        dataCollectionConfig.mutable_objects()->set_penetrationdepth(true);
        dataCollectionConfig.mutable_objects()->set_force(true);
        dataCollectionConfig.mutable_objects()->set_springforce(true);
        dataCollectionConfig.mutable_objects()->set_dampingforce(true);
        std::cout << "[DEBUG] Using default data collection config (collect everything)" << std::endl;
    }

    // set trial context
    SimulationManager::trialContext.trialId = request->trial_id();
    SimulationManager::trialContext.time = 0.0;
    SimulationManager::trialContext.endConditionMet = nullptr;

    // start trial
    if (request->trialduration() > 0) {
        std::cout << "[TRIAL] Trial started: " << request->trial_id() << " for " << request->trialduration() << "s" << std::endl;
    } else {
        std::cout << "[TRIAL] Trial started: " << request->trial_id() << " (end conditions: " << request->endconditions_size() << " condition(s))" << std::endl;
    }

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
    cVector3d retrievedDeviceVel;
    cVector3d retrievedDeviceAngVel;
    cMatrix3d retrievedDeviceRot;

    // go until trial ends (end conditions are met)
    while (SimulationManager::trialContext.endConditionMet == nullptr) {
        SimulationManager::checkEndConditions();

        if (SimulationManager::trialContext.endConditionMet != nullptr) {
            break;
        }
        
        // Collect data based on configuration
        TrialDataPoint* dataPoint = dataPointMsg.mutable_datapoint();
        dataPoint->set_timestamp(SimulationManager::trialContext.time);

        // Device position
        if (dataCollectionConfig.device().position()) {
            retrievedDevicePos = SimulationManager::simContext.tool->getDeviceGlobalPos();
            Vector3* devicePos = dataPoint->mutable_deviceposition();
            devicePos->set_x(retrievedDevicePos.x());
            devicePos->set_y(retrievedDevicePos.y());
            devicePos->set_z(retrievedDevicePos.z());
        }

        // Proxy position
        if (dataCollectionConfig.device().proxyposition()) {
            retrievedProxyPos = hapticPoint->getGlobalPosProxy();
            Vector3* proxyPos = dataPoint->mutable_proxyposition();
            proxyPos->set_x(retrievedProxyPos.x());
            proxyPos->set_y(retrievedProxyPos.y());
            proxyPos->set_z(retrievedProxyPos.z());
        }

        // Force
        if (dataCollectionConfig.device().force()) {
            retrievedComputedForce = hapticPoint->getLastComputedForce();
            Vector3* force = dataPoint->mutable_force();
            force->set_x(retrievedComputedForce.x());
            force->set_y(retrievedComputedForce.y());
            force->set_z(retrievedComputedForce.z());
        }

        // Device velocity
        if (dataCollectionConfig.device().velocity()) {
            retrievedDeviceVel = SimulationManager::simContext.tool->getDeviceGlobalLinVel();
            Vector3* devVel = dataPoint->mutable_devicevelocity();
            devVel->set_x(retrievedDeviceVel.x());
            devVel->set_y(retrievedDeviceVel.y());
            devVel->set_z(retrievedDeviceVel.z());
        }

        // Device angular velocity
        if (dataCollectionConfig.device().angularvelocity()) {
            retrievedDeviceAngVel = SimulationManager::simContext.tool->getDeviceGlobalAngVel();
            Vector3* devAngVel = dataPoint->mutable_deviceangularvelocity();
            devAngVel->set_x(retrievedDeviceAngVel.x());
            devAngVel->set_y(retrievedDeviceAngVel.y());
            devAngVel->set_z(retrievedDeviceAngVel.z());
        }

        // Device orientation
        if (dataCollectionConfig.device().orientation()) {
            retrievedDeviceRot = SimulationManager::simContext.tool->getDeviceGlobalRot();
            cQuaternion q; q.fromRotMat(retrievedDeviceRot);
            Quaternion* devQuat = dataPoint->mutable_deviceorientation();
            devQuat->set_x(q.x);
            devQuat->set_y(q.y);
            devQuat->set_z(q.z);
            devQuat->set_w(q.w);
        }

        // Make sure we have velocity for force calculations if needed
        if (!dataCollectionConfig.device().velocity() && 
            (dataCollectionConfig.objects().dampingforce() || dataCollectionConfig.objects().force())) {
            retrievedDeviceVel = SimulationManager::simContext.tool->getDeviceGlobalLinVel();
        }

        // Object states snapshot based on configuration
        for (const auto& kv : SimulationManager::simContext.objectMap) {
            const std::string& oid = kv.first;
            const std::unique_ptr<cGenericObject>& objPtr = kv.second;
            if (!objPtr) continue;
            
            ObjectState* os = dataPoint->add_objects();
            os->set_id(oid);
            
            // Position
            if (dataCollectionConfig.objects().position()) {
                cVector3d p = objPtr->getGlobalPos();
                Vector3* osp = os->mutable_position();
                osp->set_x(p.x()); osp->set_y(p.y()); osp->set_z(p.z());
            }
            
            // Orientation
            if (dataCollectionConfig.objects().orientation()) {
                cMatrix3d R = objPtr->getGlobalRot();
                cQuaternion oq; oq.fromRotMat(R);
                Quaternion* oso = os->mutable_orientation();
                oso->set_x(oq.x); oso->set_y(oq.y); oso->set_z(oq.z); oso->set_w(oq.w);
            }
            
            // Contact detection and related data
            bool inContact = false;
            if (dataCollectionConfig.objects().contact() || 
                dataCollectionConfig.objects().contactpoint() ||
                dataCollectionConfig.objects().contactnormal() ||
                dataCollectionConfig.objects().penetrationdepth() ||
                dataCollectionConfig.objects().force() ||
                dataCollectionConfig.objects().springforce() ||
                dataCollectionConfig.objects().dampingforce()) {
                
                inContact = hapticPoint->isInContact(objPtr.get());
                
                if (dataCollectionConfig.objects().contact()) {
                    os->set_incontact(inContact);
                }
                
                if (inContact && objPtr->m_interactionInside) {
                    // Get proxy position for calculations
                    cVector3d proxyPos;
                    if (!dataCollectionConfig.device().proxyposition()) {
                        proxyPos = hapticPoint->getGlobalPosProxy();
                    } else {
                        proxyPos = retrievedProxyPos;
                    }
                    
                    // Contact point (in world coordinates)
                    cVector3d contactPoint = objPtr->getGlobalPos() + cMul(objPtr->getGlobalRot(), objPtr->m_interactionPoint);
                    if (dataCollectionConfig.objects().contactpoint()) {
                        Vector3* cp = os->mutable_contactpoint();
                        cp->set_x(contactPoint.x());
                        cp->set_y(contactPoint.y());
                        cp->set_z(contactPoint.z());
                    }
                    
                    // Contact normal (in world coordinates)
                    cVector3d contactNormal = cMul(objPtr->getGlobalRot(), objPtr->m_interactionNormal);
                    if (dataCollectionConfig.objects().contactnormal()) {
                        Vector3* cn = os->mutable_contactnormal();
                        cn->set_x(contactNormal.x());
                        cn->set_y(contactNormal.y());
                        cn->set_z(contactNormal.z());
                    }
                    
                    // Compute penetration depth
                    double penetrationDepth = cDistance(proxyPos, contactPoint);
                    if (dataCollectionConfig.objects().penetrationdepth()) {
                        os->set_penetrationdepth(penetrationDepth);
                    }
                    
                    // Force calculations (if requested)
                    if ((dataCollectionConfig.objects().force() || 
                         dataCollectionConfig.objects().springforce() ||
                         dataCollectionConfig.objects().dampingforce()) && objPtr->m_material) {
                        
                        double stiffness = objPtr->m_material->getStiffness();
                        double damping = objPtr->m_material->getDamping();
                        
                        // Spring force: F = k * penetration * normal
                        cVector3d springForce = -stiffness * penetrationDepth * contactNormal;
                        if (dataCollectionConfig.objects().springforce()) {
                            Vector3* sf = os->mutable_springforce();
                            sf->set_x(springForce.x());
                            sf->set_y(springForce.y());
                            sf->set_z(springForce.z());
                        }
                        
                        // Damping force: F = -b * velocity
                        cVector3d dampingForce = -damping * retrievedDeviceVel;
                        if (dataCollectionConfig.objects().dampingforce()) {
                            Vector3* df = os->mutable_dampingforce();
                            df->set_x(dampingForce.x());
                            df->set_y(dampingForce.y());
                            df->set_z(dampingForce.z());
                        }
                        
                        // Total force from this object
                        if (dataCollectionConfig.objects().force()) {
                            cVector3d totalObjForce = springForce + dampingForce;
                            Vector3* tf = os->mutable_force();
                            tf->set_x(totalObjForce.x());
                            tf->set_y(totalObjForce.y());
                            tf->set_z(totalObjForce.z());
                        }
                    }
                }
            }
        }

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
    SimulationManager::trialContext.trialRunning = false;
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