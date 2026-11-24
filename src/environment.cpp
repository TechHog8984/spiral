/*
    environment.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#include "environment.hpp"

#include <cstdio>
#include <string_view>

#include "execution.hpp"
#include "gamevalues.hpp"
#include "logging.hpp"
#include "lua.hpp"

namespace spiral::environment {

using namespace gamevalues;

inline void add(uint8_t *L, const char *name, lua_CFunction func) {
    lua_pushcclosurek.trampoline(L, func, name, 0, NULL);
    lua_rawsetfield.trampoline(L, LUA_GLOBALSINDEX, name);
}

void apply(uint8_t *L) {
    lua_setreadonly.trampoline(L, LUA_GLOBALSINDEX, false);

    add(L, "print", [](uint8_t *L) -> int {
        std::string content;
        content.reserve(32);

        int n = lua_gettop.trampoline(L);
        for (int i = 1; i <= n; i++) {
            if (i > 1) content += '\t';

            size_t l;
            const char *s = luaL_tolstring.trampoline(L, i, &l);

            content += std::string_view(s, l);

            lua_settop.trampoline(L, -2);
        }
        content += '\n';

        logging::print(content);
        return 0;
    });

    add(L, "getrawmetatable", [](uint8_t *L) -> int {
        luaL_checkany.trampoline(L, 1);

        if (!lua_getmetatable.trampoline(L, 1)) lua_pushnil.trampoline(L);
        return 1;
    });
    add(L, "setrawmetatable", [](uint8_t *L) -> int {
        // NOTE: we could also just lua_insert(L, 2) before the setmetatable
        // call for consistency sake...
        if (lua_gettop.trampoline(L) > 2)
            luaL_error(L, "too many arguments to setrawmetatable! expected 2");

        luaL_checkany.trampoline(L, 1);
        if (!(lua_isnil(L, 2) || lua_istable(L, 2)))
            luaL_error(
                L,
                "invalid argument #2 to setrawmetatable; expected nil or table");

        lua_setmetatable.trampoline(L, 1);
        return 0;
    });

    add(L, "getreg", [](uint8_t *L) -> int {
        lua_pushvalue.trampoline(L, LUA_REGISTRYINDEX);
        return 1;
    });
    add(L, "getgenv", [](uint8_t *L) -> int {
        lua_pushvalue.trampoline(execution::spiral_state, LUA_GLOBALSINDEX);
        lua_xmove.trampoline(execution::spiral_state, L, 1);
        return 1;
    });

    add(L, "isreadonly", [](uint8_t *L) -> int {
        luaL_checktype.trampoline(L, 1, LUA_TTABLE);

        lua_pushboolean.trampoline(L, lua_getreadonly.trampoline(L, 1));
        return 1;
    });
    add(L, "setreadonly", [](uint8_t *L) -> int {
        luaL_checktype.trampoline(L, 1, LUA_TTABLE);

        lua_setreadonly.trampoline(L, 1, lua::luaL_checkboolean(L, 2));
        return 0;
    });

    add(L, "getnamecallmethod", [](uint8_t *L) -> int {
        auto namecall = luastate_namecall(L);
        const char *method = namecall ? getstr(namecall) : NULL;
        if (method)
            lua::lua_pushstring(L, method);
        else
            luaL_error(L, "not in namecall context!");

        return 1;
    });

    lua_setreadonly.trampoline(L, LUA_GLOBALSINDEX, true);
}

}  // namespace spiral::environment
