/*
 * lualib.h - useful functions and type for Lua
 *
 * Copyright © 2009 Julien Danjou <julien@danjou.info>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef AWESOME_COMMON_LUALIB
#define AWESOME_COMMON_LUALIB

#include <lauxlib.h>
#include "common/util.h"

#define luaA_checkfunction(L, n) \
    do { \
        if(!lua_isfunction(L, n)) \
            luaL_typerror(L, n, "function"); \
    } while(0)

/** Dump the Lua stack. Useful for debugging.
 * \param L The Lua VM state.
 */
static inline void
luaA_dumpstack(lua_State *L)
{
    fprintf(stderr, "-------- Lua stack dump ---------\n");
    for(int i = lua_gettop(L); i; i--)
    {
        int t = lua_type(L, i);
        switch (t)
        {
          case LUA_TSTRING:
            fprintf(stderr, "%d: string: `%s'\n", i, lua_tostring(L, i));
            break;
          case LUA_TBOOLEAN:
            fprintf(stderr, "%d: bool:   %s\n", i, lua_toboolean(L, i) ? "true" : "false");
            break;
          case LUA_TNUMBER:
            fprintf(stderr, "%d: number: %g\n", i, lua_tonumber(L, i));
            break;
          case LUA_TNIL:
            fprintf(stderr, "%d: nil\n", i);
            break;
          default:
            fprintf(stderr, "%d: %s\t#%d\t%p\n", i, lua_typename(L, t),
                    (int) lua_objlen(L, i),
                    lua_topointer(L, i));
            break;
        }
    }
    fprintf(stderr, "------- Lua stack dump end ------\n");
}

/** Convert s stack index to positive.
 * \param L The Lua VM state.
 * \param ud The index.
 * \return A positive index.
 */
static inline int
luaA_absindex(lua_State *L, int ud)
{
    return (ud > 0 || ud <= LUA_REGISTRYINDEX) ? ud : lua_gettop(L) + ud + 1;
}

static inline int
luaA_dofunction_error(lua_State *L)
{
    if(!luaL_dostring(L, "return debug.traceback(\"error while running function\", 3)"))
    {
        /* Move traceback before error */
        lua_insert(L, -2);
        /* Insert sentence */
        lua_pushliteral(L, "\nerror: ");
        /* Move it before error */
        lua_insert(L, -2);
        lua_concat(L, 3);
    }
    return 1;
}

/** Execute an Lua function on top of the stack.
 * \param L The Lua stack.
 * \param nargs The number of arguments for the Lua function.
 * \param nret The number of returned value from the Lua function.
 * \return True on no error, false otherwise.
 */
static inline bool
luaA_dofunction(lua_State *L, int nargs, int nret)
{
    /* Move function before arguments */
    lua_insert(L, - nargs - 1);
    /* Push error handling function */
    lua_pushcfunction(L, luaA_dofunction_error);
    /* Move error handling function before args and function */
    lua_insert(L, - nargs - 2);
    if(lua_pcall(L, nargs, nret, - nargs - 2))
    {
        warn("%s", lua_tostring(L, -1));
        /* Remove error function and error string */
        lua_pop(L, 2);
        return false;
    }
    /* Remove error function */
    lua_remove(L, - nret - 1);
    return true;
}

/** Convert a object to a udata if possible.
 * \param L The Lua VM state.
 * \param ud The index.
 * \param tname The type name.
 * \return A pointer to the object, NULL otherwise.
 */
static inline void *
luaA_toudata(lua_State *L, int ud, const char *tname)
{
    void *p = lua_touserdata(L, ud);
    if(p) /* value is a userdata? */
        if(lua_getmetatable(L, ud)) /* does it have a metatable? */
        {
            lua_getfield(L, LUA_REGISTRYINDEX, tname); /* get correct metatable */
            if(!lua_rawequal(L, -1, -2)) /* does it have the correct mt? */
                p = NULL;
            lua_pop(L, 2); /* remove both metatables */
        }
    return p;
}

#endif

// vim: filetype=c:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=80