/*
    gamevalues.hpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#pragma once

// GAME VERSION: 0.10.2
// LUAU VERSION: AT LEAST 0.738

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
        find loadsafe from "%s: bytecode version mismatch (expected [%d..%d],
       got %d)" string, then just follow the RunContext stuff
    */
    .name = "luau_load",
    .offset = 0x92eb7d0};
typedef char *(*luau_compile_t)(const char *source, size_t size,
                                uint8_t *options, size_t *outsize);
inline GameValue<luau_compile_t> luau_compile{
    /*
        ":%d: %s" string -> Luau::compile
            (big function with std::string stuff and
            Luau::AstStat::Luau::AstStatBlock::VTable)
        Luau::compile's only callsite is luau_compile
    */
    .name = "luau_compile",
    .offset = 0x92ee710};
typedef void (*luau_execute_t)(uint8_t *L);
inline GameValue<luau_execute_t> luau_execute{
    /*
        find from coroutine.resume
        "too many arguments to resume" string -> lua_resume -> tailcall (which I
        think is supposed to be resume_continue) -> luau_execute
    */
    .name = "luau_execute",
    .offset = 0x92e5cb0,
    .hook = hooks::hooked_luau_execute};
typedef uint8_t *(*lua_mainthread_t)(uint8_t *L);
inline GameValue<lua_mainthread_t> lua_mainthread{
    /*
        48 8b 47 18 48 8b 80 f8
        otherwise just look above lua_absindex which is above lua_gettop
    */
    .name = "lua_mainthread",
    .offset = 0x92b7c30};
typedef uint8_t *(*lua_newthread_t)(uint8_t *L);
inline GameValue<lua_newthread_t> lua_newthread{
    /*
        coroutine.create ("isyieldable" string -> co_funcs -> cocreate)
    */
    .name = "lua_newthread",
    .offset = 0x92b9f80};
typedef void (*lua_close_t)(uint8_t *L);
inline GameValue<lua_close_t> lua_close{
    /*
        find luaM_free_ from luau_load
        then find close_state from three luaM_free_ calls in a row
            there are two calls to close_state:
            the bigger function is lua_newstate
            and the smaller is lua_close
    */
    .name = "lua_close",
    .offset = 0x92ccfa0,
    .hook = hooks::hooked_lua_close};
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
inline GameValue<luaL_tolstring_t> luaL_tolstring{/*
                                                      "%s: 0x%016llx" string
                                                  */
                                                  .name = "luaL_tolstring",
                                                  .offset = 0x92bd490};
typedef void (*lua_settop_t)(uint8_t *L, int idx);
inline GameValue<lua_settop_t> lua_settop{/*
                                              see lua_createtable
                                          */
                                          .name = "lua_settop",
                                          .offset = 0x92ba050};
typedef int (*lua_gettop_t)(uint8_t *L);
inline GameValue<lua_gettop_t> lua_gettop{
    /*
        above lua_settop
    */
    .name = "lua_gettop",

    .pattern = "f3 0f 1e fa 48 8b 47 08 48 2b 47 10 48 c1 f8 04",
    .has_pattern = true};

/* HOW TO FIND lua_call
    find luaL_callmeta from luaL_tolstring:
    void* luaL_tolstring(char* L, int32_t idx, uint64_t* len)

    {
        ...
        rax_2 = luaL_callmeta(L, idx, "__tostring");

        if (!rax_2)
        {
            ...

    find lua_call from luaL_callmeta:
    ...
        lua_pushvalue(arg1, rbp);
        lua_call(arg1, 1, 1);
        return 1;
    }
*/

typedef int (*lua_pcall_t)(uint8_t *L, int nargs, int nresults, int errfunc);
inline GameValue<lua_pcall_t> lua_pcall{
    /*
        the function lua_call calls above the arg3 == -1 check is 'func1'
        'func1' has a wrapper f_call:
        int64_t f_call(char* arg1, int64_t* arg2)
        {
            return sub_92bf530(arg1, *(uint64_t*)arg2, arg2[1]);
            //     ^^ func1
        }

        lua_pcall uses f_call like this:
        result = luaD_pcall(arg1, f_call, &var_48, rbx, rbp, arg1, r13, r14,
       r15);

        so outer function of above is lua_pcall
    */
    .name = "lua_pcall",
    .offset = 0x92bb0d0};
typedef void (*lua_pushvalue_t)(uint8_t *L, int idx);
inline GameValue<lua_pushvalue_t> lua_pushvalue{
    /*
        int64_t* luaL_findtable(char* arg1, int32_t arg2, int64_t* arg3, int32_t
       arg4) { int64_t* result = arg3; lua_pushvalue(arg1, arg2); char* rbx_1;
            ...
    */
    .name = "lua_pushvalue",
    .offset = 0x92ba130};
typedef void (*lua_pushcclosurek_t)(uint8_t *L, lua_CFunction fn,
                                    const char *debugname, int nup,
                                    lua_Continuation cont);
inline GameValue<lua_pushcclosurek_t> lua_pushcclosurek{/*
                                                            see lua_replace
                                                        */
                                                        .name = "lua_cclosurek",
                                                        .offset = 0x92bb240};
typedef void (*lua_pushlstring_t)(uint8_t *L, const char *s, size_t len);
inline GameValue<lua_pushlstring_t> lua_pushlstring{/*
                                                        see lua_createtable
                                                    */
                                                    .name = "lua_pushlstring",
                                                    .offset = 0x92ba620};
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
    .offset = 0x92bacc0};
