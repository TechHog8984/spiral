/*
    gamevalues.hpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

#include "hooks.hpp"
#include "lua.hpp"
#include "polyhook2/Detour/NatDetour.hpp"

namespace spiral::gamevalues {

template <typename T>
struct GameValue {
    const char *name = nullptr;

    union {
        uintptr_t offset;
        const char *pattern;
    };
    bool has_pattern;

    T value = nullptr;

    // function-specific
    T hook = nullptr;
    T trampoline = nullptr;
    std::unique_ptr<PLH::NatDetour> detour;
};

typedef int32_t (*luau_load_t)(uint8_t *L, const char *chunkname,
                               const char *data, size_t size, int32_t env);
inline GameValue<luau_load_t> luau_load{
    /*
        find from "%s: bytecode version mismatch (expected [%d..%d], got %d)"
        string
    */
    .name = "luau_load",
    .pattern =
        "f3 0f 1e fa 41 57 49 89 ff 41 56 41 "
        "55 41 54 49 89 f4 55 53 48 89 d3 48 "
        "81 ec a8 01 00 00 64 48",
    .has_pattern = true};
typedef char *(*luau_compile_t)(const char *source, size_t size,
                                uint8_t *options, size_t *outsize);
inline GameValue<luau_compile_t> luau_compile{
    /*
    label:
        luaL_loadbufferenv has code like this:
        char* rax_3 = luau_compile(data, size, nullptr, &size_1);
        *(uint64_t*)rax_2 = rax_3;
        int32_t rax_4 = luau_load(L, name, rax_3, (int32_t)size_1, (env >> 0x1f)
    + env);
    */
    .name = "luau_compile",
    .pattern =
        "f3 0f 1e fa 41 56 66 0f ef c0 41 55 41 54 49 89 f4 55 48 89 fd 53 48 "
        "89 cb 48 81 ec 20 01 00 00 64 48 8b 04 25 28 00 00 00 48 89 84 24 18 ",
    .has_pattern = true};
typedef void (*luau_execute_t)(uint8_t *L);
inline GameValue<luau_execute_t> luau_execute{
    /*
        find from coroutine.resume
        "too many arguments to resume" string -> lua_resume -> tailcall (which I
       think is supposed to be resume_continue) -> luau_execute
    */
    .name = "luau_execute",
    .pattern =
        "41 57 41 56 41 55 41 54 49 89 fc 55 53 48 81 ec 98 00 00 00 64 48 8b "
        "04 25 28 00 00 00 48 89 84 24 88 00 00 00",
    .has_pattern = true,
    .hook = hooks::hooked_luau_execute};
typedef uint8_t *(*lua_mainthread_t)(uint8_t *L);
inline GameValue<lua_mainthread_t> lua_mainthread{
    /*
        lua_mainthread pattern (NOT START OF FUNCTION):
            48 8b 47 18 48 8b 80 f8 0a 00
        (v0.9.6)
    */
    .name = "lua_mainthread",
    .pattern =
        "f3 0f 1e fa 8d 96 0f 4a 0f 00 89 f0 81 fa 0f 4a 0f 00 ?? ?? 48 8b 57 "
        "08 48 2b 57 10 48 c1 fa 04",
    // 77 10 (v0.9.6)
    .has_pattern = true};
typedef uint8_t *(*lua_newthread_t)(uint8_t *L);
inline GameValue<lua_newthread_t> lua_newthread{
    /*
        above lua_mainthread
    */
    .name = "lua_newthread",
    .pattern =
        "f3 0f 1e fa 41 54 55 48 89 fd 48 83 ec 08 48 8b 47 18 48 8b 48 40 48 "
        "39 48 48 ?? ?? f6 45 01 04",
    // 73 54 (v0.9.6)
    .has_pattern = true};
typedef const char *(*lua_tolstring_t)(uint8_t *L, int idx, size_t *len);
inline GameValue<lua_tolstring_t> lua_tolstring{
    /* find lua_exception::what from "lua_exception: not enough memory" string,
       then look at first call:

       char const* const lua_exception::what(void* arg1)
        {
            int32_t rdx = *(uint32_t*)((char*)arg1 + 0x10);

            if (rdx != 2)
            goto label_6a1eb5d;

            char const* const result = lua_tolstring(*(uint64_t*)((char*)arg1 +
       8), 0xffffffff, nullptr);

            if (!result)
            {
                rdx = *(uint32_t*)((char*)arg1 + 0x10);
                label_6a1eb5d:
                result = "lua_exception: not enough memory";
    */
    .name = "lua_tolstring",
    .pattern =
        "f3 0f 1e fa 41 55 41 54 4c 63 e6 55 48 89 fd 53 48 89 d3 48 83 ec 08 "
        "45 85",
    .has_pattern = true};
typedef const char *(*luaL_tolstring_t)(uint8_t *L, int idx, size_t *len);
inline GameValue<luaL_tolstring_t> luaL_tolstring{
    /*
        "%s: 0x%016llx" string
    */
    .name = "luaL_tolstring",
    .pattern =
        "f3 0f 1e fa 41 56 41 55 49 89 d5 48 8d 15 f4 ae 07 fa 41 54 41 89 f4",
    .has_pattern = true};
