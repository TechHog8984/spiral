/*
    execution.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#include "execution.hpp"

#include <cstdint>

#include "environment.hpp"
#include "gamevalues.hpp"
#include "lua.hpp"

namespace spiral::execution {

uint8_t *game_state = nullptr;
uint8_t *spiral_state = nullptr;

void initializeState(uint8_t *game_state_in) {
    // don't initialize if there's an active game state still
    if (game_state) return;

    puts("[spiral::execution.initializeState] initializing...");

    game_state = game_state_in;

    spiral_state = gamevalues::lua_newthread.trampoline(game_state);
    lua::lua_ref(game_state, -1);
    gamevalues::lua_settop.trampoline(game_state, -2);

    printf(
        "[spiral::execution.initializeState] main state: %p, spiral state: %p\n",
        game_state, spiral_state);

    // luaL_sandboxthread
    {
        auto L = spiral_state;
        gamevalues::lua_createtable.trampoline(L, 0, 0);
        gamevalues::lua_createtable.trampoline(L, 0, 0);
        gamevalues::lua_pushvalue.trampoline(L, LUA_GLOBALSINDEX);
        gamevalues::lua_rawsetfield.trampoline(L, -2, "__index");
        gamevalues::lua_setreadonly.trampoline(L, -1, true);
        gamevalues::lua_setmetatable.trampoline(L, -2);
        gamevalues::lua_replace.trampoline(L, LUA_GLOBALSINDEX);
        // gamevalues::lua_setsafeenv.trampoline(L, LUA_GLOBALSINDEX, 1);
    }

    environment::apply(spiral_state);

    puts("[spiral::execution.initializeState] initialization finished.");
}

void onGameStateClose() {
    // TODO: cleanup old state
    // there isn't anything to do rn because all we'd do is unref but the
    // state is (if Vortex is doing things right which I imagine they are)
    // being closed anyway
    game_state = nullptr;
}

void runCode(const std::string &code) {
    size_t bytecode_size;
    char *bytecode = gamevalues::luau_compile.trampoline(
        code.c_str(), code.size(), NULL, &bytecode_size);
    if (bytecode == 0) {
        puts("[spiral::execution.runCode] failed to allocate bytecode.");
        return;
    }

    uint8_t *L = gamevalues::lua_newthread.trampoline(spiral_state);
    gamevalues::lua_settop.trampoline(spiral_state, -2);

    int result = gamevalues::luau_load.trampoline(L, "@spiral", bytecode,
                                                  bytecode_size, 0);
    free(bytecode);

    if (result != 0) {
        const char *error = gamevalues::lua_tolstring.trampoline(L, -1, NULL);
        printf("[spiral::execution.runCode] compile/load error: %s\n", error);
        gamevalues::lua_settop.trampoline(L, -2);
        return;
    }

    if (gamevalues::lua_pcall.trampoline(L, 0, 0, 0) != 0) {
        const char *error = gamevalues::lua_tolstring.trampoline(L, -1, NULL);
        printf("[spiral::execution.runCode] runtime error: %s\n", error);
        gamevalues::lua_settop.trampoline(L, -2);
        return;
    }
}

}  // namespace spiral::execution
