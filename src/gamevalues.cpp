/*
    gamevalues.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#include "gamevalues.hpp"

#include "application.hpp"

namespace spiral::gamevalues {

bool setup() {
    int pattern_fails = 0;
    int hook_fails = 0;

    // first pass to find values & setup detours
    puts("[spiral] first pass...");
    std::apply(
        [&pattern_fails, &hook_fails](auto &...items) {
            (
                [&]() {
                    printf("[spiral]   finding %s...\n", items.name);

                    uintptr_t value = 0;
                    if (items.has_pattern) {
                        value = PLH::findPattern(application::text_section,
                                                 application::text_section_size,
                                                 items.pattern);
                        if (value == 0) {
                            puts("[spiral]    > failed to match pattern.");
                            pattern_fails++;
                            return;
                        }
                    } else
                        value = application::base + items.offset;

                    printf("[spiral]    > found value at 0x%lx (0x%lx)\n",
                           value, value - application::base);

                    using T = std::decay_t<decltype(items.value)>;

                    items.value = reinterpret_cast<T>(value);

                    if (items.hook) {
                        items.detour = std::make_unique<PLH::NatDetour>(
                            value, reinterpret_cast<uintptr_t>(items.hook),
                            reinterpret_cast<uint64_t *>(&items.trampoline));
                    } else {
                        // TODO: maybe rename trampoline?
                        items.trampoline = items.value;
                    }
                }(),
                ...);
        },
        gamevalues::game_values);
    puts("[spiral] first pass finished.");

    // second pass to hook functions
    puts("[spiral] second pass...");
    std::apply(
        [&pattern_fails, &hook_fails](auto &...items) {
            (
                [&]() {
                    if (items.hook) {
                        if (!items.value) {
                            printf("[spiral]   skipping %s since we failed to find it.\n", items.name);
                            return;
                        }

                        printf("[spiral]   hooking %s...\n", items.name);
                        if (!items.detour->hook()) {
                            puts("[spiral]    > failed to hook.");
                            hook_fails++;
                        } else
                            puts("[spiral]    > successfully hooked.");
                    }
                }(),
                ...);
        },
        gamevalues::game_values);
    puts("[spiral] second pass finished.");

    if (pattern_fails != 0 || hook_fails != 0) {
        printf(
            "[spiral] %d patterns had no match and %d functions failed to hook. quitting...\n",
            pattern_fails, hook_fails);
        return false;
    }

    return true;
}

}  // namespace spiral::gamevalues