typedef void (*lua_settop_t)(uint8_t *L, int idx);
inline GameValue<lua_settop_t> lua_settop{
    /*
        see lua_createtable
    */
    .name = "lua_settop",
    .pattern =
        "f3 0f 1e fa 48 8b 4f 08 85 f6 ?? ?? 48 63 d6 48 c1 e2 04 48 03 57 10 "
        "48 39 d1 ?? ?? 48 89",
    // 78 44 (v0.9.6)
    // 73 2a (v0.9.6)
    .has_pattern = true};
typedef int (*lua_gettop_t)(uint8_t *L);
inline GameValue<lua_gettop_t> lua_gettop{
    /*
        above lua_settop
    */
    .name = "lua_gettop",

    .pattern = "f3 0f 1e fa 48 8b 47 08 48 2b 47 10 48 c1 f8 04",
    .has_pattern = true};
typedef int (*lua_pcall_t)(uint8_t *L, int nargs, int nresults, int errfunc);
inline GameValue<lua_pcall_t> lua_pcall{
    /*
        after lua_call

        find lua_call with "_LOADED" string:
            if (sub_6a3de80(arg1, rax_1) != 6)
                lua_settop(arg1, 0xfffffffe)
                sub_6a3e180(arg1, 0, 0)
                sub_6a3db10(arg1, "_LOADED")
                lua_pushvalue(arg1, 0xfffffffe)
                sub_6a3e520(arg1, rax_1)

            if (lua_getfield(arg1, 0xffffffff, arg2) == 0)
                lua_settop(arg1, 0xfffffffe)
                lua_pushcclosurek(arg1, arg3, 0, 0, 0)
                sub_6a3db10(arg1, arg2)
                lua_tolstring(arg1, 0xffffffff, nullptr)
                // lua_call BELOW
                lua_call(arg1, 1, 1) // RIGHT HERE
                // ^ lua_call HERE
                lua_pushvalue(arg1, 0xffffffff)
                lua_setfield(arg1, 0xfffffffd, arg2)
    */
    .name = "lua_pcall",
    .pattern =
        "f3 0f 1e fa 55 4c 63 ce 48 89 fd 53 89 d3 48 83 ec 28 64 48 8b 04 25 "
        "28 00 00 00 48 89 44 24 18 31 c0 85 c9 ?? ?? 48 8b 57 30 48 8b 47 08",
    // 75 52 (v0.9.6)
    .has_pattern = true};
typedef void (*lua_pushvalue_t)(uint8_t *L, int idx);
inline GameValue<lua_pushvalue_t> lua_pushvalue{
    /*
        see lua_pcall
        OR
        int64_t* luaL_findtable(char* arg1, int32_t arg2, int64_t* arg3, int32_t
       arg4) { int64_t* result = arg3; lua_pushvalue(arg1, arg2); char* rbx_1;
            ...
    */
    .name = "lua_pushvalue",
    .pattern =
        "f3 0f 1e fa 55 48 63 ee 53 48 89 fb 48 83 ec 08 f6 47 01 04 75 5a 85 "
        "ed 7e 36 48 8b 43 10 48",
    .has_pattern = true};
typedef void (*lua_pushcclosurek_t)(uint8_t *L, lua_CFunction fn,
                                    const char *debugname, int nup,
                                    lua_Continuation cont);
inline GameValue<lua_pushcclosurek_t> lua_pushcclosurek{
    /*
        see lua_replace
    */
    .name = "lua_cclosurek",
    .pattern =
        "f3 0f 1e fa 41 56 49 89 f6 41 55 4d 89 c5 41 54 49 89 d4 55 48 89 fd "
        "53 48 8b 47 18 89 cb 48 8b 78 40 48 39 78 48 0f 83 94 00 00 00 f6 45",
    .has_pattern = true};
typedef void (*lua_createtable_t)(uint8_t *L, int narray, int nrec);
inline GameValue<lua_createtable_t> lua_createtable{
    /*
        find luaL_findtable

        inside luaL_register:
        luaL_findtable(arg1, 0xfff0b5f0, "_LOADED", 1);
        lua_getfield(arg1, 0xffffffff, arg2);

        if (lua_type(arg1, 0xffffffff) != 6)
            ...

        in luaL_findtable:
        ...
        uint64_t r13_2 = rbx_1 - result;
        lua_pushlstring(arg1, result, r13_2);
        lua_rawget(arg1, 0xfffffffe);

        if (!lua_type(arg1, 0xffffffff))
        {
            lua_settop(arg1, 0xfffffffe);
            int32_t rdx = 1;

            if (*(uint8_t*)rbx_1 != 0x2e)
                rdx = arg4;

            lua_createtable(arg1, 0, rdx);
            lua_pushlstring(arg1, result, r13_2);
            lua_pushvalue(arg1, 0xfffffffe);
            lua_settable(arg1, 0xfffffffc);
        ...
    */
    .name = "lua_createtable",
    .pattern =
        "f3 0f 1e fa 41 55 41 54 41 89 d4 55 89 f5 53 48 89 fb 48 83 ec 08 48 "
        "8b 47 18 48 8b 48 40 48 39 48 48 ?? ?? f6 43 01 04",
    // 73 3c (v0.9.6)
    .has_pattern = true};
