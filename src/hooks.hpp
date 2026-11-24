/*
    hooks.hpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#pragma once

#include <cstddef>
#include <cstdint>

namespace spiral::hooks {

void hooked_luau_execute(uint8_t *L);
void hooked_lua_close(uint8_t *L);
void *hooked_Vortex_newstate(void);
int32_t hooked_luau_load(uint8_t *L, const char *chunkname, const char *data,
                         size_t size, int32_t env);

}  // namespace spiral::hooks
