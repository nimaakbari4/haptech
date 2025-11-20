#include "simulation_manager.h"
#include "tool_mesh.h"
#include "objects/custom_object_interface.h"
#include "extensions_registry.h"
#include "pong_game_state.h"
#include <iostream>
#include <map>
#include <algorithm>

SimulationContext SimulationManager::simContext;
TrialContext SimulationManager::trialContext;

bool SimulationManager::run() {
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    std::cout << "[SimulationManager] simContext address: " << &simContext << std::endl;
    std::cout << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "CHAI3D" << std::endl;
    std::cout << "-----------------------------------" << std::endl << std::endl << std::endl;
    std::cout << "Keyboard Options:" << std::endl << std::endl;
    std::cout << "[f] - Enable/Disable full screen mode" << std::endl;
    std::cout << "[m] - Enable/Disable vertical mirroring" << std::endl;
    std::cout << "[q] - Exit application" << std::endl;
    std::cout << std::endl << std::endl;



    //--------------------------------------------------------------------------
    // OPENGL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        std::cout << "failed initialization" << std::endl;
        cSleepMs(1000);
        return 1;
    }

    // set error callback
    glfwSetErrorCallback(errorCallback);

    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int w = 0.8 * mode->height;
    int h = 0.5 * mode->height;
    int x = 0.5 * (mode->width - w);
    int y = 0.5 * (mode->height - h);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // ensure high-DPI framebuffer on macOS Retina displays (if supported)
    #ifdef __APPLE__
    #ifdef GLFW_COCOA_RETINA_FRAMEBUFFER
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    #endif
    #endif

    // set active stereo mode
    if (simContext.stereoMode == C_STEREO_ACTIVE)
    {
        glfwWindowHint(GLFW_STEREO, GL_TRUE);
    }
    else
    {
        glfwWindowHint(GLFW_STEREO, GL_FALSE);
    }

    // create display context
    simContext.window = glfwCreateWindow(w, h, "CHAI3D", NULL, NULL);
    if (!simContext.window)
    {
        std::cout << "failed to create window" << std::endl;
        cSleepMs(1000);
        glfwTerminate();
        return false;
    }

    // get width and height of window
    glfwGetWindowSize(simContext.window, &simContext.width, &simContext.height);

    // set position of window
    glfwSetWindowPos(simContext.window, x, y);

    // set key callback
    glfwSetKeyCallback(simContext.window, keyCallback);

    // set resize callback
    glfwSetWindowSizeCallback(simContext.window, windowSizeCallback);

    // set current display context
    glfwMakeContextCurrent(simContext.window);

    // sets the swap interval for the current display context
    glfwSwapInterval(simContext.swapInterval);

    #ifdef GLEW_VERSION
    // initialize GLEW library
    if (glewInit() != GLEW_OK)
    {
        std::cout << "failed to initialize GLEW library" << std::endl;
        glfwTerminate();
        return 1;
    }
    #endif

    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    simContext.world = new cWorld();

    // set the background color of the environment
    simContext.world->m_backgroundColor.setBlack();

    // create a camera and insert it into the virtual world
    simContext.camera = new cCamera(simContext.world);
    simContext.world->addChild(simContext.camera);

    // position and orient the camera
    simContext.camera->set( cVector3d (1.0, 0.0, 0.0),    // camera position (eye)
                 cVector3d (0.0, 0.0, 0.0),    // look at position (target)
                 cVector3d (0.0, 0.0, 1.0));   // direction of the (up) vector

    // set the near and far clipping planes of the camera
    simContext.camera->setClippingPlanes(0.01, 10.0);

    // set stereo mode
    simContext.camera->setStereoMode(simContext.stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    simContext.camera->setStereoEyeSeparation(0.01);
    simContext.camera->setStereoFocalLength(0.5);

    // set vertical mirrored display mode
    simContext.camera->setMirrorVertical(simContext.mirroredDisplay);

    // create a directional light source
    simContext.light = new cDirectionalLight(simContext.world);

    // insert light source inside world
    simContext.world->addChild(simContext.light);

    // enable light source
    simContext.light->setEnabled(true);

    // define direction of light beam
    simContext.light->setDir(-1.0, 0.0, 0.0); 


    //--------------------------------------------------------------------------
    // HAPTIC DEVICE
    //--------------------------------------------------------------------------

    // create a haptic device handler
    simContext.handler = new cHapticDeviceHandler();

    // get access to the first available haptic device found
    simContext.handler->getDevice(simContext.hapticDevice, 0);

    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = simContext.hapticDevice->getSpecifications();

    // create and configure the haptic tool
    createHapticTool();

    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    cFontPtr font = NEW_CFONTCALIBRI20();
    cFontPtr biggerFont = NEW_CFONTCALIBRI36();
    
    // create a label to display the haptic and graphic rates of the simulation
    simContext.labelRates = new cLabel(font);
    simContext.labelRates->m_fontColor.setWhite();
    simContext.camera->m_frontLayer->addChild(simContext.labelRates);

    // create a label to display the current trial status (i.e. running or not)
    simContext.statusText = new cLabel(biggerFont);
    simContext.statusText->m_fontColor.setWhite();
    simContext.camera->m_frontLayer->addChild(simContext.statusText);

    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    simContext.hapticsThread = new cThread();
    simContext.hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // start debug loop
    // simContext.dataThread = new cThread();
    // simContext.dataThread->start(dataPollingThread, CTHREAD_PRIORITY_GRAPHICS);

    simContext.simulationRunning = true;
    // setup callback when application exits
    atexit(close);

    //--------------------------------------------------------------------------
    // MAIN GRAPHIC LOOP
    //--------------------------------------------------------------------------

    // call window size callback at initialization
    windowSizeCallback(simContext.window, simContext.width, simContext.height);

    // main graphic loop
    while (!glfwWindowShouldClose(simContext.window))
    {
        // get width and height of window
        glfwGetWindowSize(simContext.window, &simContext.width, &simContext.height);

        // render graphics
        updateGraphics();

        // swap buffers
        glfwSwapBuffers(simContext.window);

        // process events
        glfwPollEvents();

        // signal frequency counter
        simContext.freqCounterGraphics.signal(1);
    }

    // close window
    glfwDestroyWindow(simContext.window);

    // terminate GLFW library
    glfwTerminate();

    return true;
}

