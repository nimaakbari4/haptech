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
// Toggle default cursor implementation: true = cToolMesh, false = cToolCursor.
bool SimulationManager::useMeshCursorByDefault = true;

bool SimulationManager::run() {
    //--------------------------------------------------------------------------
    // INITIALIZATIONa
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

    // initialize physics clock for graphics thread physics updates
    simContext.physicsClock.start();

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

    std::cout << "[RESET] Starting environment reset..." << std::endl;
    
    // Ensure an OpenGL context is current before deleting GL resources.
    // reset() is invoked from the gRPC thread, not the graphics thread.
    if (simContext.window) {
        glfwMakeContextCurrent(simContext.window);
    }
    
    // Check force before disabling haptics
    cVector3d forceBeforeReset(0, 0, 0);
    if (simContext.hapticDevice) {
        simContext.hapticDevice->getForce(forceBeforeReset);
        std::cout << "[RESET] Force before reset: " << forceBeforeReset.str(3) 
                  << " (mag: " << forceBeforeReset.length() << "N)" << std::endl;
    }

    // disable haptic calculations
    std::cout << "[RESET] Disabling haptics..." << std::endl;
    simContext.hapticsEnabled = false;

    // Clear tool state to drop any contacts before removing objects
    std::cout << "[RESET] Clearing tool state..." << std::endl;
    if (simContext.tool) {
        simContext.tool->initialize();
    }

    cSleepMs(100);

    std::cout << "[RESET] Removing objects and clearing scene..." << std::endl;
    setBackgroundColor(0.0, 0.0, 0.0);
    clearTextOverlays();
    removeAllObjects();
    ResetPongGameState();

    // reset camera to default position (this also updates light direction)
    setCamera(1.0, 0.0, 0.0,  // position
              0.0, 0.0, 0.0,  // lookAt
              0.0, 0.0, 1.0); // up
    
    // Re-enable cursor tool visual (in case it was hidden for pong game)
    if (simContext.tool) {
        // Re-enable the tool's image if it exists
        if (simContext.tool->m_image) {
            simContext.tool->m_image->setEnabled(true);
        }
        simContext.tool->setEnabled(true);
    }
    
    {
        std::lock_guard<std::mutex> lock(simContext.objectMapMutex);
        // Clear all containers
        trialContext.startConditions.clear();
        trialContext.endConditions.clear();
        trialContext.startConditionMet = nullptr;
        trialContext.endConditionMet = nullptr;
    }
    
    // Ramp force down gradually instead of abrupt zero
    std::cout << "[RESET] Ramping force to zero gradually..." << std::endl;
    rampDeviceForceToZero(150); // 150ms ramp down
    
    cVector3d forceAfterRamp(0, 0, 0);
    if (simContext.hapticDevice) {
        simContext.hapticDevice->getForce(forceAfterRamp);
        std::cout << "[RESET] Force after ramp: " << forceAfterRamp.str(3) 
                  << " (mag: " << forceAfterRamp.length() << "N)" << std::endl;
    }

    // allow one or two frames for deferred deletion to flush
    cSleepMs(100);

    // Re-enable haptics so the user can move the cursor in the wait room.
    std::cout << "[RESET] Re-enabling haptics for wait room cursor control" << std::endl;
    simContext.hapticsEnabled = true;

    std::cout << "[RESET] Reset complete" << std::endl;
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
        // Ensure light is enabled
        simContext.light->setEnabled(true);
        
        // Calculate light direction from camera position to look-at point
        cVector3d dir = lookAt - position;
        if (dir.lengthsq() > 1e-12) {
            dir.normalize();
            // Light should come from camera direction (opposite of view direction)
            cVector3d lightDir(dir.x(), dir.y(), dir.z());
            simContext.light->setDir(lightDir.x(), lightDir.y(), lightDir.z());
        } else {
            // Fallback: use default light direction
            simContext.light->setDir(-1.0, 0.0, 0.0);
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
                double maxDamping = hapticDeviceInfo.m_maxLinearDamping / workspaceScaleFactor;

                // Preserve existing material or create new one
                cMaterialPtr material = chaiObj->m_material;
                if (!material) {
                    material = cMaterial::create();
                    chaiObj->setMaterial(material);
                }

                // Use conservative stiffness/damping to avoid safety threshold triggers
                // Start with 10-20% of max stiffness for safer interaction
                material->setStiffness(1.0 * maxStiffness);
                material->setDamping(0.3 * maxDamping);

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
    // UPDATE PHYSICS (moved from haptic loop for performance)
    /////////////////////////////////////////////////////////////////////
    
    // Update physics for all objects that have custom interfaces
    // This runs at graphics refresh rate (~60 Hz) instead of haptic rate (1000 Hz)
    // to avoid slowing down the critical haptic loop
    if (trialContext.trialRunning) {
        // Calculate deltaTime for physics updates
        simContext.physicsClock.stop();
        double physicsDeltaTime = simContext.physicsClock.getCurrentTimeSeconds();
        simContext.physicsClock.reset();
        simContext.physicsClock.start();
        
        // Clamp deltaTime to prevent large jumps (e.g., when window is minimized)
        // Also ensure minimum deltaTime to avoid division by zero or very small values
        if (physicsDeltaTime > 0.1) {
            physicsDeltaTime = 0.016; // Cap at ~60 Hz equivalent
        } else if (physicsDeltaTime < 0.0001) {
            physicsDeltaTime = 0.016; // Minimum deltaTime for first frame or very fast updates
        }
        
        // Update physics with proper mutex locking
        {
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
                        customInterface->updatePhysics(chaiObj, physicsDeltaTime);
                    }
                }
            }
        }
    }

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
    
    bool justEnabledHaptics = false;
    int cyclesSinceEnabled = 0;

    while (simContext.simulationRunning) {
        // update delta time
        clock.stop();

        deltaTime = clock.getCurrentTimeSeconds();

        clock.reset();
        clock.start();

        // HAPTICS
        if (!simContext.hapticsEnabled) {
            // wait until haptics is enabled
            justEnabledHaptics = true;
            cyclesSinceEnabled = 0;
            cSleepMs(100);
            continue;
        }

        // Log the first few cycles after haptics is enabled
        // if (justEnabledHaptics && cyclesSinceEnabled < 3) {
        //     std::cout << "[HAPTICS] Cycle " << cyclesSinceEnabled << " after enabling" << std::endl;
        //     cyclesSinceEnabled++;
        //     if (cyclesSinceEnabled >= 3) {
        //         justEnabledHaptics = false;
        //     }
        // }

        // update world transforms
        simContext.world->computeGlobalPositions(true);

        // update tool from device only (mouse controls removed)
        simContext.tool->updateFromDevice();

        // Heuristic to unstick the proxy if it gets wedged deep inside objects.
        // If the separation between proxy and goal stays large, reset the tool.
        if (simContext.tool) {
            static int stuckCounter = 0;
            const double STUCK_SEPARATION = 0.03; // 3 cm
            const int STUCK_CYCLES = 20;          // ~20 ms at 1 kHz

            cHapticPoint* hp = simContext.tool->getHapticPoint(0);
            if (hp) {
                cVector3d proxyPos = hp->getGlobalPosProxy();
                cVector3d goalPos  = hp->getGlobalPosGoal();
                double separation = cDistance(proxyPos, goalPos);

                if (separation > STUCK_SEPARATION) {
                    stuckCounter++;
                    if (stuckCounter >= STUCK_CYCLES) {
                        std::cout << "[HAPTICS] Proxy stuck (separation=" << separation
                                  << "m). Re-initializing tool to unstick." << std::endl;
                        simContext.tool->initialize();
                        stuckCounter = 0;
                    }
                } else {
                    stuckCounter = 0;
                }
            } else {
                stuckCounter = 0;
            }
        }

        // Log contacts BEFORE computing forces
        // if (cyclesSinceEnabled > 0 && cyclesSinceEnabled <= 3) {
        //     int numPoints = simContext.tool->getNumHapticPoints();
        //     std::cout << "[HAPTICS] Cycle " << (cyclesSinceEnabled-1) << " - checking " << numPoints << " haptic points:" << std::endl;
        //     for (int i = 0; i < numPoints; i++) {
        //         auto hp = simContext.tool->getHapticPoint(i);
        //         if (hp) {
        //             int numContacts = hp->getNumCollisionEvents();
        //             cVector3d proxyPos = hp->getGlobalPosProxy();
        //             cVector3d devicePos = hp->getGlobalPosGoal();
        //             double separation = cDistance(proxyPos, devicePos);
        //             std::cout << "  Point " << i << ": " << numContacts << " contacts, separation=" 
        //                       << separation << "m, proxy=" << proxyPos.str(3) 
        //                       << ", device=" << devicePos.str(3) << std::endl;
                    
        //             for (int c = 0; c < numContacts; c++) {
        //                 auto collision = hp->getCollisionEvent(c);
        //                 if (collision && collision->m_object) {
        //                     std::cout << "    Contact " << c << " with object at " 
        //                               << collision->m_object->getGlobalPos().str(3) << std::endl;
        //                 }
        //             }
        //         }
        //     }
        // }
        
        // compute interaction forces
        simContext.tool->computeInteractionForces();

        // Log force after first few computations
        if (cyclesSinceEnabled > 0 && cyclesSinceEnabled <= 3) {
            cVector3d computedForce;
            if (simContext.hapticDevice) {
                simContext.hapticDevice->getForce(computedForce);
                std::cout << "[HAPTICS] Cycle " << (cyclesSinceEnabled-1) << " computed force: " 
                          << computedForce.str(3) << " (mag: " << computedForce.length() << "N)" << std::endl;
            }
        }

        // apply computed forces
        simContext.tool->applyToDevice();

        // update haptic loop frequency
        simContext.freqCounterHaptics.signal(1);

        // NOTE: Physics updates have been moved to the graphics thread
        // to avoid slowing down the haptic loop. Physics runs at ~60 Hz
        // which is sufficient for visual updates and collision detection.

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
    
    std::cout << "[RAMP] Starting force ramp from " << currentForce.str(3) 
              << " (mag: " << currentForce.length() << "N) over " << durationMs << "ms" << std::endl;

    const int steps = std::max(1, durationMs / 5);
    for (int i = 0; i < steps; ++i) {
        double alpha = 1.0 - static_cast<double>(i + 1) / steps;
        cVector3d f = alpha * currentForce;
        simContext.hapticDevice->setForce(f);
        
        // Log every 5th step to avoid spam
        if (i % 5 == 0 || i == steps - 1) {
            std::cout << "[RAMP] Step " << (i+1) << "/" << steps << ": force = " 
                      << f.str(3) << " (mag: " << f.length() << "N)" << std::endl;
        }
        cSleepMs(5);
    }
    simContext.hapticDevice->setForce(cVector3d(0, 0, 0));
}

