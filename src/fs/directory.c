/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "directory.h"

#include "../common.h"
#include "../cdmi.h"
#include "../net.h"

#include <curl/curl.h>
#include <errno.h>
#include <fcntl.h>
#include <jansson.h>
#include <net.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int cdmifs_readdir(
		const char *path,
		void *buf,
		fuse_fill_dir_t filler,
		off_t offset,
		struct fuse_file_info *fi )
{
	(void)offset;
	(void)fi;

	int ret;
	unsigned int i;
	const char *cp;
	char childbuf[PATH_MAX];

	cdmi_request_t request;

	memset( &request, 0, sizeof( cdmi_request_t ) );
	request.type = GET;
	request.cdmi = 1;
	request.fields = (char*[]){"children",NULL};
	request.flags = CDMI_SINGLE;

	ret = cdmi_get( &request, path );
	if( ret == -1 || !json_is_array( request.root ) )
	{
		cdmi_free( &request );
		return errno == 0 ? -1 : -errno;
	}

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	for(i = 0; i < json_array_size(request.root); i++)
	{
		if( json_is_string( json_array_get(request.root, i) ) )
		{
			cp = json_string_value(json_array_get(request.root, i));
			if( cp[strlen(cp)-1] == '/' )
			{
				strcpy( childbuf, cp );
				childbuf[strlen(cp)-1] = 0;
				filler( buf, childbuf, NULL, 0 );
			}
			else
				filler( buf, cp, NULL, 0 );
		}
	}

	cdmi_free( &request );

	return 0;
}

int cdmifs_mkdir( const char *path, mode_t mode )
{
	(void)mode;

	int ret;
	cdmi_request_t request;
	memset( &request, 0, sizeof( cdmi_request_t ) );
	request.type = PUT;
	request.cdmi = 0;
	request.flags = CDMI_CONTAINER;
	ret = cdmi_put( &request, path );
	if( ret == -1 )
	{
		cdmi_free( &request );
		return errno == 0 ? -EIO : -errno;
	}
	cdmi_free( &request );
	return 0;
}

