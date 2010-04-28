/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "control.h"

#include "../cdmi.h"
#include "../common.h"
#include "../net.h"
#include "../util.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

static int cdmifs_common(
		const char *path,
		mode_t mode,
		struct fuse_file_info *fi )
{
	(void)fi;
	int ret, flags;
	json_t *root;

	flags = CDMI_DATAOBJECT;
	if( mode == 0 )
		flags |= CDMI_CHECK;
	root = cdmi_get( path, (char*[]){"objectID",NULL}, flags );
	if( root == NULL && errno == ENOENT && mode != 0 )
	{
		/* new file */
		root = json_object();
		json_object_set(root, "mimetype", json_string(MAGIC_DEFAULT) );
		json_object_set(root, "length", json_integer( 0 ) );
		json_object_set(root, "value", json_string("") );
		ret = cdmi_put( path, root, CDMI_DATAOBJECT | CDMI_NONCDMI );
		if( ret < 1 )
			return errno == 0 ? -EIO : -errno;
		json_decref( root );
		return 0;
	}
	else if( root == NULL )
		return errno == 0 ? -EIO : -errno;

	json_decref( root );
	return 0;
}

extern int cdmifs_open(
		const char *path,
		struct fuse_file_info *fi )
{
	return cdmifs_common( path, 0, fi );
}

int cdmifs_create(
		const char *path,
		mode_t mode,
		struct fuse_file_info *fi
	)
{
	return cdmifs_common( path, mode, fi );
}


