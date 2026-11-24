/*
    hooks.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#include <cstdint>

#include "execution.hpp"
#include "gamevalues.hpp"
#include "jobs.hpp"
#include "lua.hpp"

namespace spiral::hooks {

bool newstate_called = false;
bool done_manual_init = false;

void hooked_luau_execute(uint8_t *L) {
    /*
        if luau_execute has been called BEFORE Vortex_newstate, then we injected
        into the game after a state has already been created,
        so we need to queue an initialiation job manually.
    */
    if (!newstate_called && !done_manual_init) {
        puts("[spiral] doing manual init...");
        done_manual_init = true;
        jobs::queueInitializeState();
    }

    static bool anti_recursion = false;

    if (!anti_recursion) {
        anti_recursion = true;

        std::queue<jobs::Job> job_queue;
        jobs::swapJobQueue(job_queue);

        while (!job_queue.empty()) {
            auto &job = job_queue.front();

            switch (job.tag) {
                case jobs::Job::InitializeState: {
                    gamevalues::lua_getfield.trampoline(L, LUA_GLOBALSINDEX,
                                                        "game");
                    int game_type = gamevalues::lua_type.trampoline(L, -1);
                    gamevalues::lua_settop.trampoline(L, -2);

                    // wait for game to be set
                    if (game_type != LUA_TUSERDATA) {
                        auto &attempts = std::get<int>(job.value);
                        if (++attempts > 5000)
                            puts(
                                "[spiral] Did you inject into the Vortex after a game launch? Spiral doesn't support that, so please rejoin your game.");
                        else
                            jobs::queueJob(job);
                    } else
                        execution::initializeState(
                            gamevalues::lua_mainthread.trampoline(L));

                    break;
                }
                case jobs::Job::ExecuteScript:
                    if (execution::game_state)
                        execution::runCode(std::get<std::string>(job.value));
                    else
                        // TODO: is this slow af since job.value holds a
                        // std::string?
                        jobs::queueJob(job);
                    break;
            }

            job_queue.pop();
        }

        anti_recursion = false;
    }

    gamevalues::luau_execute.trampoline(L);
}

void hooked_lua_close(uint8_t *L) {
    uint8_t *main = gamevalues::lua_mainthread.trampoline(L);
    if (execution::game_state && main == execution::game_state)
        execution::onGameStateClose();

    gamevalues::lua_close.trampoline(L);
}

void *hooked_Vortex_newstate(void) {
    newstate_called = true;

    auto result = gamevalues::Vortex_newstate.trampoline();

    jobs::queueInitializeState();

    return result;
}

}  // namespace spiral::hooks
