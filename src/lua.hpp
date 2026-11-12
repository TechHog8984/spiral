/*
    lua.hpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#pragma once

#include <cstdint>

// NOTE: what the fuck...
#define LUAI_MAXCSTACK 1000000

#define LUA_REGISTRYINDEX (-LUAI_MAXCSTACK - 2000)
#define LUA_ENVIRONINDEX (-LUAI_MAXCSTACK - 2001)
#define LUA_GLOBALSINDEX (-LUAI_MAXCSTACK - 2002)

typedef int (*lua_CFunction)(uint8_t *L);
typedef int (*lua_Continuation)(uint8_t *L, int status);

namespace spiral::lua {

int lua_ref(uint8_t *L, int idx);

}  // namespace spiral::lua