bool SimulationManager::applyCursorConfig(const std::string& visualType,
    const std::map<std::string, std::string>& params) {
    // Pause haptics loop to avoid races with the haptics thread while swapping tools
    simContext.hapticsEnabled = false;
    cSleepMs(100);

    // Ensure a haptic device is available
    if (!simContext.hapticDevice) {
    std::cout << "[CURSOR] No haptic device present; cannot create tool." << std::endl;
    simContext.hapticsEnabled = true;
    return false;
    }

    // Default cursor path when no custom visual type is provided:
    // just reset to default visual/tether on the existing tool.
    if (visualType.empty()) {
        if (simContext.tool) {
            if (useMeshCursorByDefault) {
                if (auto toolMeshPtr = dynamic_cast<cToolMesh*>(simContext.tool)) {
                    const int sampleCount = 6;
                    const double contactRadius = 0.02;
                    cMultiMesh* sphereVisual = new cMultiMesh();
                    cMesh* sub = sphereVisual->newMesh();
                    cCreateSphere(sub, 0.02);
                    sphereVisual->m_material->setWhite();
                    toolMeshPtr->setVisual(sphereVisual, sampleCount, contactRadius);
                    toolMeshPtr->setTetherEnabled(false);
                    toolMeshPtr->setGravityForceScale(0.0);
                    toolMeshPtr->setWaitForSmallForce(true);
                    toolMeshPtr->initialize();
                    toolMeshPtr->setShowContactPoints(false, false);
                }
            } else if (auto cursor = dynamic_cast<cToolCursor*>(simContext.tool)) {
                const double radius = 0.01;
                cursor->setRadius(radius);
                cursor->setWaitForSmallForce(true);
                cursor->initialize();
                cursor->setShowContactPoints(false, false);
            }
        }
        simContext.hapticsEnabled = true;
        return true;
    }

    // Build visual mesh via registry if possible
    cGenericObject* visual = nullptr;

    // Try registry
    if (!visualType.empty() && CustomObjectRegistry::isTypeRegistered(visualType)) {
    auto customObj = CustomObjectRegistry::createObject(visualType);
    if (customObj) {
    // Create object at origin, white color (visual only)
    cGenericObject* obj = customObj->createObject(params, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    // Accept any scene-graph object as the visual (mesh or segments)
    if (auto mm = dynamic_cast<cMultiMesh*>(obj)) {
    visual = mm;
    } else if (auto m = dynamic_cast<cMesh*>(obj)) {
    cMultiMesh* wrap = new cMultiMesh();
    wrap->addMesh(m);
    visual = wrap;
    } else if (auto segs = dynamic_cast<cMultiSegment*>(obj)) {
    visual = segs;
    } else {
    visual = obj; // fallback generic
    }
    }
    }

    // Built-in primitive support when not using the registry
    if (!visual && !visualType.empty()) {
    std::string vt = visualType;
    std::transform(vt.begin(), vt.end(), vt.begin(), ::tolower);

    if (vt == "sphere") {
    double radius = 0.01; // default ~2cm diameter to match prior cube
    auto itR = params.find("radius");
    if (itR != params.end()) {
    double rParsed = atof(itR->second.c_str());
    if (rParsed > 0.0) radius = rParsed;
    }
    cMultiMesh* mm = new cMultiMesh(); visual = mm;
    cMesh* sub = mm->newMesh();
    cCreateSphere(sub, radius);
    mm->m_material->setWhite();
    } else if (vt == "box") {
    double w = 0.02, h = 0.02, d = 0.02; // defaults
    auto itW = params.find("width"); if (itW != params.end()) { double v = atof(itW->second.c_str()); if (v > 0.0) w = v; }
    auto itH = params.find("height"); if (itH != params.end()) { double v = atof(itH->second.c_str()); if (v > 0.0) h = v; }
    auto itD = params.find("depth"); if (itD != params.end()) { double v = atof(itD->second.c_str()); if (v > 0.0) d = v; }
    cMultiMesh* mm = new cMultiMesh(); visual = mm;
    cMesh* sub = mm->newMesh();
    cCreateBox(sub, w, h, d);
    mm->m_material->setWhite();
    } else if (vt == "cylinder") {
    double radius = 0.01, height = 0.02; // defaults
    auto itR = params.find("radius"); if (itR != params.end()) { double v = atof(itR->second.c_str()); if (v > 0.0) radius = v; }
    auto itH = params.find("height"); if (itH != params.end()) { double v = atof(itH->second.c_str()); if (v > 0.0) height = v; }
    cMultiMesh* mm = new cMultiMesh(); visual = mm;
    cMesh* sub = mm->newMesh();
    cCreateCylinder(sub, radius, height);
    mm->m_material->setWhite();
    }
    }

    // Fallback: simple sphere (should rarely happen)
    if (!visual) {
        std::cout << "[CURSOR] Visual creation failed; falling back to default sphere." << std::endl;
        cMultiMesh* mm = new cMultiMesh(); visual = mm;
        cMesh* sub = mm->newMesh();
        cCreateSphere(sub, 0.02);
        mm->m_material->setWhite();
    }

    // Optional visual color (from params: color_r,color_g,color_b)
    auto itCr = params.find("color_r");
    auto itCg = params.find("color_g");
    auto itCb = params.find("color_b");
    if (itCr != params.end() && itCg != params.end() && itCb != params.end()) {
    double rr = atof(itCr->second.c_str());
    double gg = atof(itCg->second.c_str());
    double bb = atof(itCb->second.c_str());
    rr = std::max(0.0, std::min(1.0, rr));
    gg = std::max(0.0, std::min(1.0, gg));
    bb = std::max(0.0, std::min(1.0, bb));
    if (auto mm = dynamic_cast<cMultiMesh*>(visual)) {
    if (!mm->m_material) {
    mm->m_material = cMaterial::create();
    }
    mm->m_material->setColorf(rr, gg, bb);
    } else if (auto segs = dynamic_cast<cMultiSegment*>(visual)) {
    // For segments, set line color and material color
    segs->setLineColor(cColorf(rr, gg, bb, 1.0));
    if (!segs->m_material) {
    segs->m_material = cMaterial::create();
    }
    segs->m_material->setColorf(rr, gg, bb);
    }
    }

    // Apply optional transforms
    auto itScale = params.find("scale");
    if (itScale != params.end()) {
    double s = atof(itScale->second.c_str());
    if (s > 0.0) visual->scale(s);
    }
    auto itRx = params.find("rot_x");
    auto itRy = params.find("rot_y");
    auto itRz = params.find("rot_z");
    double rx = itRx != params.end() ? atof(itRx->second.c_str()) : 0.0;
    double ry = itRy != params.end() ? atof(itRy->second.c_str()) : 0.0;
    double rz = itRz != params.end() ? atof(itRz->second.c_str()) : 0.0;
    if (rx != 0.0) visual->rotateAboutLocalAxisDeg(cVector3d(1,0,0), rx);
    if (ry != 0.0) visual->rotateAboutLocalAxisDeg(cVector3d(0,1,0), ry);
    if (rz != 0.0) visual->rotateAboutLocalAxisDeg(cVector3d(0,0,1), rz);

    // Sampling/contact parameters
    int sampleCount = 24;
    auto itSamples = params.find("sample_count");
    if (itSamples != params.end()) {
    sampleCount = std::max(1, atoi(itSamples->second.c_str()));
    }
    double contactRadius = 0.005;
    auto itContact = params.find("contact_radius");
    if (itContact != params.end()) {
    contactRadius = std::max(1e-4, atof(itContact->second.c_str()));
    }

    // Prefer updating an existing tool to avoid reopening the device
    cToolMesh* toolMeshPtr = dynamic_cast<cToolMesh*>(simContext.tool);
    if (toolMeshPtr) {
    toolMeshPtr->setVisual(visual, sampleCount, contactRadius);
    } else {
    // No existing mesh tool; create one and attach device
    destroyHapticTool();
    simContext.tool = new cToolMesh(simContext.world, visual, sampleCount, contactRadius);
    simContext.world->addChild(simContext.tool);
    simContext.tool->setHapticDevice(simContext.hapticDevice);
    simContext.tool->setWorkspaceRadius(1.0);
    toolMeshPtr = dynamic_cast<cToolMesh*>(simContext.tool);
    }

    // Optional gravity behavior
    if (toolMeshPtr) {
    // Check for tether mode (hybrid: mass attached by spring-damper)
    bool tetherEnabled = false;
    auto itTether = params.find("tether_enabled");
    if (itTether != params.end()) {
    std::string tetherStr = itTether->second;
    std::transform(tetherStr.begin(), tetherStr.end(), tetherStr.begin(), ::tolower);
    tetherEnabled = (tetherStr == "true" || tetherStr == "1");
    }

    if (tetherEnabled) {
    // Parse tether parameters with defaults
    double mass = 0.05;
    auto itMass = params.find("tether_mass");
    if (itMass != params.end()) {
    double v = atof(itMass->second.c_str());
    if (v > 0.0) mass = v;
    }

    double stiffness = 50.0;
    auto itStiff = params.find("tether_stiffness");
    if (itStiff != params.end()) {
    double v = atof(itStiff->second.c_str());
    if (v > 0.0) stiffness = v;
    }

    double damping = 2.0;
    auto itDamp = params.find("tether_damping");
    if (itDamp != params.end()) {
    double v = atof(itDamp->second.c_str());
    if (v >= 0.0) damping = v;
    }

    double gravityScale = 1.0;
    auto itGScale = params.find("tether_gravity_scale");
    if (itGScale != params.end()) {
    double v = atof(itGScale->second.c_str());
    if (v >= 0.0) gravityScale = v;
    }

    double maxForce = 30.0;
    auto itMaxF = params.find("tether_max_force");
    if (itMaxF != params.end()) {
    double v = atof(itMaxF->second.c_str());
    if (v > 0.0) maxForce = v;
    }

    // Optional anchor object id for the tether
    std::string anchorId;
    auto itAnchor = params.find("tether_anchor_id");
    if (itAnchor != params.end()) {
    anchorId = itAnchor->second;
    }

    // Configure tether
    toolMeshPtr->setTetherParams(mass, stiffness, damping, gravityScale, maxForce);
    if (!anchorId.empty()) {
    toolMeshPtr->setTetherAnchorId(anchorId);
    }
    toolMeshPtr->setTetherEnabled(true);

    // Disable other gravity modes to avoid conflicts
    toolMeshPtr->setGravityPhysics(0.0);
    toolMeshPtr->setGravityForceScale(0.0);

    std::cout << "[CURSOR] Tether enabled: mass=" << mass << "kg, k=" << stiffness 
    << "N/m, c=" << damping << "N·s/m, g_scale=" << gravityScale 
    << ", max_force=" << maxForce << "N" << std::endl;
    } else {
    // No tether: use force-mode gravity if specified
    double gfs = 0.0;
    auto itGFS = params.find("gravity_force_scale");
    if (itGFS != params.end()) {
    gfs = atof(itGFS->second.c_str());
    } else {
    // Backward compatibility: map gravity_scale to force mode
    auto itGS = params.find("gravity_scale");
    if (itGS != params.end()) {
    gfs = atof(itGS->second.c_str());
    }
    }
    toolMeshPtr->setGravityForceScale(gfs);
    // Ensure pose is device-driven (disable physics mode) for coupling
    toolMeshPtr->setGravityPhysics(0.0);
    toolMeshPtr->setTetherEnabled(false);
    }
    }

    // Optional proxy visuals
    auto itShow = params.find("show_contact_points");
    if (itShow != params.end() && (itShow->second == "true" || itShow->second == "1")) {
    simContext.tool->setShowContactPoints(true, true);
    } else {
    simContext.tool->setShowContactPoints(false, false);
    }

    // Start tool only if not already enabled (avoid re-opening device)
    if (!simContext.tool->getEnabled()) {
    simContext.tool->start();
    }
    std::cout << "[CURSOR] Applied cursor visual type: " << visualType
    << ", samples: " << sampleCount
    << ", contact_radius: " << contactRadius << std::endl;

    // Resume haptics loop
    simContext.hapticsEnabled = true;
    return true;
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

    if (useMeshCursorByDefault) {
        // Default cursor: simple mesh-based cursor (sphere)
        auto sphereMulti = new cMultiMesh();
        {
            cMesh* sub = sphereMulti->newMesh();
            cCreateSphere(sub, 0.02);
            sphereMulti->m_material->setWhite();
        }

        const int sampleCount = 6;
        // Make haptic contact points smaller than the visual cursor sphere
        const double contactRadius = 0.005;
        simContext.tool = new cToolMesh(simContext.world, static_cast<cGenericObject*>(sphereMulti), sampleCount, contactRadius);
        simContext.world->addChild(simContext.tool);
        simContext.tool->setHapticDevice(simContext.hapticDevice);
        simContext.tool->enableDynamicObjects(true);
        simContext.tool->setWorkspaceRadius(1.0);
        simContext.tool->setWaitForSmallForce(true);
        simContext.tool->setShowContactPoints(false, false);
        simContext.tool->start();
        std::cout << "[DEBUG] Created and started new mesh cursor tool (default)." << std::endl;
    } else {
        // Default cursor: CHAI3D cToolCursor
        auto cursor = new cToolCursor(simContext.world);
        simContext.tool = cursor;
        simContext.world->addChild(cursor);

        cursor->setHapticDevice(simContext.hapticDevice);
        cursor->setWorkspaceRadius(1.0);

        const double cursorRadius = 0.012;
        cursor->setRadius(cursorRadius);
        cursor->setWaitForSmallForce(true);
        cursor->setShowContactPoints(false, false);
        cursor->start();
        std::cout << "[DEBUG] Created and started new CHAI cursor tool." << std::endl;
    }
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
