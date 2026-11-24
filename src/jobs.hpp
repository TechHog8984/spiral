/*
    jobs.hpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#pragma once

#include <queue>
#include <string>
#include <variant>

namespace spiral::jobs {

struct Job {
    enum { InitializeState, ExecuteScript, LuauLoadCapture } tag;

    std::variant<std::monostate, int, std::string> value;
};

void queueJob(const Job &job);

void queueScript(const std::string &script);
void queueInitializeState();

void swapJobQueue(std::queue<Job> &local_queue);

void startPipe();
void stopPipe();

}  // namespace spiral::jobs
