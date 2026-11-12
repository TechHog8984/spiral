/*
    hooks.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#include "execution.hpp"
#include "gamevalues.hpp"
#include "jobs.hpp"

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
                case jobs::Job::InitializeState:
                    execution::initializeState(L);
                    break;
                case jobs::Job::ExecuteScript:
                    execution::runCode(std::get<std::string>(job.value));
                    break;
            }

            job_queue.pop();
        }

        anti_recursion = false;
    }

    gamevalues::luau_execute.trampoline(L);
}

}  // namespace spiral::hooks