void SimulationManager::setDebugMode(bool debug) {
    simContext.debug = debug;
}

void SimulationManager::checkTrialConditions() {
    checkStartConditions();
    checkEndConditions();
}

void SimulationManager::checkStartConditions() {
    if (trialContext.trialRunning && trialContext.startConditionMet == nullptr) {
        for (size_t i = 0; i < trialContext.startConditions.size(); ++i) {
            auto& condition = trialContext.startConditions[i];
            if (condition && condition->evaluate()) {
                trialContext.startConditionMet = std::move(condition);
                break;
            }
        }
    }
}

void SimulationManager::checkEndConditions() {
    if (trialContext.trialRunning && trialContext.endConditionMet == nullptr) {
        for (size_t i = 0; i < trialContext.endConditions.size(); ++i) {
            auto& condition = trialContext.endConditions[i];
            if (condition && condition->evaluate()) {
                trialContext.endConditionMet = std::move(condition);
                break;
            }
        }
    }
}

// SIMULATION MANIPULATION //

bool SimulationManager::reset() {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    // disable haptic calculations
    simContext.hapticsEnabled = false;

    // Clear tool state to drop any contacts before removing objects
    if (simContext.tool) {
        simContext.tool->initialize();
    }

    cSleepMs(100);

    setBackgroundColor(0.0, 0.0, 0.0);
    clearTextOverlays();
    removeAllObjects();
    ResetPongGameState();
    
    {
        std::lock_guard<std::mutex> lock(simContext.objectMapMutex);
        // Clear all containers
        trialContext.startConditions.clear();
        trialContext.endConditions.clear();
        trialContext.startConditionMet = nullptr;
        trialContext.endConditionMet = nullptr;
    }
    
    // ensure device sees zero force while haptics are disabled
    if (simContext.hapticDevice) {
        simContext.hapticDevice->setForce(cVector3d(0, 0, 0));
    }

    // allow one or two frames for deferred deletion to flush
    cSleepMs(100);

    // re-enable haptics for next trial
    simContext.hapticsEnabled = true;

    return true;
}

