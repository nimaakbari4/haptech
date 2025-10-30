#pragma once
#ifndef SIMULATION_MANAGER_H
#define SIMULATION_MANAGER_H
#define DATA_POLL_FREQUENCY 500 // Hz
#define HAPTICS_LOOP_FREQUENCY 1000 // Hz

#include "chai3d.h"
#include "trial_conditions/trial_condition.h"
#include "objects/custom_object_interface.h"
#include <GLFW/glfw3.h>
#include <mutex>
#include <unordered_map>
#include <set>

using namespace chai3d;

struct SimulationContext {
    // flag to enable/disable haptic calculations
    bool hapticsEnabled = true;
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
    std::unordered_map<std::string, std::unique_ptr<cGenericObject>> objectMap;

    // also map object ID's to pointers to custom object interface
    std::unordered_map<std::string, std::unique_ptr<CustomObjectInterface>> objectInterfaceMap;

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
    cGenericTool* tool;

    // Set of object IDs pending deletion (processed by graphics thread)
    std::set<std::string> pendingDelete;

    // Mutex to protect shared object maps from race conditions between threads
    std::mutex objectMapMutex;

    // flag to indicate if the haptic simulation currently running
    bool simulationRunning = true;

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
    double startTime = 0;

    std::string trialId = "";
    std::vector<std::unique_ptr<TrialConditionInterface>> startConditions;
    std::vector<std::unique_ptr<TrialConditionInterface>> endConditions;
    std::unique_ptr<TrialConditionInterface> startConditionMet;
    std::unique_ptr<TrialConditionInterface> endConditionMet;
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

    // checks trial conditions - returns the one that was satisfied
    static void checkTrialConditions();
    static void checkStartConditions();
    static void checkEndConditions();

    // SCENE SETTINGS
    static bool reset();
    static bool setBackgroundColor(double r, double g, double b);

    // GENERAL OBJECT FUNCTIONALITY

    /*
    NOTE: Once an object is added to the simulation via addObject(),
    the SimulationManager takes full ownership and is responsible for
    deleting the object. Do not manually delete or reuse pointers added here.
    */

    static bool addObject(std::string id, cGenericObject* obj, 
        double px, double py, double pz,
        double rx, double ry, double rz);
    static bool removeObject(std::string id);
    static bool removeAllObjects();

    // OBJECT HELPERS
    static bool addSphere(std::string id, double radius, double px, double py, double pz);
    static bool addObjectFromConfig(const std::string& id, const std::string& type, 
                                   double px, double py, double pz,
                                   double rx, double ry, double rz,
                                   double r, double g, double b,
                                   const std::map<std::string, std::string>& params);
    
    // ramp device force to zero over given milliseconds (exposed for server teardown)
    static void rampDeviceForceToZero(int durationMs);

private:
    // haptic tool management
    static void createHapticTool();
    static void destroyHapticTool();
    
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