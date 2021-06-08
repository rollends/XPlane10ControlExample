#include "pch.h"

#include <math.h>
#include <string.h>

#include "LuaXplaneInterface.h"
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

PLUGIN_API float PrimaryFlightLoop(
    float   inElapsedSinceLastCall,
    float   inElapsedTimeSinceLastFlightLoop,
    int     inCounter,
    void*   inRefcon
)
{
    runLuaFlightLoops(luaState);
    return 0.050f; // ~20 FPS
}

XPLMFlightLoopID PrimaryFlightLoopID;

PLUGIN_API int XPluginEnable(void)
{
    // Initialize Lua First
    luaState = luaL_newstate();
    if (!luaState)
    {
        return 0; // Failed to Initialize the Lua [Interpreter] State.
    }
    luaL_openlibs(luaState);

    // Add Custom XPLane Functions
    lua_createtable(luaState, 0, 0);
    luaL_setfuncs(luaState, getLuaXPlaneFunctionTable(), 0);
    lua_setglobal(luaState, "xplane");

    // Add Control Functions
    int code = luaL_dofile(luaState, "Resources\\plugins\\control.lua");
    if (LUA_OK != code)
    {
        return 0; // failed to load control library.
    }

    // Load Initial Script (Absolute Path For now)
    code = luaL_dofile(luaState, "C:\\Users\\rolle\\source\\repos\\XPlane10ControlExample\\test1.lua");
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
    return 1;
}

PLUGIN_API int XPluginDisable(void)
{
    XPLMDestroyFlightLoop(PrimaryFlightLoopID);

    // Cleanup the Lua State
    lua_close(luaState);

    // Clean up all flight loops
    clearFlightLoops();

    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMessage, void* inParam)
{
    // Messaging.
}