typedef void (*lua_rawsetfield_t)(uint8_t *L, int idx, const char *k);
inline GameValue<lua_rawsetfield_t> lua_rawsetfield{
    /*
        below lua_setfield (idk how I found lua_setfield sorry)
    */
    .name = "lua_rawsetfield",
    .offset = 0x92b9380};
typedef void (*lua_rawseti_t)(uint8_t *L, int idx, int n);
inline GameValue<lua_rawseti_t> lua_rawseti{
    /*
        table.insert ("foreachi" string -> tab_funcs -> tinsert)

        tinsert has this at the end:
        lua_rawseti(L, idx: 1, n: rdx)
        return 0
    */
    .name = "lua_rawseti",
    .pattern =
        "f3 0f 1e fa 41 54 55 48 89 fd 53 48 83 ec 10 85 f6 7e 7d 48 8b 47 10 "
        "48 63",
    .has_pattern = true};
typedef int (*lua_rawget_t)(uint8_t *L, int idx);
inline GameValue<lua_rawget_t> lua_rawget{
    /*
        see lua_createtable
    */
    .name = "lua_rawget",
    .pattern =
        "f3 0f 1e fa 55 48 63 ee 53 48 89 fb 48 83 ec 08 f6 47 01 04 75 6a 85 "
        "ed 7e 46 48 8b 43 10 48 c1 e5 04 48 8b 73",
    .has_pattern = true};
typedef int (*lua_getfield_t)(uint8_t *L, int idx, const char *k);
inline GameValue<lua_getfield_t> lua_getfield{/*
                                                  see lua_createtable
                                              */
                                              .name = "lua_getfield",
                                              .offset = 0x92ba850};
typedef void (*lua_setreadonly_t)(uint8_t *L, int objindex, int enabled);
inline GameValue<lua_setreadonly_t> lua_setreadonly{
    /*
        find luaL_register(arg1, "vector", &vectorlib)
        this function is luaopen_vector

        which has this at the end:
        lua_setfield(arg1, 0xfffffffe, "__index");
        lua_setreadonly(arg1, 0xffffffff, 1);
        lua_settop(arg1, 0xfffffffe);
        return 1;
    */
    .name = "lua_setreadonly",
    .offset = 0x92b90d0};
typedef int (*lua_setmetatable_t)(uint8_t *L, int objindex);
inline GameValue<lua_setmetatable_t> lua_setmetatable{
    /*
        2 below lua_rawseti
        double check since it gets called after lua_rawsetfield(L, -2, "__mode")
       in places
    */
    .name = "lua_setmetatable",
    .offset = 0x92b9710};
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
                                                      just find setfenv
                                                  */
                                                  .name = "lua_setsafeenv",
                                                  .offset = 0x92b91b0};
typedef int (*lua_objlen_t)(uint8_t *L, int idx);
inline GameValue<lua_objlen_t> lua_objlen{/*
                                              table.foreachi
                                          */
                                          .name = "lua_objlen",
                                          .offset = 0x92b89c0};
typedef int (*lua_type_t)(uint8_t *L, int idx);
inline GameValue<lua_type_t> lua_type{/*
                                          see lua_createtable
                                      */
                                      .name = "lua_type",
                                      .offset = 0x92b7f60};

typedef char *(*luaA_toobject_t)(uint8_t *L, int idx);
inline GameValue<luaA_toobject_t> luaA_toobject{
    /* "invalid argument #%d to '%s' (%s expected, got %s)" string ->
       luaL_typerrorL:

        void luaL_typeerrorL(char* arg1, int32_t arg2)
        {
            char* rax = sub_6a1c800(arg1);
            void** rax_1 = luaA_toobject(arg1, arg2);
            //             ^^ luaA_toobject HERE

            ...

            sub_6a2eab0(arg1, rax_1);
            luaL_errorL(arg1,
                "invalid argument #%d to '%s' (%s expected, got %s)", 0);
            }
    */
    .name = "luaA_toobject",
    .offset = 0x92b7a90};

inline GameValue<lua_CFunction> luaB_print{
    /*
        just find base_funcs by searching for "print", "newproxy", etc:

        char const (* data_6dc1350)[0x6] = data_a8b034 {"print"}
        void* data_6dc1358 = luaB_print
        //                   ^^ luaB_print HERE
    */
    .name = "luaB_print",
    .pattern =
        "f3 0f 1e fa 41 57 41 56 41 55 41 54 55 48 89 fd 53 48 83 ec 28 64 48 "
        "8b 04 25 28 00 00 00 48 89 44 24 18 31 c0",
    .has_pattern = true};

typedef void *(*Vortex_newstate_t)(void);
inline GameValue<Vortex_newstate_t> Vortex_newstate{
    /*
        calls lua_newstate
    */
    .name = "Vortex_newstate",
    .offset = 0x6f833a0,
    .hook = hooks::hooked_Vortex_newstate};

inline auto game_values = std::tie(
    luau_load, luau_compile, luau_execute, lua_mainthread, lua_newthread,
    lua_close, lua_tolstring, luaL_tolstring, lua_settop, lua_gettop, lua_pcall,
    lua_pushvalue, lua_pushcclosurek, lua_pushlstring, lua_createtable,
    lua_rawsetfield, lua_rawseti, lua_rawget, lua_getfield, lua_setreadonly,
    lua_setmetatable, lua_replace, lua_setsafeenv, lua_objlen, lua_type,
    luaA_toobject, luaB_print, Vortex_newstate);

bool setup();

}  // namespace spiral::gamevalues
