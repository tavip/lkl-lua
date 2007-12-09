#include "utils.h"
#include "lua_main.h"
#define LUA_LIB
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_wapr.h"


#include "wapr_fileops.h"
#include "wapr_user.h"

#define DIR_METATABLE "APR directory metatable"

/* the Lua interpreter */
lua_State* L;
apr_pool_t * gp;

struct dir_data_t
{
        apr_dir_t * d;
	int  closed;
};


int luapr_stat(lua_State * L)
{
	int n;
	const char * fname;
	int rc;
	apr_finfo_t finfo;

        fname = luaL_checkstring (L, 1);

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

int luapr_get_pid(lua_State * L)
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





/**
 * Directory iterator
 */
static int luapr_dir_iter (lua_State *L)
{
        apr_status_t rc;
        apr_finfo_t finfo;
	struct dir_data_t * sd;

        sd = (struct dir_data_t*)lua_touserdata (L, lua_upvalueindex (1));

	luaL_argcheck (L, !sd->closed, 1, "closed directory");

        rc = apr_dir_read(&finfo, APR_FINFO_NAME, sd->d);
        if(APR_SUCCESS == rc)
        {
                printf("%s\n", finfo.name);
                lua_pushstring (L, finfo.name);
                return 1;
        }
        else
        {
		/* no more entries => close directory */
                sd->closed = 1;
		apr_dir_close (sd->d);
		return 0;
	}
}




/**
 * Factory of directory iterators
 */
static int luapr_dir_iter_factory (lua_State *L)
{
	const char *path;
        apr_status_t rc;
        struct dir_data_t * sd;

	path = luaL_checkstring (L, 1);
	sd = (struct dir_data_t*)lua_newuserdata (L, sizeof(struct dir_data_t *));
	luaL_getmetatable (L, DIR_METATABLE);
	lua_setmetatable (L, -2);

        sd->closed = 0;
	rc = apr_dir_open(&sd->d, path, gp);
        if(APR_SUCCESS != rc)
        {
                sd->closed = 1;
		luaL_error (L, "cannot open %s: %s", path, lfd_apr_strerror_thunsafe (rc));
        }
	lua_pushcclosure (L, luapr_dir_iter, 1);
        return 1;
}


/**
 * Closes directory iterators
 */
static int luapr_dir_close (lua_State *L)
{
	struct dir_data_t * sd = (struct dir_data_t*)lua_touserdata (L, 1);
        if(!sd->closed)
        {
                apr_dir_close(sd->d);
	}
	return 0;
}

/**
 * Creates directory metatable.
 */
static int dir_create_meta (lua_State *L) {
	luaL_newmetatable (L, DIR_METATABLE);
	/* set its __gc field */
	lua_pushstring (L, "__gc");
	lua_pushcfunction (L, luapr_dir_close);
	lua_settable (L, -3);
	return 1;
}

static const struct luaL_reg fslib[] = {
	//{"attributes", file_info},
	{"get_pid",      luapr_get_pid             },
        {"stat",         luapr_stat                },
        {"dir",          luapr_dir_iter_factory    },
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
	lua_register(L, "stat", luapr_stat);
        dir_create_meta(L);
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




