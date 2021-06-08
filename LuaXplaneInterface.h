#ifndef LUA_XPLANE_INTERFACE
#define LUA_XPLANE_INTERFACE

#include "lua\lauxlib.h"

luaL_Reg const* getLuaXPlaneFunctionTable();
void runLuaFlightLoops(lua_State* L);
void clearFlightLoops();

#endif