bool SimulationManager::setBackgroundColor(double r, double g, double b) {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    simContext.world->m_backgroundColor.setR(r);
    simContext.world->m_backgroundColor.setG(g);
    simContext.world->m_backgroundColor.setB(b);
    return true;
}

bool SimulationManager::setCamera(double px, double py, double pz,
                                  double lx, double ly, double lz,
                                  double ux, double uy, double uz) {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    if (!simContext.camera) {
        std::cout << "[ERROR] Camera has not been initialized." << std::endl;
        return false;
    }

    cVector3d position(px, py, pz);
    cVector3d lookAt(lx, ly, lz);
    cVector3d up(ux, uy, uz);

    if (up.lengthsq() < 1e-12) {
        std::cout << "[WARN] Camera up vector too small; defaulting to (0, 0, 1)." << std::endl;
        up.set(0.0, 0.0, 1.0);
    }

    simContext.camera->set(position, lookAt, up);

    if (simContext.light) {
        cVector3d dir = lookAt - position;
        if (dir.lengthsq() > 1e-12) {
            dir.normalize();
            simContext.light->setDir(-dir.x(), -dir.y(), -dir.z());
        }
    }

    return true;
}

bool SimulationManager::addTextOverlay(const std::string& text, double x, double y, double fontSize, double r, double g, double b) {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    // Create font with specified size (CHAI3D has preset sizes: 20, 32, 40)
    cFontPtr font = NEW_CFONTCALIBRI20();
    if (fontSize > 0) {
        if (fontSize >= 36) {
            font = NEW_CFONTCALIBRI40();
        } else if (fontSize >= 26) {
            font = NEW_CFONTCALIBRI32();
        } else {
            font = NEW_CFONTCALIBRI20();
        }
    }

    // Create label
    cLabel* label = new cLabel(font);
    label->setText(text);
    label->m_fontColor.setR(r);
    label->m_fontColor.setG(g);
    label->m_fontColor.setB(b);
    label->setLocalPos((int)x, (int)y);
    
    // Add to camera front layer
    simContext.camera->m_frontLayer->addChild(label);
    
    // Store in custom overlays vector
    simContext.customTextOverlays.push_back(label);
    
    std::cout << "[SimulationManager] Added text overlay: \"" << text << "\" at (" << x << ", " << y << ")" << std::endl;
    return true;
}

bool SimulationManager::clearTextOverlays() {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    // Remove all custom text overlays
    for (auto* label : simContext.customTextOverlays) {
        simContext.camera->m_frontLayer->removeChild(label);
        delete label;
    }
    simContext.customTextOverlays.clear();
    
    std::cout << "[SimulationManager] Cleared all text overlays" << std::endl;
    return true;
}

