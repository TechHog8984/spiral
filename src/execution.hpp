/*
    execution.hpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#pragma once

#include <cstdint>
#include <string>

namespace spiral::execution {

extern uint8_t *game_state;
extern uint8_t *spiral_state;

void initializeState(uint8_t *game_state_in);
void runCode(const std::string &code);

}  // namespace spiral::execution
