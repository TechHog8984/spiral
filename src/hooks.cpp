/*
    hooks.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#include "execution.hpp"
#include "gamevalues.hpp"
#include "jobs.hpp"
#include "lua.hpp"

namespace spiral::hooks {

void hooked_luau_execute(uint8_t *L) {
    static bool anti_recursion = false;
    if (!anti_recursion) {
        anti_recursion = true;

        std::queue<jobs::Job> job_queue;
        jobs::swapJobQueue(job_queue);

        while (!job_queue.empty()) {
            auto &job = job_queue.front();

            switch (job.tag) {
                case jobs::Job::InitializeState: {
                    lua::lua_pushstring(L, "game");
                    gamevalues::lua_rawget.trampoline(L, LUA_GLOBALSINDEX);
                    int game_type = gamevalues::lua_type.trampoline(L, -1);
                    gamevalues::lua_settop.trampoline(L, -2);

                    /*
                        wait for game to be set
                        TODO: is game set from a lua script? are we still
                         executing that same script?
                    */
                    if (game_type != LUA_TTABLE)
                        jobs::queueJob(job);
                    else
                        execution::initializeState(L);
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

}  // namespace spiral::hooks