bool SimulationManager::addObject(std::string id, cGenericObject* obj, 
        double px, double py, double pz,
        double rx, double ry, double rz) {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    if (!obj) {
        std::cout << "[ERROR] Object is null." << std::endl;
        return false;
    }

    if (simContext.objectMap.find(id) != simContext.objectMap.end()) {
        std::cout << "[ERROR] Object with ID " << id << " already exists." << std::endl;
        return false;
    }

    // Add the object into the world immediately
    simContext.world->addChild(obj);
    obj->setLocalPos(px, py, pz);
    
    // Set rotation using Euler angles (in radians)
    cMatrix3d rotMatrix;
    rotMatrix.setExtrinsicEulerRotationRad(rx, ry, rz, C_EULER_ORDER_XYZ);
    obj->setLocalRot(rotMatrix);

    // Get workspace scale
    double workspaceScaleFactor = simContext.tool->getWorkspaceScaleFactor();
    cHapticDeviceInfo hapticDeviceInfo = simContext.hapticDevice->getSpecifications();
    double maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;

    // Preserve existing material or create new one
    cMaterialPtr material = obj->m_material;
    if (!material) {
        material = cMaterial::create();
        obj->setMaterial(material);
    }
    material->setStiffness(maxStiffness);

    // Add the object to the map using unique_ptr
    {
        std::lock_guard<std::mutex> lock(simContext.objectMapMutex);
        simContext.objectMap[id] = std::unique_ptr<cGenericObject>(obj);
    }
    std::cout << "[DEBUG] Added object with ID " << id << " at position (" << px << ", " << py << ", " << pz << ") with rotation (" << rx << ", " << ry << ", " << rz << ")." << std::endl;

    return true;
}

bool SimulationManager::removeObject(std::string id) {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    bool success;

	// Defer deletion: mark for removal and let graphics thread remove safely
    {
        std::lock_guard<std::mutex> lock(simContext.objectMapMutex);
        auto it = simContext.objectMap.find(id);
        if (it != simContext.objectMap.end() && it->second) {
            cGenericObject* obj = it->second.get();
            std::cout << "[DEBUG] Setting object " << id << " to position (10000, 10000, 10000)." << std::endl;
            obj->setLocalPos(10000, 10000, 10000);
            success = true;
        } else {
            std::cout << "[ERROR] Object with ID " << id << " not found." << std::endl;
            success = false;
        }
    }

    return success;
}

bool SimulationManager::removeAllObjects() {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(simContext.objectMapMutex);
        for (const auto& pair : simContext.objectMap) {
            // Note: We need to call removeObject without the lock, so we'll just move objects far away
            if (pair.second) {
                pair.second->setLocalPos(10000, 10000, 10000);
            }
        }
    }

    // All objects are now removed and deleted by unique_ptr
    std::cout << "[DEBUG] Removed all objects." << std::endl;
    return true;
}

bool SimulationManager::addSphere(std::string id, double radius, double x, double y, double z) {
    std::unique_ptr<cMesh> sphere = std::make_unique<cMesh>();
    cCreateSphere(sphere.get(), radius);
    sphere->createAABBCollisionDetector(radius);

    return addObject(id, sphere.release(), x, y, z, 0, 0, 0);
}

bool SimulationManager::addObjectFromConfig(const std::string& id, const std::string& type, 
                                           double px, double py, double pz,
                                           double rx, double ry, double rz,
                                           double r, double g, double b,
                                           const std::map<std::string, std::string>& params) {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    // Try to create using custom object system (now includes built-in types)
    if (CustomObjectRegistry::isTypeRegistered(type)) {
        std::cout << "[DEBUG] Creating object using custom object system: " << type << std::endl;
        
        auto customObj = CustomObjectRegistry::createObject(type);
        if (customObj) {
            cGenericObject* chaiObj = customObj->createObject(params, px, py, pz, r, g, b);
            
            if (chaiObj) {
                // Set position
                chaiObj->setLocalPos(px, py, pz);

                // Set rotation using Euler angles (in radians)
                cMatrix3d rotMatrix;
                rotMatrix.setExtrinsicEulerRotationRad(rx, ry, rz, C_EULER_ORDER_XYZ);
                chaiObj->setLocalRot(rotMatrix);

                // Add to world and object map
                simContext.world->addChild(chaiObj);
                
                // Get workspace scale
                double workspaceScaleFactor = simContext.tool->getWorkspaceScaleFactor();
                cHapticDeviceInfo hapticDeviceInfo = simContext.hapticDevice->getSpecifications();
                double maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;

                // Preserve existing material or create new one
                cMaterialPtr material = chaiObj->m_material;
                if (!material) {
                    material = cMaterial::create();
                    chaiObj->setMaterial(material);
                }
                material->setStiffness(maxStiffness);
                
                {
                    std::lock_guard<std::mutex> lock(simContext.objectMapMutex);
                    simContext.objectMap[id] = std::unique_ptr<cGenericObject>(chaiObj);
                    simContext.objectInterfaceMap[id] = std::move(customObj);
                }
                std::cout << "[DEBUG] Added custom object with ID " << id << " at position (" << px << ", " << py << ", " << pz << ") with rotation (" << rx << ", " << ry << ", " << rz << ")." << std::endl;
                return true;
            }
        }
        std::cout << "[ERROR] Failed to create object: " << type << std::endl;
        return false;
    }

    // If we get here, the object type is not registered
    std::cout << "[ERROR] Unknown object type: " << type << std::endl;
    return false;
}

