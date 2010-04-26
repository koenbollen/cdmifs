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
	int i;
	const char *cp;
	char childbuf[PATH_MAX];
	json_t *children;

	children = cdmi_get( path, (char*[]){"children",NULL}, CDMI_SINGLE );
	if( children == NULL || !json_is_array( children ) )
		return errno == 0 ? -1 : -errno;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	for(i = 0; i < json_array_size(children); i++)
	{
		if( json_is_string( json_array_get(children, i) ) )
		{
			cp = json_string_value(json_array_get(children, i));
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

	json_decref( children );

	return 0;
}

int cdmifs_mkdir( const char *path, mode_t mode )
{
	int ret;
	ret = cdmi_put( path, NULL, CDMI_CONTAINER | CDMI_NONCDMI );
	if( ret < 1 )
		return errno == 0 ? -EIO : -errno;
	return 0;
}

