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
	int i;
	json_t *root;

	root = cdmi_get( path, (char*[]){ "objectID", "metadata", NULL }, CDMI_CHECK | CDMI_CONTENTTYPE );
	if( root == NULL )
		return errno == 0 ? -EIO : -errno;

	if( strcmp( json_string_value( json_object_get( root, "_contenttype" ) ), mime[M_CONTAINER] ) == 0 )
		stbuf->st_mode = S_IFDIR;
	else if( strcmp( json_string_value( json_object_get( root, "_contenttype" ) ), mime[M_DATAOBJECT] ) == 0 )
		stbuf->st_mode = S_IFREG;
	else
	{
		json_decref( root );
		return -EPROTO;
	}

	if( parse_metadata( json_object_get(root, "metadata"), stbuf ) < 0 )
	{
		DEBUG( "error: unable to parse metadata\n" );
		json_decref( root );
		return -EPROTO;
	}

	json_decref( root );

	if( S_ISDIR( stbuf->st_mode ) )
	{
		stbuf->st_mode |= 0755;
		root = cdmi_get( path, (char*[]){"children",NULL}, CDMI_SINGLE );
		if( root == NULL )
			return errno == 0 ? -EIO : -errno;

		stbuf->st_nlink = 2;
		for(i = 0; i < json_array_size(root); i++)
		{
			if( json_is_string( json_array_get(root, i) ) )
			{
				const char *cp = json_string_value(json_array_get(root, i));
				if( cp[strlen(cp)-1] == '/' )
					(stbuf->st_nlink)++;
			}
		}

		json_decref( root );

		return 0;
	}
	else
	{
		stbuf->st_mode |= 0444;
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

	return 0;
}


