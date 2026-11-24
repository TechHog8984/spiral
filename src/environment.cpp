/*
    environment.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#include "environment.hpp"

#include <cstdio>
#include <string_view>

#include "gamevalues.hpp"
#include "logging.hpp"
#include "lua.hpp"

namespace spiral::environment {

inline void add(uint8_t *L, const char *name, lua_CFunction func) {
    gamevalues::lua_pushcclosurek.trampoline(L, func, name, 0, NULL);
    gamevalues::lua_rawsetfield.trampoline(L, LUA_GLOBALSINDEX, name);
}

void apply(uint8_t *L) {
    gamevalues::lua_setreadonly.trampoline(L, LUA_GLOBALSINDEX, false);

    add(L, "print", [](uint8_t *L) -> int {
        std::string content;
        content.reserve(32);

        int n = gamevalues::lua_gettop.trampoline(L);
        for (int i = 1; i <= n; i++) {
            if (i > 1) content += '\t';

            size_t l;
            const char *s = gamevalues::luaL_tolstring.trampoline(L, i, &l);

            content += std::string_view(s, l);

            gamevalues::lua_settop.trampoline(L, -2);
        }
        content += '\n';

        logging::print(content);
        return 0;
    });

    gamevalues::lua_setreadonly.trampoline(L, LUA_GLOBALSINDEX, true);
}

}  // namespace spiral::environment
