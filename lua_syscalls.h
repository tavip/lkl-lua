#ifndef LUA_SYSCALLS_HH__
#define LUA_SYSCALLS_HH__

#include "lua.h"

#include <asm/lkl.h>

int lusys_change_dir (lua_State *L);
int lusys_get_dir (lua_State *L);
int lusys_mkdir (lua_State *L);
int lusys_rmdir (lua_State *L);
int lusys_utime (lua_State *L);
int lusys_utimes (lua_State *L);


#endif//LUA_SYSCALLS_HH__
