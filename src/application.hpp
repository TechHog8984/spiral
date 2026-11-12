/*
    application.hpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#pragma once

#include <cstdint>

namespace spiral::application {

inline uintptr_t base = 0;
inline char path[512];
inline uintptr_t text_section = 0;
inline uintptr_t text_section_size = 0;

bool readApplication();

}  // namespace spiral::application
