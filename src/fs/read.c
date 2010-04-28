/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "read.h"

#include "../common.h"
#include "../cdmi.h"
#include "../net.h"
#include "../util.h"
#include "../b64/cdecode.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <jansson.h>

int cdmifs_read(
		const char *path,
		char *buf,
		size_t size,
		off_t offset,
		struct fuse_file_info *fi )
{
	(void)fi;

	json_t *root;
	int ret, start, end;
	char valuefield[32];
	char *data = NULL;

	sprintf( valuefield, "value:%d-%d", (int)offset, (int)offset+size );
	root = cdmi_get( path, (char*[]){"valuerange", valuefield, "mimetype", NULL}, CDMI_DATAOBJECT | CDMI_CHECK );

	ret = sscanf( json_string_value( json_object_get(root,"valuerange") ), "%u-%u", &start, &end );
	if( ret != 2 )
	{
		json_decref(root);
		return -EIO;
	}

	if( !json_is_string( json_object_get(root,"value") ) )
	{
		json_decref(root);
		return -EIO;
	}

	if( end-start > (int)size )
	{
		return -EPROTO;
	}

	data = NULL;
	size = end-start;
	if( !startswith( json_string_value( json_object_get(root, "mimetype") ), "text/" ) )
	{
		size = b64_dsize( end-start );
		data = alloc( NULL, size );
		if( data == NULL )
			return -errno;

		base64_decodestate state;
		base64_init_decodestate( &state );
		ret = base64_decode_block(
				json_string_value(json_object_get(root,"value")),
				end-start,
				data,
				&state
			);
		size = ret;
	}

	if( data == NULL )
	{
		memcpy( buf, json_string_value(json_object_get(root,"value")), size );
	}
	else
	{
		memcpy( buf, data, size );
		free( data );
	}
	json_decref(root);
	return size;
}

