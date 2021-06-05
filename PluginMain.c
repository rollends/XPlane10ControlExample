#include "pch.h"

#include <math.h>
#include <string.h>

#include "lua\lauxlib.h"

char const PluginName[]           = "Basic Control System";
char const PluginSignature[]      = "ca.rollends.xplane.basic";
char const PluginDescription[]    = "No Description.";

PLUGIN_API int XPluginStart(char* outName, char* outSignature, char* outDescription)
{
    memcpy_s(outName, 256, PluginName, sizeof(PluginName));
    memcpy_s(outSignature, 256, PluginSignature, sizeof(PluginSignature));
    memcpy_s(outDescription, 256, PluginDescription, sizeof(PluginDescription));
    return 1;
}

PLUGIN_API void XPluginStop(void)
{
    // Do Nothing for Now.
}

static lua_State * luaState = NULL;
static XPLMDataRef xplmData_zulu_time_sec;
static XPLMDataRef xplmData_yoke_pitch_ratio;
static XPLMDataRef xplmData_the_vac_ind_deg;
static XPLMDataRef xplmData_phi_vac_ind_deg;
static XPLMDataRef xplmData_yoke_roll_ratio;

PLUGIN_API float PrimaryFlightLoop(
    float   inElapsedSinceLastCall,
    float   inElapsedTimeSinceLastFlightLoop,
    int     inCounter,
    void*   inRefcon
)
{
    static float lastPitch = NAN, lastRoll = NAN;

    // Push onto the Stack the Coroutine.
    lua_getglobal(luaState, "PitchControllerCoroutine");
    lua_State* corPitchController = lua_tothread(luaState, -1);
    lua_pop(luaState, 1);

    lua_getglobal(luaState, "RollControllerCoroutine");
    lua_State* corRollController = lua_tothread(luaState, -1);
    lua_pop(luaState, 1);

    // Indicated Pitch & Roll
    float pitch = XPLMGetDataf(xplmData_the_vac_ind_deg);
    float roll = XPLMGetDataf(xplmData_phi_vac_ind_deg);

    if (isnan(lastPitch) || isnan(lastRoll))
    {
        lastPitch = pitch;
        lastRoll = roll;
    }

    // Estimated Pitch Angular Velocity
    float pitchVelocity = (pitch - lastPitch) / inElapsedTimeSinceLastFlightLoop;
    float rollVelocity = (roll - lastRoll) / inElapsedTimeSinceLastFlightLoop;

    // Pass Parameters
    lua_pushnumber(corPitchController, pitch);
    lua_pushnumber(corPitchController, pitchVelocity);
    lua_pushnumber(corRollController, roll);
    lua_pushnumber(corRollController, rollVelocity);

    // Resume Coroutine
    int nReturned = 0;
    int code = lua_resume(corPitchController, luaState, 2, &nReturned);
    if (LUA_YIELD != code || nReturned != 1)
    {
        // Stop Callbacks. Lua Script is bad!
        return 0;
    }

    code = lua_resume(corRollController, luaState, 2, &nReturned);
    if (LUA_YIELD != code || nReturned != 1)
    {
        // Stop Callbacks. Lua Script is bad!
        return 0;
    }

    float controlPitch = lua_tonumber(corPitchController, 1);
    float controlRoll = lua_tonumber(corRollController, 1);
    lua_pop(corPitchController, 1);
    lua_pop(corRollController, 1);

    // Write Joystick data.
    XPLMSetDataf(xplmData_yoke_pitch_ratio, controlPitch);
    XPLMSetDataf(xplmData_yoke_roll_ratio, controlRoll);

    return 0.050f; // ~20 FPS
}

static XPLMFlightLoopID PrimaryFlightLoopID;

PLUGIN_API int XPluginEnable(void)
{
    // Initialize Lua First
    luaState = luaL_newstate();
    if (!luaState)
    {
        return 0; // Failed to Initialize the Lua [Interpreter] State.
    }
    luaL_openlibs(luaState);

    // Load Initial Script (Absolute Path For now)
    int code = luaL_dofile(luaState, "C:\\Users\\rolle\\source\\repos\\XPlane10ControlExample\\test1.lua");
    if (LUA_OK != code)
    {
        return 0; // Failed to Load the Script.
    }

    // Register Flight Loop
    XPLMCreateFlightLoop_t flightLoop;
    flightLoop.structSize = sizeof(XPLMCreateFlightLoop_t);
    flightLoop.refcon = NULL;
    flightLoop.phase = xplm_FlightLoop_Phase_BeforeFlightModel;
    flightLoop.callbackFunc = &PrimaryFlightLoop;

    PrimaryFlightLoopID = XPLMCreateFlightLoop(&flightLoop);
    XPLMScheduleFlightLoop(PrimaryFlightLoopID, 0.050f, 0);

    // Acquire Reference to Data Variables
    xplmData_yoke_pitch_ratio = XPLMFindDataRef("sim/joystick/yoke_pitch_ratio");
    xplmData_yoke_roll_ratio = XPLMFindDataRef("sim/joystick/yoke_roll_ratio");
    xplmData_zulu_time_sec = XPLMFindDataRef("sim/time/zulu_time_sec");
    xplmData_the_vac_ind_deg = XPLMFindDataRef("sim/cockpit/gyros/the_vac_ind_deg");
    xplmData_phi_vac_ind_deg = XPLMFindDataRef("sim/cockpit/gyros/phi_vac_ind_deg");
    return 1;
}

PLUGIN_API int XPluginDisable(void)
{
    XPLMDestroyFlightLoop(PrimaryFlightLoopID);

    // Cleanup the Lua State
    lua_close(luaState);

    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMessage, void* inParam)
{
    // Messaging.
}