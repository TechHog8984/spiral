/*
    entry.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#include <cstdio>
#include <thread>

#include "application.hpp"
#include "gamevalues.hpp"
#include "jobs.hpp"
#include "logging.hpp"

namespace spiral {

void init() {
    puts("[spiral] initialization started.");

    // FIXME: handle leaving and joining a new game (don't forget to clean old
    // state)
    {
        jobs::Job job{.tag = jobs::Job::InitializeState};
        jobs::queueJob(job);
    }

    {
        std::string script =
            "print('it this thing on?', type(game), if game then "
            "type(game.Players) else 'nil')";
        // std::string script =
        //     "game.Players.LocalPlayer.Character.WalkSpeed = 40";
        jobs::queueScript(script);
    }

    std::filesystem::create_directory("/tmp/spiral");

    jobs::startPipe();

    puts("[spiral] reading application...");
    if (!application::readApplication()) return;

    puts("[spiral] reading and hooking game values...");
    if (!gamevalues::setup()) return;

    puts("[spiral] initialization finished.");
}

__attribute((constructor)) void __entry() { std::thread(init).detach(); }

__attribute((destructor)) void __cleanup() {
    puts("[spiral] cleaning up...");

    std::apply(
        [](auto &...items) {
            (
                [&]() {
                    if (items.detour) items.detour.reset();
                }(),
                ...);
        },
        gamevalues::game_values);

    jobs::stopPipe();

    logging::cleanup();

    puts("[spiral] clean up finished.");
}

}  // namespace spiral
