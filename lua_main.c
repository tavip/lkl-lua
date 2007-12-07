#include "utils.h"
#include "lua_main.h"
#define LUA_LIB
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_wapr.h"


#include "wapr_fileops.h"
#include "wapr_user.h"

/* the Lua interpreter */
lua_State* L;
apr_pool_t * gp;
int luk_stat(lua_State * L)
{
	int n = lua_gettop(L);
	const char * fname;
	int rc;
	apr_finfo_t finfo;

	if(1 != n || !lua_isstring(L, 1))
	{
		lua_pushstring(L, "Expecting one string argument");
	    lua_error(L);
	}

	fname = lua_tostring(L, 1);
	rc = wapr_stat(&finfo, fname, APR_FINFO_SIZE|APR_FINFO_TYPE, gp);
	if(APR_SUCCESS != rc)
	{
		lua_pushstring(L, "Error accessing file, error: ");
		lua_pushstring(L, lfd_apr_strerror_thunsafe(rc));
	    lua_error(L);
	}
	
	//lua_pushnumber(L, finfo.inode);
	lua_pushnumber(L, finfo.filetype);
	lua_pushnumber(L, finfo.size);
	//lua_pushstring(L, finfo.name);
	//lua_pushstring(L, finfo.fname);
	return 2;
}

int luk_get_pid(lua_State * L)
{
	apr_status_t rc;
	wapr_uid_t userid;
	wapr_gid_t groupid;
	
	rc = wapr_uid_current(&userid, &groupid, gp);
	if(APR_SUCCESS != rc)
	{
		lua_pushnil (L);
		lua_pushfstring (L, "cannot obtain user and process id information: '%s'", lfd_apr_strerror_thunsafe(rc));
	}
	lua_pushnumber(L, userid);
	lua_pushnumber(L, groupid);
	return 2;
}

static const struct luaL_reg fslib[] = {
	//{"attributes", file_info},
	{"get_pid", luk_get_pid},
/*
	{"chdir", change_dir},
	{"currentdir", get_dir},
	{"dir", dir_iter_factory},
	{"lock", file_lock},
	{"mkdir", make_dir},
	{"rmdir", remove_dir},
	{"touch", file_utime},
	{"unlock", file_unlock},
*/
	{NULL, NULL},
};

static apr_status_t lua_wapr_register(lua_State * L, apr_pool_t * root_pool)
{
	gp = root_pool;
	lua_register(L, "stat", luk_stat);
	luaL_register (L, "lkl", fslib);
	return APR_SUCCESS;
}
///=========================================================


















///==============================================================

static const luaL_reg lualibs[] =
{
        { "base",       luaopen_base },
        { "lkl",        lua_wapr_register},
        { NULL,         NULL }
};

/* A function to open up all the Lua libraries we've declared above. */
static void openlualibs(lua_State *l)
{
        const luaL_reg *lib;

        for (lib = lualibs; lib->func != NULL; lib++)
        {
                lib->func(l);
                lua_settop(l, 0);
        }
}

static void lua_run_script(lua_State*L, const char *filename) {
	#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >=501
	(void)luaL_dofile(L, filename);
	#else
	(void)lua_dofile(L, filename);
	#endif
}


apr_status_t lua_lkl_main(const char * script_file, apr_pool_t * root_pool)
{
	/* initialize Lua */
	L = lua_open();

	/* load Lua base libraries */
	openlualibs(L);

	/* register our functions */
        luaopen_wapr(L);

	/* run the script */
	lua_run_script(L, script_file);

	/* cleanup Lua */
	lua_close(L);

	return APR_SUCCESS;
}




