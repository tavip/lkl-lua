#include "lua_syscalls.h"
#include "syscalls.h"
#include "utils.h"
#include "lualib.h"
#include "lauxlib.h"
#include "wapr_user.h"

#include <sys/types.h>
#include <sys/stat.h>


int lusys_change_dir (lua_State *L)
{
        apr_status_t rc;
	const char *path = luaL_checkstring(L, 1);
        rc = wrapper_sys_chdir(path);
	if (0 != rc)
        {
		lua_pushnil (L);
		lua_pushfstring (L,"Unable to change working directory to '%s'\n%s\n",
				path, lfd_apr_strerror_thunsafe(rc));
		return 2;
	}
        else 
        {
		lua_pushboolean (L, 1);
		return 1;
	}
}


/*
** This function returns the current directory
** If unable to get the current directory, it returns nil
**  and a string describing the error
*/
int lusys_get_dir (lua_State *L)
{
	char path[255+2];
        apr_status_t rc;

        rc = wrapper_sys_getcwd(path, 255);
        printf("wrapper_sys_getcwd ret [%d], [%s]\n", rc, path);
        if (rc <= 0)
        {
		lua_pushnil(L);
		lua_pushfstring(L, "getcwd failed because [%s]\n", lfd_apr_strerror_thunsafe(-rc));
		return 2;
	}
	else
        {
		lua_pushstring(L, path);
		return 1;
	}
}

int lusys_mkdir (lua_State *L)
{
	const char *path = luaL_checkstring (L, 1);
	int fail;
	mode_t oldmask = wrapper_sys_umask( (mode_t)0 );
	fail =  wrapper_sys_mkdir (path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
	                     S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH );

	if (fail) {
		lua_pushnil (L);
                lua_pushfstring (L, "%s", lfd_apr_strerror_thunsafe(fail));
		return 2;
	}
	wrapper_sys_umask (oldmask);
	lua_pushboolean (L, 1);
	return 1;
}
