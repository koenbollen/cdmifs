/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "write.h"

#include "../common.h"
#include "../cdmi.h"
#include "../util.h"

#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>

#include <unistd.h>
int cdmifs_write(
		const char *path,
		const char *data, size_t size, off_t offset,
		struct fuse_file_info *fi )
{
	json_t *handle = long2pointer( fi->fh );

	int ret;
	const char *mime = NULL;
	cdmi_request_t request;

	if( data == NULL )
		data = "";

	mime = json_string_value( json_object_get(handle, "mimetype") );
	if( offset == 0 )
		mime = mimetype( data, size, mime );

	memset( &request, 0, sizeof( cdmi_request_t ) );
	request.type = PUT;
	request.cdmi = 0;
	request.rawdata = data;
	request.length = size;
	request.offset = offset;
	request.flags = CDMI_DATAOBJECT;
	ret = cdmi_put( &request, path );
	cdmi_free( &request );
	if( ret < 1 )
		return errno == 0 ? -EIO : -errno;
	return size;
}

int cdmifs_truncate( const char *path, off_t offset )
{
	cdmi_request_t request;
	int ret;

	memset( &request, 0, sizeof( cdmi_request_t ) );
	request.type = PUT;
	request.cdmi = 0;
	request.rawdata = NULL;
	request.length = 0;
	request.offset = offset;
	request.flags = CDMI_DATAOBJECT;
	ret = cdmi_put( &request, path );

	if( ret == -1 )
		return errno == 0 ? -EIO : -errno;
	return 0;
}

