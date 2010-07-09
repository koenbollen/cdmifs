/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "attr.h"

#include "../cdmi.h"
#include "../common.h"
#include "../mime.h"
#include "../net.h"
#include "../util.h"

#include "directory.h"

#include <curl/curl.h>
#include <errno.h>
#include <fcntl.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int parse_metadata( json_t *metadata, struct stat *stbuf );

int cdmifs_getattr(
		const char *path,
		struct stat *stbuf )
{
	int ret, i;
	cdmi_request_t request;

	memset( &request, 0, sizeof( cdmi_request_t ) );
	request.type = GET;
	request.cdmi = 1;
	request.fields = (char*[]){ "objectID", "metadata", NULL };
	request.flags = CDMI_CHECK;

	ret = cdmi_get( &request, path );
	if( ret == -1 )
		return errno == 0 ? -EIO : -errno;

	if( strcmp( request.contenttype, mime[M_CONTAINER] ) == 0 )
		stbuf->st_mode = S_IFDIR;
	else if( strcmp( request.contenttype, mime[M_DATAOBJECT] ) == 0 )
		stbuf->st_mode = S_IFREG;
	else
	{
		cdmi_free( &request );
		return -EPROTO;
	}

	if( parse_metadata( json_object_get(request.root, "metadata"), stbuf ) < 0 )
	{
		DEBUG( "error: unable to parse metadata\n" );
		cdmi_free( &request );
		return -EPROTO;
	}

	cdmi_free( &request );

	if( S_ISDIR( stbuf->st_mode ) )
	{
		stbuf->st_mode |= 0755;

		memset( &request, 0, sizeof( cdmi_request_t ) );
		request.type = GET;
		request.cdmi = 1;
		request.fields = (char*[]){"children",NULL};
		request.flags = CDMI_SINGLE;
		ret = cdmi_get( &request, path );
		if( ret == -1 )
			return errno == 0 ? -EIO : -errno;

		stbuf->st_nlink = 2;
		for(i = 0; i < (int)json_array_size(request.root); i++)
		{
			if( json_is_string( json_array_get(request.root, i) ) )
			{
				const char *cp = json_string_value(json_array_get(request.root, i));
				if( cp[strlen(cp)-1] == '/' )
					(stbuf->st_nlink)++;
			}
		}

		cdmi_free( &request );

		return 0;
	}
	else
	{
		stbuf->st_mode |= 0644;
		stbuf->st_nlink = 1;

		return 0;
	}

	return -ENOENT;
}

static int parse_metadata( json_t *metadata, struct stat *stbuf )
{
	if( !json_is_object( metadata ) )
		return -1;

	if( json_is_integer( json_object_get(metadata, "cdmi_size") ) )
		stbuf->st_size = json_integer_value( json_object_get(metadata, "cdmi_size") );

	if( json_is_string( json_object_get(metadata, "cdmi_ctime") ) )
	{
		stbuf->st_ctime = iso8601_decode(
				json_string_value( json_object_get(metadata, "cdmi_ctime") )
			);
	}

	if( json_is_string( json_object_get(metadata, "cdmi_atime") ) )
	{
		stbuf->st_atime = iso8601_decode(
				json_string_value( json_object_get(metadata, "cdmi_atime") )
			);
	}

	if( json_is_string( json_object_get(metadata, "cdmi_mtime") ) )
	{
		stbuf->st_mtime = iso8601_decode(
				json_string_value( json_object_get(metadata, "cdmi_mtime") )
			);
	}

	if( options.gotmeta && json_is_string( json_object_get(metadata, "cdmifs_mode") ) )
	{
		const char *modestr = json_string_value( json_object_get(metadata, "cdmifs_mode" ) );
		sscanf( modestr, "%o", &(stbuf->st_mode) );
	}

	return 0;
}

int cdmifs_chmod( const char *path, mode_t mode )
{
	if( !options.gotmeta )
		return -ENOSYS;
	int ret;
	char modestr[16];
	sprintf( modestr, "0%o", mode );
	json_t *meta = json_object();
	json_object_set( meta, "cdmifs_mode", json_string(modestr) );
	ret = setmetadata( path, meta );
	json_decref( meta );
	if( ret != 1 )
		return errno;
	return 0;
}

int cdmifs_utimens( const char *path, const struct timespec tv[2] )
{
	(void)path;
	(void)tv;
	return 0;
}

