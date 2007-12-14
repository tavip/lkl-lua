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
        wapr_dir_t * d;
	int  closed;
};


static const int stat_field_options[] = 
{
    APR_FINFO_LINK   ,
    APR_FINFO_MTIME  ,
    APR_FINFO_CTIME  ,
    APR_FINFO_ATIME  ,
    APR_FINFO_SIZE   ,
    APR_FINFO_CSIZE  ,
    APR_FINFO_DEV    ,
    APR_FINFO_INODE  ,
    APR_FINFO_NLINK  ,
    APR_FINFO_TYPE   ,
    APR_FINFO_USER   ,
    APR_FINFO_GROUP  ,
    APR_FINFO_UPROT  ,
    APR_FINFO_GPROT  ,
    APR_FINFO_WPROT  ,
    APR_FINFO_ICASE  ,
    APR_FINFO_NAME   ,
    APR_FINFO_MIN    ,
    APR_FINFO_IDENT  ,
    APR_FINFO_OWNER  ,
    APR_FINFO_PROT   ,
    APR_FINFO_NORM   ,
    APR_FINFO_DIRENT 
};

static const char* stat_string_options[] = 
{
    "link"   ,
    "mtime"  ,
    "ctime"  ,
    "atime"  ,
    "size"   ,
    "csize"  ,
    "dev"    ,
    "inode"  ,
    "nlink"  ,
    "type"   ,
    "user"   ,
    "group"  ,
    "uprot"  ,
    "gprot"  ,
    "wprot"  ,
    "icase"  ,
    "name"   ,
    "min"    ,
    "ident"  ,
    "owner"  ,
    "prot"   ,
    "norm"   ,
    "dirent" 
};

typedef int (*Selector)(lua_State *L, int i, const void *data);

static int doselection(lua_State *L, int i, const char *const S[], Selector F, const void *data)
{
	if (lua_isnone(L, i))
	{
		lua_newtable(L);
		for (i=0; S[i]!=NULL; i++)
		{
			lua_pushstring(L, S[i]);
			F(L, i, data);
			lua_settable(L, -3);
		}
		return 1;
	}
	else
	{
		int j = luaL_checkoption(L, i, NULL, S);
		if (j==-1) luaL_argerror(L, i, "unknown selector");
		return F(L, j, data);
	}
}

static apr_int32_t string_modes_to_int32(lua_State*L, int start_arg)
{
    apr_int32_t ret = 0;
    int i, j, n;
    n = lua_gettop(L);
    for(i = start_arg; i <= n; i++)
    {
    	j = luaL_checkoption(L, i, NULL, stat_string_options);
        if(-1 == j)
        {
            luaL_argerror(L, i, "unknown selector");
            return 0;
        }
        else
        {
            ret |= stat_field_options[j];
        }
    }
    return ret;
}


static const char * apr_file_type_to_string(apr_filetype_e filetype)
{
    switch(filetype)
    {
        case APR_NOFILE: return "nofile";
        case APR_REG: 	 return "regular";
        case APR_DIR: 	 return "directory";
        case APR_CHR: 	 return "character";
        case APR_BLK: 	 return "block";
        case APR_PIPE:   return "pipe";
        case APR_LNK: 	 return "symbolic";
        case APR_SOCK: 	 return "socket";
        default:
        case APR_UNKFILE:return "unknown";
    }
}

static void apr_finfo_number_to_field(lua_State*L, int fieldno, apr_finfo_t*finfo, int table_pos)
{

    switch(fieldno)
    {
        //case 0: LINK, nothing to do
        case 1: lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->mtime); lua_rawset(L, table_pos);    break;
        case 2: lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->ctime); lua_rawset(L, table_pos);    break;
        case 3: lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->atime); lua_rawset(L, table_pos);    break;

        case 4: lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->size);  lua_rawset(L, table_pos);    break;
        case 5: lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->csize); lua_rawset(L, table_pos);    break;

        case 6: lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->device);lua_rawset(L, table_pos);    break;
        case 7: lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->inode); lua_rawset(L, table_pos);    break;
        case 8: lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->nlink); lua_rawset(L, table_pos);    break;
        case 9: lua_pushstring(L, stat_string_options[fieldno]);    lua_pushstring(L, apr_file_type_to_string(finfo->filetype));
                lua_rawset(L, table_pos);    break;
        case 10:lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->user);  lua_rawset(L, table_pos);    break;
        case 11:lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->group); lua_rawset(L, table_pos);    break;
        case 12:lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->protection); lua_rawset(L, table_pos);    break;//TODO:
        case 13:lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->protection); lua_rawset(L, table_pos);    break;//TODO:
        case 14:lua_pushstring(L, stat_string_options[fieldno]);    lua_pushnumber(L, finfo->protection); lua_rawset(L, table_pos);    break;//TODO:
        //case 15:     nothing to output
        case 16:lua_pushstring(L, stat_string_options[fieldno]);    lua_pushstring(L, finfo->name); lua_rawset(L, table_pos);    break;
        //case 17: MIN:combination of other fields,
        //case 18: IDENT: inode and dev
        //case 19: OWNER: user&group
        //case 20: PROT: done by userp, groupp, otherp
        //case 21: norm
        //case 22: dirent
    }
}
static apr_int32_t finfo_to_lua_results(lua_State*L, int start_arg, apr_finfo_t * finfo)
{
    int i, mask;
    const int stat_field_options_size = sizeof(stat_field_options)/sizeof(stat_field_options[0]);
    lua_newtable(L);
    for(mask = 1; mask != 0; mask <<= 1)
    {
	if(finfo->valid & mask)
	{
	    for(i = 0; i < stat_field_options_size; i++)
	    {
		if(mask == stat_field_options[i])
		{
		    break;
		}
	    }
	    if(i != stat_field_options_size)
	    {
		apr_finfo_number_to_field(L, i, finfo, -3);
	    }
	}
    }
    return 1;
}

