/*
    environment.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#include "environment.hpp"

#include <cstdio>

#include "gamevalues.hpp"

namespace spiral::environment {

inline void writestring(const char *s, size_t l) { fwrite(s, 1, l, stdout); }

inline void push(uint8_t *L, const char *name, lua_CFunction func) {
    gamevalues::lua_pushcclosurek.trampoline(L, func, name, 0, NULL);
    gamevalues::lua_rawsetfield.trampoline(L, LUA_GLOBALSINDEX, name);
}

void apply(uint8_t *L) {
    gamevalues::lua_setreadonly.trampoline(L, LUA_GLOBALSINDEX, false);

    push(L, "print", [](uint8_t *L) -> int {
        int n = gamevalues::lua_gettop.trampoline(L);
        for (int i = 1; i <= n; i++) {
            size_t l;
            const char *s = gamevalues::luaL_tolstring.trampoline(L, i, &l);
            if (i > 1) writestring("\t", 1);
            writestring(s, l);
            gamevalues::lua_settop.trampoline(L, -2);
        }
        writestring("\n", 1);
        return 0;
    });

    gamevalues::lua_setreadonly.trampoline(L, LUA_GLOBALSINDEX, true);
}

}  // namespace spiral::environment