// PRIVATE CALLBACKS //

// callback when the window display is resized
void SimulationManager::windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height) {
    // update window size
    simContext.width  = a_width;
    simContext.height = a_height;
}

// callback when an error GLFW occurs
void SimulationManager::errorCallback(int error, const char* a_description) {
    std::cout << "Error: " << a_description << std::endl;
}

// callback when a key is pressed
void SimulationManager::keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods) {
    // filter calls that only include a key press
    if (a_action != GLFW_PRESS)
    {
        return;
    }

    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    // option - toggle fullscreen
    else if (a_key == GLFW_KEY_F)
    {
        // toggle state variable
        simContext.fullscreen = !simContext.fullscreen;

        // get handle to monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        // get information about monitor
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // set fullscreen or window mode
        if (simContext.fullscreen)
        {
            glfwSetWindowMonitor(simContext.window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSwapInterval(simContext.swapInterval);
        }
        else
        {
            int w = 0.8 * mode->height;
            int h = 0.5 * mode->height;
            int x = 0.5 * (mode->width - w);
            int y = 0.5 * (mode->height - h);
            glfwSetWindowMonitor(simContext.window, NULL, x, y, w, h, mode->refreshRate);
            glfwSwapInterval(simContext.swapInterval);
        }
    }

    // option - toggle vertical mirroring
    else if (a_key == GLFW_KEY_M)
    {
        simContext.mirroredDisplay = !simContext.mirroredDisplay;
        simContext.camera->setMirrorVertical(simContext.mirroredDisplay);
    }

}

// this function renders the scene
void SimulationManager::updateGraphics(void) {
    // No queued additions path
    // Process any pending deletions when haptics are disabled (safe point)

    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // update haptic and graphic rate data
    simContext.labelRates->setText(cStr(simContext.freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        cStr(simContext.freqCounterHaptics.getFrequency(), 0) + " Hz");

    // query framebuffer size (in pixels) for correct Retina scaling
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(simContext.window, &fbw, &fbh);

    // update position of label using framebuffer width
    simContext.labelRates->setLocalPos((int)(0.5 * (fbw - simContext.labelRates->getWidth())), 15);

    // update status text
    if (trialContext.trialRunning) {
        simContext.statusText->setText("Trial running: " + trialContext.trialId + " | t = " + cStr(trialContext.time, 4));
    } else {
        simContext.statusText->setText("Waiting for a trial to begin");
    }

    // place status text using framebuffer height
    simContext.statusText->setLocalPos(5, fbh - simContext.statusText->getHeight() - 5);

    /////////////////////////////////////////////////////////////////destroyHaptic////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    if (!trialContext.trialRunning) {
        // during teardown/idle, skip shadow map updates to avoid touching deleted geometry
    } else {
        simContext.world->updateShadowMaps(false, simContext.mirroredDisplay);
    }

    // render world using framebuffer dimensions to fill the window
    simContext.camera->renderView(fbw, fbh);

    // wait until all GL commands are completed
    glFinish();

    // No deferred garbage mechanism

    // check for any OpenGL errors
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) {
        const GLubyte* msg = gluErrorString(err);
        std::cout << "OpenGL error: " << (msg ? reinterpret_cast<const char*>(msg) : "unknown") << std::endl;
    }
}

