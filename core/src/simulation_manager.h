#ifndef SIMULATION_MANAGER_H
#define SIMULATION_MANAGER_H

#pragma once
#include "chai3d.h"
#include <GLFW/glfw3.h>
#include <mutex>
#include <unordered_map>

using namespace chai3d;

#define DATA_POLL_FREQUENCY 10 // Hz

struct SimulationContext {
    // stereo Mode
    /*
        C_STEREO_DISABLED:            Stereo is disabled 
        C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
        C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
        C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
    */
    cStereoMode stereoMode = C_STEREO_DISABLED;

    // fullscreen mode
    bool fullscreen = false;

    // mirrored display
    bool mirroredDisplay = false;

    // a world that contains all objects of the virtual environment
    cWorld* world;

    // a camera to render the world in the window display
    cCamera* camera;

    // map object ID's to pointers to the actual objects
    std::unordered_map<std::string, cGenericObject*> objectMap;

    // a light source to illuminate the objects in the world
    cDirectionalLight *light;

    // a haptic device handler
    cHapticDeviceHandler* handler;

    // a pointer to the current haptic device
    cGenericHapticDevicePtr hapticDevice;

    // a label to display the rates [Hz] at which the simulation is running
    cLabel* labelRates;

    // a label to display the current status, i.e. "waiting for trial to begin"
    cLabel* statusText;

    // a virtual tool representing the haptic device in the scene
    cToolCursor* tool;

    // flag to indicate if the haptic simulation currently running
    bool simulationRunning = false;

    // flag to indicate if the haptic simulation has terminated
    bool simulationFinished = false;

    // flag to indicate data thread running
    bool dataThreadRunning = false;

    // a frequency counter to measure the simulation graphic rate
    cFrequencyCounter freqCounterGraphics;

    // a frequency counter to measure the simulation haptic rate
    cFrequencyCounter freqCounterHaptics;

    // haptic thread
    cThread* hapticsThread;

    // debug thread
    cThread* dataThread;

    // a handle to window display context
    GLFWwindow* window = NULL;

    // current width of window
    int width  = 0;

    // current height of window
    int height = 0;

    // swap interval for the display context (vertical synchronization)
    int swapInterval = 1;

    bool debug = false;
};

struct TrialContext {
    bool trialRunning = false;
    double time = 0;

    std::string trialId = "";
    double trialDuration = 0;
};

// Singleton SimulationManager class
class SimulationManager {
public:
    // data structure with "global variables" needed
    static SimulationContext simContext;

    // trial info to manage
    static TrialContext trialContext;

    // runs the simulation
    static bool run();

    // sets debug mode
    static void setDebugMode(bool debug);

    // SCENE SETTINGS
    static bool reset();
    static bool setBackgroundColor(double r, double g, double b);

    // GENERAL OBJECT FUNCTIONALITY

    /*
    NOTE: Once an object is added to the simulation via addObject(),
    the SimulationManager takes full ownership and is responsible for
    deleting the object. Do not manually delete or reuse pointers added here.
    */

    static bool addObject(std::string id, cGenericObject* obj, double x, double y, double z);
    static bool removeObject(std::string id);
    static bool removeAllObjects();

    // OBJECT HELPERS
    static bool addSphere(std::string id, double radius, double x, double y, double z);
private:
    // callback when the window display is resized
    static void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

    // callback when an error GLFW occurs
    static void errorCallback(int error, const char* a_description);

    // callback when a key is pressed
    static void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

    // this function renders the scene
    static void updateGraphics(void);

    // this function contains the main haptics simulation loop
    static void updateHaptics(void);

    // Thread to poll for data
    static void dataPollingThread();

    // this function closes the application
    static void close(void);
};

#endif