typedef void (*lua_rawsetfield_t)(uint8_t *L, int idx, const char *k);
inline GameValue<lua_rawsetfield_t> lua_rawsetfield{
    /*
        see lua_createtable
    */
    .name = "lua_rawsetfield",
    .pattern =
        "f3 0f 1e fa 41 54 49 89 d4 55 53 48 89 fb 48 83 ec 20 64 48 8b 04 25 "
        "28 00 00 00 48 89 44 24 18 31 c0 85 f6",
    .has_pattern = true};
typedef void (*lua_rawseti_t)(uint8_t *L, int idx, int n);
inline GameValue<lua_rawseti_t> lua_rawseti{
    /*
        idk
    */
    .name = "lua_rawseti",
    .pattern =
        "f3 0f 1e fa 41 54 55 48 89 fd 53 48 83 ec 10 85 f6 7e 7d 48 8b 47 10 "
        "48 63",
    .has_pattern = true};
typedef void (*lua_setreadonly_t)(uint8_t *L, int objindex, int enabled);
inline GameValue<lua_setreadonly_t> lua_setreadonly{
    /*
        I think I found setsafeenv by just scrolling up from lua_rawsetfield
        which leads you to 'luaL_sandbox' (probably not it since it looks like
       mlua) which calls lua_setreadonly a lot
    */
    .name = "lua_setreadonly",
    .pattern =
        "f3 0f 1e fa 41 89 d0 85 f6 7e 2d 48 8b 47 10 48 63 f6 48 c1 e6 04 48 "
        "8d 44 30 f0 48 3b 47",
    .has_pattern = true};
typedef int (*lua_setmetatable_t)(uint8_t *L, int objindex);
inline GameValue<lua_setmetatable_t> lua_setmetatable{
    /*
        below lua_rawseti
    */
    .name = "lua_setmetatable",
    .pattern =
        "f3 0f 1e fa 55 48 89 fd 85 f6 0f 8e a0 00 00 00 48 8b 47 10 48 63 f6 "
        "48 8b 57 08 48 8d 0d 4e ae 2a fb 48 c1 e6 04 48 8d 44 30 f0 48 39",
    .has_pattern = true};
typedef void (*lua_replace_t)(uint8_t *L, int idx);
inline GameValue<lua_replace_t> lua_replace{
    /*
        gmatch_aux

        find gmatch from 'gmatch' string in string table function lookup

        int64_t gmatch(char* arg1) {
            luaL_checklstring(arg1, 1, nullptr);
            luaL_checklstring(arg1, 2, nullptr);
            lua_settop(arg1, 2);
            lua_pushinteger(arg1, 0);
            lua_pushcclosurek(arg1, gmatch_aux, 0, 3, 0);
            return 1;
        }

        gmatch_aux has this:
        while (true)
        {
            int32_t var_250_1 = 0;
            char* rax_9 = sub_6a301a0(&var_278, r12_1, rax_3);

            if (rax_9)
            {
                int32_t rsi_2 = 0;
                (uint8_t)rsi_2 = rax_9 == r12_1;
                lua_pushinteger(arg1, rsi_2 + (uint32_t)rax_9 -
       (uint32_t)rax_2);
                // lua_replace BELOW
                lua_replace(arg1, 0xfff0b5eb);
                // ^ lua_replace HERE
                r14_1 = var_250_1;
        ...
    */
    .name = "lua_replace",
    .pattern =
        "f3 0f 1e fa 55 48 63 ee 53 48 89 fb 48 83 ec 08 f6 47 01 04 0f 85 e6 "
        "00 00 00 85 ed",
    .has_pattern = true};
typedef void (*lua_setsafeenv_t)(uint8_t *L, int objindex, int enabled);
inline GameValue<lua_setsafeenv_t> lua_setsafeenv{/*
                                                      below lua_createtable
                                                  */
                                                  .name = "lua_setsafeenv",
                                                  .offset = 0x6a3e2e0};
typedef int (*lua_objlen_t)(uint8_t *L, int idx);
inline GameValue<lua_objlen_t> lua_objlen{
    /*
        idk
    */
    .name = "lua_objlen",
    .pattern =
        "f3 0f 1e fa 85 f6 7e 58 48 8b 47 10 48 63 f6 48 c1 e6 04 48 8d 44 30 "
        "f0 48 3b 47 08 72 07 48 8d",
    .has_pattern = true};

inline auto game_values =
    std::tie(luau_load, luau_compile, luau_execute, lua_mainthread,
             lua_newthread, lua_tolstring, luaL_tolstring, lua_settop,
             lua_gettop, lua_pcall, lua_pushvalue, lua_pushcclosurek,
             lua_createtable, lua_rawsetfield, lua_rawseti, lua_setreadonly,
             lua_setmetatable, lua_replace, lua_setsafeenv, lua_objlen);

bool setup();

}  // namespace spiral::gamevalues
