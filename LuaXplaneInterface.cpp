#include <forward_list>

#include "pch.h"
extern "C" {
#include "lua\lauxlib.h"
}

std::forward_list<lua_State*> flightLoops;

extern "C" {
    static int lua_xplaneFindDataRef(lua_State* L)
    {
        int n = lua_gettop(L);

        if (n != 1 || !lua_isstring(L, 1))
        {
            // Clear Arguments and throw an error.
            lua_pop(L, n);
            lua_pushliteral(L, "XPlaneFindDataRef expects a single string as an argument.");
            lua_error(L);
            return 0;
        }

        // Argument is a string.
        const char* name = lua_tostring(L, 1);

        // Read void * data ref as chars.
        XPLMDataRef ref = XPLMFindDataRef(name);
        char const* const refasbytes = reinterpret_cast<char const * const>(&ref);

        // Clear arguments
        lua_pop(L, 1);

        // Encode pointer data as a lua table for
        // reliability across platforms.
        lua_createtable(L, sizeof(XPLMDataRef), 0);

        for (int i = 0; i < sizeof(XPLMDataRef); ++i)
        {
            lua_pushinteger(L, i);
            lua_pushinteger(L, refasbytes[i]);
            lua_settable(L, 1);
        }

        return 1;
    }

    static int lua_xplaneReadDataFloat(lua_State* L)
    {
        int n = lua_gettop(L);

        if (n != 1 || !lua_istable(L, 1))
        {
            // Clear Arguments and throw an error.
            lua_pop(L, n);
            lua_pushliteral(L, "XPlaneReadDataFloat expects a single table found using XPlaneFindDataRef.");
            lua_error(L);
            return 0;
        }

        // Unpack the XPLMDataRef,
        XPLMDataRef dataref = NULL;
        char* const datarefasbytes = reinterpret_cast<char * const>(&dataref);

        for (int i = 0; i < sizeof(XPLMDataRef); ++i)
        {
            lua_pushinteger(L, i);
            lua_gettable(L, 1);

            datarefasbytes[i] = (char)lua_tointeger(L, 2);
            lua_pop(L, 1);
        }

        // Read Float.
        float data = XPLMGetDataf(dataref);

        // Clear arguments.
        lua_pop(L, 1);

        // Push Return Value (the Float)
        lua_pushnumber(L, data);

        return 1;
    }

    static int lua_xplaneWriteDataFloat(lua_State* L)
    {
        int n = lua_gettop(L);

        if (n != 2 || !lua_istable(L, 1) || !lua_isnumber(L, 2))
        {
            // Clear Arguments and throw an error.
            lua_pop(L, n);
            lua_pushliteral(L, "XPlaneWriteDataFloat expects a table, found using XPlaneFindDataRef, and a Lua number.");
            lua_error(L);
            return 0;
        }

        // Read the float and pop it off
        float data = lua_tonumber(L, 2);
        lua_pop(L, 1);

        // Unpack the XPLMDataRef,
        XPLMDataRef dataref = NULL;
        char* const datarefasbytes = reinterpret_cast<char* const>(&dataref);

        for (int i = 0; i < sizeof(XPLMDataRef); ++i)
        {
            lua_pushinteger(L, i);
            lua_gettable(L, 1);

            datarefasbytes[i] = (char)lua_tointeger(L, 2 + i);
        }

        // Clear the stack
        lua_settop(L, 0);

        // Write Float.
        XPLMSetDataf(dataref, data);

        return 0;
    }

    static int lua_xplaneRegisterFlightLoop(lua_State* L)
    {
        int n = lua_gettop(L);

        if (n != 1 || !lua_isthread(L, 1))
        {
            // Clear Arguments and throw an error.
            lua_pop(L, n);
            lua_pushliteral(L, "RegisterFlightLoop expects a Lua Thread (resumeable coroutine).");
            lua_error(L);
            return 0;
        }

        // Pop Coroutine Off
        lua_State* thread = lua_tothread(L, 1);
        lua_pop(L, 1);

        // Store Coroutine in List of Flight Loops.
        flightLoops.push_front(thread);

        return 0;
    }

    static luaL_Reg LuaXPlaneFunctions[] =
    {
        {
            "findDataRef",
            lua_xplaneFindDataRef
        },
        {
            "readDataFloat",
            lua_xplaneReadDataFloat
        },
        {
            "writeDataFloat",
            lua_xplaneWriteDataFloat
        },
        {
            "registerFlightLoop",
            lua_xplaneRegisterFlightLoop
        },
        {
            NULL,
            NULL
        }
    };

    luaL_Reg const* getLuaXPlaneFunctionTable()
    {
        return LuaXPlaneFunctions;
    }

    void clearFlightLoops()
    {
        flightLoops.clear();
    }

    void runLuaFlightLoops(lua_State* L)
    {
        std::forward_list<lua_State*> badCalls;
        for (auto coroutine : flightLoops)
        {
            int nReturned = 0;
            int code = lua_resume(coroutine, L, 0, &nReturned);
            if (LUA_YIELD != code || nReturned != 0)
            {
                // Stop callbacks from this flight loop, register
                // this iterator to be removed.
                badCalls.push_front(coroutine);
            }
        }
        for (auto badcoroutine : badCalls)
        {
            std::remove(flightLoops.begin(), flightLoops.end(), badcoroutine);
        }
    }
}