// implement the main() + config, pass control to listener
#include <stdlib.h>
#include <signal.h>

#include <apr.h>
#include <apr_pools.h>
#include <apr_signal.h>
#include <apr_general.h>
#include <apr_errno.h>
#include <apr_atomic.h>
#include <apr_getopt.h>
#include <apr_file_io.h>
#include <apr_file_info.h>

#include <asm/lkl.h>
#include <asm/disk.h>
#include <asm/env.h>

#include "utils.h"
#include "lua_main.h"

apr_pool_t	* root_pool;
static const char *script_name="default.lua";

#ifdef LKL_FILE_APIS
static const char *disk_image="disk";
static const char *fs_type;
static char mount_point[32];
static int ro=0;
static __kernel_dev_t devno;
static apr_file_t *disk_file;

int lkl_add_disk(void)
{
	apr_status_t rc;
	apr_off_t off=0;

	rc=apr_file_open(&disk_file, disk_image,
			 APR_FOPEN_READ| (ro?0:APR_FOPEN_WRITE)|
			 APR_FOPEN_BINARY, APR_OS_DEFAULT,
			 root_pool);
	if (rc != APR_SUCCESS) {
		lfd_log(LFD_ERROR, "failed to open disk image '%s': %s", disk_image, lfd_apr_strerror_thunsafe(rc));
		return -1;
	}

	rc=apr_file_seek(disk_file, APR_END, &off);
	if (rc != APR_SUCCESS) {
		lfd_log(LFD_ERROR, "failed to seek to the end of'%s': %s", disk_image, lfd_apr_strerror_thunsafe(rc));
		return -1;
	}

	devno=lkl_disk_add_disk(disk_file, off/512);
	if (devno == 0) {
		apr_file_close(disk_file);
		return -1;
	}

	return 0;
}
#endif

static const apr_getopt_option_t opt_option[] = {
	/* long-option, short-option, has-arg flag, description */
#ifdef LKL_FILE_APIS
	{ "read-only", 'r', FALSE, "read-only mode" },
	{ "fs-type", 't', TRUE, "filesystem type (ext3, etc.)" },
	{ "diskname", 'd', TRUE, "path to disk (image) to use, default='disk'" },
#endif
        { "file", 'f', TRUE, "path to script file to run, default='default.lua'" },
	{ "help", 'h', 0, "display this help and exit" },
	{ NULL, 0, 0, NULL },
};

void show_help(const char *name)
{
	const struct apr_getopt_option_t *i=opt_option;

	printf("Usage: %s [ OPTIONS ] \n\n", name);
	while (i->name)
	{
		printf(" -%c, --%s \t %s\n", (char)i->optch, i->name, i->description);
		i++;
	}
	printf("\nReport bugs to lkl-dev@ixlabs.cs.pub.ro\n");
	exit(0);
}

static int parse_command_line(int argc, char const *const * argv)
{
	int rv, optch;
	const char *optarg;

	apr_getopt_t *opt;

	apr_getopt_init(&opt, root_pool, argc, argv);

	while (APR_SUCCESS == (rv = apr_getopt_long(opt, opt_option, &optch, &optarg)))
	{
		switch (optch)
		{
#ifdef LKL_APIS
		case 'r':
			ro=1;
			break;
		case 't':
			fs_type=optarg;
			break;
		case 'd':
			disk_image=optarg;
			break;
#endif
		case 'f':
			script_name=optarg;
			break;
		case 'h':
			show_help(argv[0]);
			break;
		}
	}

	if (APR_EOF != rv)
	{
		printf("Try `%s --help` for more information.\n", argv[0]);
		return -1;
	}

	return 0;
}

int main(int argc, char const *const * argv, char const *const * engv)
{
	apr_status_t	  rc;

	rc = apr_app_initialize(&argc, &argv, &engv);
	if(APR_SUCCESS != rc)
	{
		lfd_log(LFD_ERROR, "apr_initialize_app failed with errorcode %d errormsg %s", rc, lfd_apr_strerror_thunsafe(rc));
		return 1;
	}
	atexit(apr_terminate);

	rc = apr_pool_create(&root_pool, NULL);
	if(APR_SUCCESS != rc)
	{
		lfd_log(LFD_ERROR, "main's apr_pool_create failed with errorcode %d errormsg %s", rc, lfd_apr_strerror_thunsafe(rc));
		return 2;
	}

	if (parse_command_line(argc, argv) != 0)
		return 3;

	apr_atomic_init(root_pool);

#ifdef LKL_FILE_APIS
	if (lkl_env_init(16*1024*1024) != 0) 
		return -1;

	if ((rc=lkl_add_disk()) < 0 ||
	    (rc=lkl_mount_dev(devno, fs_type, 0, NULL, mount_point,
			      sizeof(mount_point))) < 0 ||
	    (rc=lkl_sys_chdir(mount_point)) < 0 ||
	    (rc=lkl_sys_chroot(".")) < 0) {
		//FIXME: add string error code; note that the error code is not
		//compatible with apr (unless you are running on linux/i386); we
		//most likely need error codes strings in lkl itself; need to
		//fix other cases as well
		lkl_sys_halt();
		apr_file_close(disk_file);
		lfd_log(LFD_ERROR, "failed to mount disk: %d", rc);
		return -1;
	}
#endif 

	printf("Preparing to enter LUA code.\n");
	lua_lkl_main(script_name, root_pool);
	printf("LUA code is not running any more. All will be obliterated.\n");

#ifdef LKL_FILE_APIS
	lkl_sys_umount("/", 0);
	lkl_sys_halt();
	apr_file_close(disk_file);
#endif
	return 0;
}

int *stupidGdb;
