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

static void lua_lkl_register(lua_State * L)
{
	lua_register(L, "stat", luk_stat);
	luaL_register (L, "lkl", fslib);
}





typedef void (*my_lua_registerer_t) (lua_State *L);
static const my_lua_registerer_t lualibs[] = 
{
        luaL_openlibs,
        luaopen_wapr,
        lua_lkl_register
};

/* A function to open up all the Lua libraries we've declared above. */
static void openlualibs(lua_State *l)
{
        int i;
        for(i = 0; i < sizeof(lualibs)/sizeof(my_lua_registerer_t); i++)
        {
            lualibs[i](l);
            lua_settop(l, 0);
        }
}

static void lua_run_script(lua_State*L, const char *filename) 
{
        int s = luaL_loadfile(L, filename);
        if(0 == s)
        {
                s = lua_pcall(L, 0, LUA_MULTRET, 0);
        }
        if (0 != s)
        {
                fprintf(stderr, "LUA Error: %s\n", lua_tostring(L, -1));
                lua_pop(L, 1); // remove error message
        }
}


apr_status_t lua_lkl_main(const char * script_file, apr_pool_t * root_pool)
{
        gp = root_pool;
	/* initialize Lua */
	L = lua_open();
        if(NULL == L)
            return -1;

	/* load Lua base libraries & our extensions */
	openlualibs(L);

	/* run the script */
	lua_run_script(L, script_file);

	/* cleanup Lua */
	lua_close(L);

	return APR_SUCCESS;
}