// this function contains the main haptics simulation loop
void SimulationManager::updateHaptics(void) {
    simContext.simulationRunning = true;
    simContext.simulationFinished = false;
    cPrecisionClock clock;
    double deltaTime;
    clock.start();

    std::cout << "Haptics thread started, simRunning = " << simContext.simulationRunning << std::endl;

    while (simContext.simulationRunning) {
        // update delta time
        clock.stop();

        deltaTime = clock.getCurrentTimeSeconds();

        clock.reset();
        clock.start();

        // HAPTICS
        if (!simContext.hapticsEnabled) {
            // wait until haptics is enabled
            cSleepMs(100);
            continue;
        }

        // update world transforms
        simContext.world->computeGlobalPositions(true);

        // update tool from device only (mouse controls removed)
        simContext.tool->updateFromDevice();

        // compute interaction forces
        simContext.tool->computeInteractionForces();

        // apply computed forces
        simContext.tool->applyToDevice();

        // update haptic loop frequency
        simContext.freqCounterHaptics.signal(1);

        // PHYSICS
        // Update physics for all objects that have custom interfaces
        if (trialContext.trialRunning) {
            std::lock_guard<std::mutex> lock(simContext.objectMapMutex);
            for (const auto& objPair : simContext.objectMap) {
                const std::string& objId = objPair.first;
                cGenericObject* chaiObj = objPair.second.get();
                
                // Look for corresponding custom object interface
                auto interfaceIt = simContext.objectInterfaceMap.find(objId);
                if (interfaceIt != simContext.objectInterfaceMap.end()) {
                    CustomObjectInterface* customInterface = interfaceIt->second.get();
                    if (customInterface && chaiObj) {
                        // Call physics update with deltaTime
                        customInterface->updatePhysics(chaiObj, deltaTime);
                    }
                }
            }
        }

        cSleepMs(1000 / HAPTICS_LOOP_FREQUENCY);
    }

    simContext.simulationFinished = true;

    std::cout << "Haptics thread terminated, simRunning = " << simContext.simulationRunning << std::endl;
}

void SimulationManager::dataPollingThread() {
    simContext.dataThreadRunning = true;

    while (simContext.simulationRunning) {
        if (trialContext.trialRunning) {
            cHapticPoint* hapticPoint = simContext.tool->getHapticPoint(0);

            if (hapticPoint) {
                cVector3d proxyPos = hapticPoint->getGlobalPosProxy();
                cVector3d computedForce = hapticPoint->getLastComputedForce();

                cVector3d devicePos;
                simContext.hapticDevice->getPosition(devicePos);

                cVector3d deviceForce;
                simContext.hapticDevice->getForce(deviceForce);
                
                // Debug output removed
            }

            cSleepMs(1000 / DATA_POLL_FREQUENCY);
        }
    }

    simContext.dataThreadRunning = false;
}

// this function closes the application
void SimulationManager::close(void) {
    // disable haptic calculations
    simContext.hapticsEnabled = false;
    
    // softly ramp to zero to avoid torque discontinuity reflex
    rampDeviceForceToZero(100);

    // remove all objects from the world
    removeAllObjects();

    // stop the simulation
    simContext.simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simContext.simulationFinished) { cSleepMs(100); }

    // destroy haptic tool cleanly
    destroyHapticTool();
    
    delete simContext.hapticsThread;
    simContext.hapticsThread = nullptr;

    if (simContext.dataThreadRunning) {
        cSleepMs(100);
    }
    delete simContext.dataThread;
    simContext.dataThread = nullptr;
    
    delete simContext.handler;
    simContext.handler = nullptr;
    
    delete simContext.world;
    simContext.world = nullptr;    
}

