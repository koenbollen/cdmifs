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
	int ret;
	cdmi_request_t request;

	memset( &request, 0, sizeof( cdmi_request_t ) );
	request.type = GET;
	request.cdmi = 1;
	request.fields = (char*[]){"objectID","mimetype",NULL};
	request.flags = CDMI_DATAOBJECT;
	if( mode == 0 )
		request.flags |= CDMI_CHECK;
	ret = cdmi_get( &request, path );
	if( ret == -1 && errno == ENOENT && mode != 0 )
	{
		/* new file */
		cdmi_free( &request );
		memset( &request, 0, sizeof( cdmi_request_t ) );
		request.type = PUT;
		request.cdmi = 0;
		request.length = 0;
		request.rawdata = NULL;
		request.contenttype = MAGIC_DEFAULT;
		request.flags = CDMI_DATAOBJECT;
		ret = cdmi_put( &request, path );
		if( ret < 1 )
			return errno == 0 ? -EIO : -errno;
	}
	else if( ret == -1 )
	{
		cdmi_free( &request );
		return errno == 0 ? -EIO : -errno;
	}

	json_t *handle = request.root;
	json_incref( handle );
	fi->fh = (unsigned long)handle;

	cdmi_free( &request );

	return 0;
}

int cdmifs_open(
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

int cdmifs_release(
		const char *path,
		struct fuse_file_info *fi )
{
	(void)path;
	if( fi->fh != 0 )
	{
		json_t *root = long2pointer( fi->fh );
		json_decref( root );
		fi->fh = 0;
	}
	return 0;
}

int cdmifs_unlink( const char *path )
{
	int ret;
	cdmi_request_t request;
	memset( &request, 0, sizeof( cdmi_request_t ) );
	request.type = DELETE;
	request.cdmi = 0;
	ret = cdmi_delete( &request, path );
	if( ret == -1 )
		return errno == 0 ? -EIO : -errno;
	return 0;
}

