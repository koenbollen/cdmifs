/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "read.h"

#include "../common.h"
#include "../cdmi.h"
#include "../net.h"

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
 
	json_t *root;
	int ret, start, end;
	char valuefield[32];

	sprintf( valuefield, "value:%d-%d", (int)offset, (int)offset+size );
	root = cdmi_get( path, (char*[]){"valuerange", valuefield, NULL}, CDMI_DATAOBJECT | CDMI_CHECK );
 
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
 
	memcpy( buf, json_string_value(json_object_get(root,"value")), end-start ); 
	json_decref(root);
	return end-start; 
} 
 
