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

void cleanup();

void init() {
    puts("[spiral] initialization started.");

    {
        std::string script =
            "print('it this thing on?', 'type(game)=' .. type(game), 'type(game.Players)=' .. (if game then type(game.Players) else 'nil'))";
        // std::string script =
        //     "game.Players.LocalPlayer.Character.WalkSpeed = 40";
        jobs::queueScript(script);
    }

    std::filesystem::create_directory("/tmp/spiral");

    jobs::startPipe();

    puts("[spiral] reading application...");
    if (!application::readApplication()) goto CLEANUP;

    puts("[spiral] reading and hooking game values...");
    if (!gamevalues::setup()) goto CLEANUP;

    puts("[spiral] initialization finished.");

    return;

CLEANUP:
    cleanup();
}

__attribute((constructor)) void __entry() { std::thread(init).detach(); }

void cleanup() {
    static bool cleanup_ran = false;
    if (cleanup_ran) return;
    cleanup_ran = true;

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

__attribute((destructor)) void __cleanup() { cleanup(); }

}  // namespace spiral
