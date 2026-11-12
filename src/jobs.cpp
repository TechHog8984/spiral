/*
    jobs.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#include "jobs.hpp"

#include <sys/stat.h>
#include <unistd.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>

namespace spiral::jobs {

std::queue<Job> job_queue;
std::mutex job_queue_mutex;

void queueJob(const Job &job) {
    std::lock_guard<std::mutex> lock(job_queue_mutex);
    job_queue.push(job);
}
void queueScript(const std::string &script) {
    Job job{.tag = Job::ExecuteScript, .value = script};
    queueJob(job);
}

void swapJobQueue(std::queue<Job> &local_queue) {
    std::lock_guard<std::mutex> lock(job_queue_mutex);
    local_queue.swap(job_queue);
}

constexpr const char *pipe_path = "/tmp/spiral/pipe";
static std::atomic_bool pipe_running = false;

void startPipe() {
    if (std::filesystem::exists(pipe_path)) std::filesystem::remove(pipe_path);

    mkfifo(pipe_path, 0666);

    pipe_running = true;
    std::thread([&]() {
        while (pipe_running) {
            std::ifstream stream(pipe_path);
            if (stream.is_open()) {
                std::string script((std::istreambuf_iterator<char>(stream)),
                                   std::istreambuf_iterator<char>());
                queueScript(script);
                stream.close();
            }
        }
    }).detach();
}
void stopPipe() {
    pipe_running = false;
    unlink(pipe_path);
}

}  // namespace spiral::jobs