void SimulationManager::rampDeviceForceToZero(int durationMs) {
    if (!simContext.hapticDevice) return;

    cVector3d currentForce;
    simContext.hapticDevice->getForce(currentForce);

    const int steps = std::max(1, durationMs / 5);
    for (int i = 0; i < steps; ++i) {
        double alpha = 1.0 - static_cast<double>(i + 1) / steps;
        cVector3d f = alpha * currentForce;
        simContext.hapticDevice->setForce(f);
        cSleepMs(5);
    }
    simContext.hapticDevice->setForce(cVector3d(0, 0, 0));
}

void SimulationManager::applyConstantImpulse(double fx, double fy, double fz, int durationMs) {
    if (!simContext.hapticDevice) {
        std::cout << "[WARN] Cannot apply impulse: haptic device is null." << std::endl;
        return;
    }

    cVector3d impulseForce(fx, fy, fz);
    cVector3d originalForce;
    simContext.hapticDevice->getForce(originalForce);

    // Apply the impulse force for the specified duration
    const int stepMs = 1;  // 1kHz update to match haptics loop
    const int steps = std::max(1, durationMs / stepMs);
    
    for (int i = 0; i < steps; ++i) {
        simContext.hapticDevice->setForce(impulseForce);
        cSleepMs(stepMs);
    }

    // Return to original force (or let haptics loop take over)
    simContext.hapticDevice->setForce(originalForce);
}

void SimulationManager::createHapticTool() {
    if (!simContext.handler || !simContext.world) {
        std::cout << "[ERROR] Cannot create haptic tool: handler or world is null." << std::endl;
        return;
    }

    // create a tool (mesh-based cursor) and insert into the world
    

    auto teapotMulti = new cMultiMesh();
    {
        cMesh* sub = teapotMulti->newMesh();
        cCreateTeaPot(sub, 0.5, 10, cVector3d(0,0,0), cMatrix3d(0, 0, 0, C_EULER_ORDER_XYZ), cColorf(1, 0,0));
        teapotMulti->m_material->setWhite();
    }

    auto boxMulti = new cMultiMesh();
    {
        cMesh* sub = boxMulti->newMesh();
        cCreateBox(sub, 0.02, 0.02, 0.02);
        boxMulti->m_material->setWhite();
    }

    // Instantiate cToolMesh with sampling and contact radius
    const int sampleCount = 24;
    const double contactRadius = 0.01;
    simContext.tool = new cToolMesh(simContext.world, boxMulti, sampleCount, contactRadius);

    simContext.world->addChild(simContext.tool);

    // connect the haptic device to the virtual tool
    simContext.tool->setHapticDevice(simContext.hapticDevice);

    simContext.tool->enableDynamicObjects(true);

    // map the physical workspace of the haptic device to a larger virtual workspace.
    simContext.tool->setWorkspaceRadius(1.0);

    // haptic forces are enabled only if small forces are first sent to the device;
    // this mode avoids the force spike that occurs when the application starts when 
    // the tool is located inside an object for instance. 
    simContext.tool->setWaitForSmallForce(true);

    // start the haptic tool
    simContext.tool->start();
    
    std::cout << "[DEBUG] Created and started new haptic tool." << std::endl;
}

void SimulationManager::destroyHapticTool() {
    if (!simContext.tool) {
        return;
    }

    // Stop the tool first
    simContext.tool->stop();
    
    // Remove from world
    if (simContext.world) {
        simContext.world->removeChild(simContext.tool);
    }
    
    // Delete the tool
    delete simContext.tool;
    simContext.tool = nullptr;
    
    // Force zero on device
    if (simContext.hapticDevice) {
        simContext.hapticDevice->setForce(cVector3d(0, 0, 0));
    }
    
    std::cout << "[DEBUG] Destroyed haptic tool." << std::endl;
}
