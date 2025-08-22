#include "simulation_manager.h"
#include <iostream>

SimulationContext SimulationManager::simContext;
TrialContext SimulationManager::trialContext;

bool SimulationManager::run() {
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

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
    simContext.camera->set( cVector3d (0.5, 0.0, 0.0),    // camera position (eye)
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

    // create a tool (cursor) and insert into the world
    simContext.tool = new cToolCursor(simContext.world);
    simContext.world->addChild(simContext.tool);

    // connect the haptic device to the virtual tool
    simContext.tool->setHapticDevice(simContext.hapticDevice);

    // define a radius for the virtual tool (sphere)
    simContext.tool->setRadius(0.03);

    // map the physical workspace of the haptic device to a larger virtual workspace.
    simContext.tool->setWorkspaceRadius(1.0);

    // haptic forces are enabled only if small forces are first sent to the device;
    // this mode avoids the force spike that occurs when the application starts when 
    // the tool is located inside an object for instance. 
    simContext.tool->setWaitForSmallForce(true);

    // start the haptic tool
    simContext.tool->start();

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
    simContext.dataThread = new cThread();
    simContext.dataThread->start(dataPollingThread, CTHREAD_PRIORITY_GRAPHICS);

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

    setBackgroundColor(0.0, 0.0, 0.0);
    removeAllObjects();
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

    // Add the object into the world
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

    // Add the object to the map
    simContext.objectMap[id] = obj;
    std::cout << "[DEBUG] Added object with ID " << id << " at position (" << x << ", " << y << ", " << z << ")." << std::endl;

    return true;
}

bool SimulationManager::removeObject(std::string id) {
    if (!simContext.simulationRunning) {
        std::cout << "[ERROR] Simulation is not running." << std::endl;
        return false;
    }

    bool success;

    // Remove the object from the world
    auto it = simContext.objectMap.find(id);
    if (it != simContext.objectMap.end()) {
        cGenericObject* obj = it->second;
        simContext.world->removeChild(obj);
        delete obj;
        simContext.objectMap.erase(it);
        std::cout << "[DEBUG] Removed object with ID " << id << "." << std::endl;
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

    std::vector<std::string> ids;
    for (const auto& pair : simContext.objectMap) {
        ids.push_back(pair.first);
    }

    for (const auto& id : ids) {
        removeObject(id);
    }

    std::cout << "[DEBUG] Removed all objects." << std::endl;
    return true;
}

bool SimulationManager::addSphere(std::string id, double radius, double x, double y, double z) {
    cMesh* sphere = new cMesh();
    cCreateSphere(sphere, radius);
    sphere->createAABBCollisionDetector(radius);

    return addObject(id, sphere, x, y, z);
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
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // update haptic and graphic rate data
    simContext.labelRates->setText(cStr(simContext.freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        cStr(simContext.freqCounterHaptics.getFrequency(), 0) + " Hz");

    // update position of label
    simContext.labelRates->setLocalPos((int)(0.5 * (simContext.width - simContext.labelRates->getWidth())), 15);

    // update status text
    if (trialContext.trialRunning) {
        simContext.statusText->setText("Trial running: " + trialContext.trialId + " | t = " + cStr(trialContext.time, 4));
    } else {
        simContext.statusText->setText("Waiting for a trial to begin");
    }

    simContext.statusText->setLocalPos(5, simContext.height - simContext.statusText->getHeight() - 5);

    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    simContext.world->updateShadowMaps(false, simContext.mirroredDisplay);

    // render world
    simContext.camera->renderView(simContext.width, simContext.height);

    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) std::cout << "Error:  %s\n" << gluErrorString(err);
}

// this function contains the main haptics simulation loop
void SimulationManager::updateHaptics(void) {
    simContext.simulationRunning  = true;
    simContext.simulationFinished = false;

    while (simContext.simulationRunning) {
        // update world transforms
        simContext.world->computeGlobalPositions(true);

        // update tool from device
        simContext.tool->updateFromDevice();

        // compute interaction forces
        simContext.tool->computeInteractionForces();

        auto hapticPoint = simContext.tool->getHapticPoint(0);
        cVector3d proxyPos = hapticPoint->getGlobalPosProxy();
        cVector3d computedForce = hapticPoint->getLastComputedForce();
        

        // apply forces
        simContext.tool->applyToDevice();

        // update haptic loop frequency
        simContext.freqCounterHaptics.signal(1);
    }

    simContext.simulationFinished = true;
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
                
                if (simContext.debug) {
                    std::cout << "[DEBUG] t: " << trialContext.time
                            << " | Device pos: " << devicePos
                            << " | Proxy pos: " << proxyPos
                            << " | Device force: " << deviceForce
                            << " | Computed force: " << computedForce << std::endl;
                }
            }

            cSleepMs(1000 / DATA_POLL_FREQUENCY);
        }
    }

    simContext.dataThreadRunning = false;
}

// this function closes the application
void SimulationManager::close(void) {
    // remove all objects from the world
    removeAllObjects();

    // stop the simulation
    simContext.simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simContext.simulationFinished) { cSleepMs(100); }

    // free resources
    simContext.tool->stop();
    
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