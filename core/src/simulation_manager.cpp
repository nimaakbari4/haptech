#include "simulation_manager.h"
#include "custom_object_interface.h"
#include "researcher_objects_registry.h"
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
    removeAllObjects();
    
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

bool SimulationManager::addObject(std::string id, cGenericObject* obj, double x, double y, double z) {
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
    obj->setLocalPos(x, y, z);

    // Get workspace scale
    double workspaceScaleFactor = simContext.tool->getWorkspaceScaleFactor();
    cHapticDeviceInfo hapticDeviceInfo = simContext.hapticDevice->getSpecifications();
    double maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;

    // Assign material
    cMaterialPtr material = cMaterial::create();
    material->setStiffness(maxStiffness);
    obj->setMaterial(material);

    // Add the object to the map using unique_ptr
    simContext.objectMap[id] = std::unique_ptr<cGenericObject>(obj);
    std::cout << "[DEBUG] Added object with ID " << id << " at position (" << x << ", " << y << ", " << z << ")." << std::endl;

    return true;
}

bool SimulationManager::removeObject(std::string id) {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    bool success;

	// Defer deletion: mark for removal and let graphics thread remove safely
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

    return success;
}

bool SimulationManager::removeAllObjects() {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    for (const auto& pair : simContext.objectMap) {
        removeObject(pair.first);
    }

    // All objects are now removed and deleted by unique_ptr
    std::cout << "[DEBUG] Removed all objects." << std::endl;
    return true;
}

bool SimulationManager::addSphere(std::string id, double radius, double x, double y, double z) {
    std::unique_ptr<cMesh> sphere = std::make_unique<cMesh>();
    cCreateSphere(sphere.get(), radius);
    sphere->createAABBCollisionDetector(radius);

    return addObject(id, sphere.release(), x, y, z);
}

bool SimulationManager::addObjectFromConfig(const std::string& id, const std::string& type, 
                                           double x, double y, double z, 
                                           double r, double g, double b,
                                           const std::map<std::string, std::string>& params) {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    // First, try to create using custom object system
    if (CustomObjectRegistry::isTypeRegistered(type)) {
        std::cout << "[DEBUG] Creating custom object: " << type << std::endl;
        
        auto customObj = CustomObjectRegistry::createObject(type);
        if (customObj) {
            cGenericObject* chaiObj = customObj->createObject(params, x, y, z, r, g, b);
            if (chaiObj) {
                // Add to world and object map
                simContext.world->addChild(chaiObj);
                simContext.objectMap[id] = std::unique_ptr<cGenericObject>(chaiObj);
                std::cout << "[DEBUG] Added custom object with ID " << id << " at position (" << x << ", " << y << ", " << z << ")." << std::endl;
                return true;
            }
        }
        std::cout << "[ERROR] Failed to create custom object: " << type << std::endl;
        return false;
    }

    // Fall back to built-in object types
    std::unique_ptr<cMesh> mesh = std::make_unique<cMesh>();
    
    if (type == "sphere") {
        auto radiusIt = params.find("radius");
        if (radiusIt == params.end()) {
            std::cout << "[ERROR] Sphere requires 'radius' parameter." << std::endl;
            return false;
        }
        double radius = std::stod(radiusIt->second);
        cCreateSphere(mesh.get(), radius);
        mesh->createAABBCollisionDetector(radius);
    }
    else if (type == "box") {
        auto widthIt = params.find("width");
        auto heightIt = params.find("height");
        auto depthIt = params.find("depth");
        
        if (widthIt == params.end() || heightIt == params.end() || depthIt == params.end()) {
            std::cout << "[ERROR] Box requires 'width', 'height', and 'depth' parameters." << std::endl;
            return false;
        }
        double width = std::stod(widthIt->second);
        double height = std::stod(heightIt->second);
        double depth = std::stod(depthIt->second);
        cCreateBox(mesh.get(), width, height, depth);
        double maxDim = std::max({width, height, depth});
        mesh->createAABBCollisionDetector(maxDim);
    }
    else if (type == "cylinder") {
        auto radiusIt = params.find("radius");
        auto heightIt = params.find("height");

        if (radiusIt == params.end() || heightIt == params.end()) {
            std::cout << "[ERROR] Cylinder requires 'radius' and 'height' parameters." << std::endl;
            return false;
        }
        double radius = std::stod(radiusIt->second);
        double height = std::stod(heightIt->second);
        cCreateCylinder(mesh.get(), radius, height);
        double maxDim = std::max(radius, height);
        mesh->createAABBCollisionDetector(maxDim);
    }
    else if (type == "ball") {
        // Custom ball object with bounciness
        auto radiusIt = params.find("radius");
        if (radiusIt == params.end()) {
            std::cout << "[ERROR] Ball requires 'radius' parameter." << std::endl;
            return false;
        }
        double radius = std::stod(radiusIt->second);
        cCreateSphere(mesh.get(), radius);
        mesh->createAABBCollisionDetector(radius);
        
        // Set bounciness if provided
        auto bouncinessIt = params.find("bounciness");
        if (bouncinessIt != params.end()) {
            double bounciness = std::stod(bouncinessIt->second);
            mesh->m_material->setDamping(50.0 * (1.0 - bounciness)); // Less damping = more bouncy
        }
    }
    else {
        std::cout << "[ERROR] Unknown object type: " << type << std::endl;
        return false;
    }

    // Set color
    mesh->m_material->m_ambient.set(r, g, b);
    mesh->m_material->m_diffuse.set(r, g, b);
    mesh->m_material->m_specular.set(r, g, b);

    std::cout << "[DEBUG] About to call addObject for: " << id << std::endl;
    bool result = addObject(id, mesh.release(), x, y, z);
    std::cout << "[DEBUG] addObject result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    return result;
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

    std::cout << "Haptics thread started, simRunning = " << simContext.simulationRunning << std::endl;

    while (simContext.simulationRunning) {
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

void SimulationManager::createHapticTool() {
    if (!simContext.handler || !simContext.world) {
        std::cout << "[ERROR] Cannot create haptic tool: handler or world is null." << std::endl;
        return;
    }

    // create a tool (cursor) and insert into the world
    simContext.tool = new cToolCursor(simContext.world);
    simContext.world->addChild(simContext.tool);

    // connect the haptic device to the virtual tool
    simContext.tool->setHapticDevice(simContext.hapticDevice);

    // define a larger radius for the virtual tool (display + contact)
    simContext.tool->setRadius(0.05);
    simContext.tool->setRadiusDisplay(0.05);
    // show proxy/goal spheres for visibility
    // simContext.tool->setShowContactPoints(true, true);

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
