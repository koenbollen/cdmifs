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

#include <assert.h>
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
	int ret;

	cdmi_request_t request;
	memset( &request, 0, sizeof( cdmi_request_t ) );
	request.type = GET;
	request.cdmi = 0;
	request.offset = offset;
	request.length = size;
	request.flags = CDMI_DATAOBJECT;

	ret = cdmi_get( &request, path );
	if( ret == -1 )
	{
		cdmi_free( &request );
		return errno == 0 ? -EIO : -errno;
	}

	if( request.length > size )
	{
		DEBUG( "recv more the needed!\n" );
		request.length = size;
	}

	memcpy( buf, request.rawdata, request.length );

	ret = request.length;
	cdmi_free( &request );
	return ret;
}

