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

#include <unistd.h>
int cdmifs_write(
		const char *path,
		const char *data, size_t size, off_t offset,
		struct fuse_file_info *fi )
{
	int ret;
	json_t *meta, *info;

	const char *mime = NULL;

	meta = getmetadata( path );
	if( meta && offset != 0 )
		mime = json_string_value( json_object_get(meta, "mimetype") );
	if( mime == NULL )
		mime = mimetype(data, size);
	json_decref( meta );

	if( data == NULL )
		data = "";

	errno = 0;
	info = json_object();
	json_object_set(info, "value", json_string_nocheck(data) );
	json_object_set(info, "length", json_integer(size) );
	json_object_set(info, "offset", json_integer(offset) );
	json_object_set(info, "mimetype", json_string(mime) );
	ret = cdmi_put( path, info, CDMI_DATAOBJECT | CDMI_NONCDMI );
	json_decref( info );
	if( ret < 1 )
		return errno == 0 ? -EIO : -errno;
	return size;
}

