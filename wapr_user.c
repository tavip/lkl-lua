#ifdef LKL_FILE_APIS
#include <apr_strings.h>
#include "wapr_user.h"

/**
 * Get the userid (and groupid) of the calling process
 * @param userid   Returns the user id
 * @param groupid  Returns the user's group id
 * @param p The pool from which to allocate working space
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_uid_current(wapr_uid_t *userid,
                                          wapr_gid_t *groupid,
                                          apr_pool_t *p)
{
    *userid = lkl_sys_getuid();
    *groupid = lkl_sys_getgid();
    return APR_SUCCESS;
}

/**
 * Get the user name for a specified userid
 * @param username Pointer to new string containing user name (on output)
 * @param userid The userid
 * @param p The pool from which to allocate the string
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_uid_name_get(char **username, wapr_uid_t userid,
                                           apr_pool_t *p)
{
    //TODO:FIXME: find a propper value
    *username = apr_pstrdup(p, "WAPR_FIXME__NO_USERNAME_SET");
    return APR_SUCCESS;
}

/**
 * Get the userid (and groupid) for the specified username
 * @param userid   Returns the user id
 * @param groupid  Returns the user's group id
 * @param username The username to lookup
 * @param p The pool from which to allocate working space
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_uid_get(wapr_uid_t *userid, wapr_gid_t *groupid,
                                      const char *username, apr_pool_t *p)
{
    //TODO:FIXME: we currently find the CURRENT uid and gid, the function should find the uid and gid for the specified USERNAME.
    *userid = lkl_sys_getuid();
    *groupid = lkl_sys_getgid();
    return APR_SUCCESS;
}

/**
 * Get the home directory for the named user
 * @param dirname Pointer to new string containing directory name (on output)
 * @param username The named user
 * @param p The pool from which to allocate the string
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_uid_homepath_get(char **dirname, 
                                               const char *username, 
                                               apr_pool_t *p)
{
    //TODO:FIXME: we currently return "/" because we don;t know where the user's home dir is situated.
    *dirname = apr_pstrdup(p, "/");
    return APR_SUCCESS;
}


/**
 * Get the group name for a specified groupid
 * @param groupname Pointer to new string containing group name (on output)
 * @param groupid The groupid
 * @param p The pool from which to allocate the string
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_gid_name_get(char **groupname, 
                                             wapr_gid_t groupid, apr_pool_t *p)
{
    *groupname = apr_pstrdup(p, "/");
    return APR_SUCCESS;
}

/**
 * Get the groupid for a specified group name
 * @param groupid Pointer to the group id (on output)
 * @param groupname The group name to look up
 * @param p The pool from which to allocate the string
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_gid_get(wapr_gid_t *groupid, 
                                      const char *groupname, apr_pool_t *p)
{
    //TODO:FIXME: we return the current user's GID because we don;t have a mappig between usernames and gids
    *groupid = lkl_sys_getgid();
    return APR_SUCCESS;
}

#endif//LKL_FILE_APIS