int luapr_stat(lua_State * L)
{
	const char * fname;
	apr_status_t rc;
	apr_finfo_t finfo;
        apr_int32_t wanted;
        fname = luaL_checkstring (L, 1);
        wanted = string_modes_to_int32(L, 2);
	if(0 == wanted)
	{
	    wanted = -1;
	}
	rc = wapr_stat(&finfo, fname, wanted, gp);
	if(APR_SUCCESS != rc && APR_INCOMPLETE != rc)
	{
            lua_pushstring(L, "Error accessing file, error: ");
            lua_pushstring(L, lfd_apr_strerror_thunsafe(rc));
	    lua_error(L);
	}
	
	return finfo_to_lua_results(L, 2, &finfo);
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

        rc = wapr_dir_read(&finfo, APR_FINFO_NAME, sd->d);
        if(APR_SUCCESS == rc)
        {
                lua_pushstring (L, finfo.name);
                return 1;
        }
        else
        {
		/* no more entries => close directory */
                sd->closed = 1;
		wapr_dir_close (sd->d);
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
	sd = (struct dir_data_t*)lua_newuserdata (L, sizeof(struct dir_data_t));
	luaL_getmetatable (L, DIR_METATABLE);
	lua_setmetatable (L, -2);

        sd->closed = 0;
	rc = wapr_dir_open(&sd->d, path, gp);
        if(APR_SUCCESS != rc)
        {
                sd->closed = 1;
                return 0;
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
                sd->closed = 1;
                wapr_dir_close(sd->d);
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

static int luapr_file_rename(lua_State * L)
{
    apr_status_t rc;
    const char * source, * dest;
    source = luaL_checkstring(L, 1);
    dest = luaL_checkstring(L, 2);
    rc = wapr_file_rename(source, dest, gp);
    if(APR_SUCCESS != rc)
    {
	printf("wapr_file_rename in luapr_file_rename failed with %d, %s\n", rc, lfd_apr_strerror_thunsafe(rc));
	printf("wapr_file_rename failed for [%s] -> [%s]\n", source, dest);
    }
    lua_pushnumber(L, rc);
    return 1;
}

static int luapr_file_copy(lua_State * L)
{
    apr_status_t rc;
    const char * source, * dest;
    source = luaL_checkstring(L, 1);
    dest = luaL_checkstring(L, 2);
    //TODO: get user set permissions from the LUA state
    rc = wapr_file_copy(source, dest, APR_OS_DEFAULT, gp);
    if(APR_SUCCESS != rc)
    {
        printf("wapr_file_copy in luapr_file_copy failed with %d, %s\n", rc, lfd_apr_strerror_thunsafe(rc));
        printf("wapr_file_copy failed for [%s] -> [%s]\n", source, dest);
    }
    lua_pushnumber(L, rc);
    return 1;
}

static int luapr_dir_make(lua_State * L)
{
    apr_status_t rc;
    const char * path;
    path = luaL_checkstring(L, 1);
    //TODO: get permissions from the script
    rc = wapr_dir_make(path, APR_OS_DEFAULT, gp);
    if(APR_SUCCESS != rc)
    {
        printf("wapr_dir_make in luapr_dir_make failed with %d, %s\n", rc, lfd_apr_strerror_thunsafe(rc));
        printf("wapr_dir_make failed for [%s]\n", path);
    }
    lua_pushnumber(L, rc);
    return 1;
}

static int luapr_dir_make_recursive(lua_State * L)
{
    apr_status_t rc;
    const char * path;
    path = luaL_checkstring(L, 1);
    //TODO: get permissions from the script
    rc = wapr_dir_make_recursive(path, APR_OS_DEFAULT, gp);
    if(APR_SUCCESS != rc)
    {
        printf("wapr_dir_make in luapr_dir_make failed with %d, %s\n", rc, lfd_apr_strerror_thunsafe(rc));
        printf("wapr_dir_make failed for [%s]\n", path);
    }
    lua_pushnumber(L, rc);
    return 1;
}

static const struct luaL_reg fslib[] = {
	//{"attributes", file_info},
	{"get_pid",      luapr_get_pid             },
        {"stat",         luapr_stat                },
        {"dir",          luapr_dir_iter_factory    },
	{"file_rename",  luapr_file_rename         },
	{"file_copy",    luapr_file_copy           },
	{"dir_make",     luapr_dir_make            },
	{"dir_make_rec", luapr_dir_make_recursive  },
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




