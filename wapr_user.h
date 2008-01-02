#ifndef LKL_USER_H
#define LKL_USER_H

#ifndef LKL_FILE_APIS
		
#define wapr_uid_t 		apr_uid_t
#define wapr_gid_t 		apr_gid_t
#define wapr_uid_current 	apr_uid_current
#define wapr_uid_name_get 	apr_uid_current
#define wapr_uid_get 		apr_uid_get
#define wapr_uid_homepath_get 	apr_uid_homepath_get
#define wapr_gid_name_get 	apr_gid_name_get
#define wapr_gid_get 		apr_gid_get

#define wapr_gid_compare(left,right) (((left) == (right)) ? LKL_SUCCESS : LKL_EMISMATCH)
#define wapr_uid_compare(left,right) (((left) == (right)) ? APR_SUCCESS : APR_EMISMATCH)

#else//LKL_FILE_APIS

/**
 * @file wapr_user.h
 * @brief LKL User ID Services 
 */
#include <apr.h>
#include <apr_errno.h>
#include <apr_pools.h>

#include <asm/lkl.h>

/**
 * Structure for determining user ownership.
 */

typedef __kernel_uid_t                     wapr_uid_t;

/**
 * Structure for determining group ownership.
 */
typedef __kernel_gid_t                     wapr_gid_t;


/**
 * Get the userid (and groupid) of the calling process
 * @param userid   Returns the user id
 * @param groupid  Returns the user's group id
 * @param p The pool from which to allocate working space
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_uid_current(wapr_uid_t *userid,
                                          wapr_gid_t *groupid,
                                          apr_pool_t *p);

/**
 * Get the user name for a specified userid
 * @param username Pointer to new string containing user name (on output)
 * @param userid The userid
 * @param p The pool from which to allocate the string
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_uid_name_get(char **username, wapr_uid_t userid,
                                           apr_pool_t *p);

/**
 * Get the userid (and groupid) for the specified username
 * @param userid   Returns the user id
 * @param groupid  Returns the user's group id
 * @param username The username to lookup
 * @param p The pool from which to allocate working space
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_uid_get(wapr_uid_t *userid, wapr_gid_t *groupid,
                                      const char *username, apr_pool_t *p);

/**
 * Get the home directory for the named user
 * @param dirname Pointer to new string containing directory name (on output)
 * @param username The named user
 * @param p The pool from which to allocate the string
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_uid_homepath_get(char **dirname, 
                                               const char *username, 
                                               apr_pool_t *p);

/**
 * Compare two user identifiers for equality.
 * @param left One uid to test
 * @param right Another uid to test
 * @return APR_SUCCESS if the apr_uid_t strutures identify the same user,
 * APR_EMISMATCH if not, APR_BADARG if an apr_uid_t is invalid.
 * @remark This function is available only if APR_HAS_USER is defined.
 */
#define apr_uid_compare(left,right) (((left) == (right)) ? APR_SUCCESS : APR_EMISMATCH)


/**
 * Get the group name for a specified groupid
 * @param groupname Pointer to new string containing group name (on output)
 * @param groupid The groupid
 * @param p The pool from which to allocate the string
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_gid_name_get(char **groupname, 
                                             wapr_gid_t groupid, apr_pool_t *p);

/**
 * Get the groupid for a specified group name
 * @param groupid Pointer to the group id (on output)
 * @param groupname The group name to look up
 * @param p The pool from which to allocate the string
 * @remark This function is available only if APR_HAS_USER is defined.
 */
APR_DECLARE(apr_status_t) wapr_gid_get(wapr_gid_t *groupid, 
                                      const char *groupname, apr_pool_t *p);

/**
 * Compare two group identifiers for equality.
 * @param left One gid to test
 * @param right Another gid to test
 * @return APR_SUCCESS if the apr_gid_t strutures identify the same group,
 * APR_EMISMATCH if not, APR_BADARG if an apr_gid_t is invalid.
 * @remark This function is available only if APR_HAS_USER is defined.
 */
#define apr_gid_compare(left,right) (((left) == (right)) ? APR_SUCCESS : APR_EMISMATCH)

#endif//LKL_FILE_APIS

#endif  /* ! LKL_USER_H */
