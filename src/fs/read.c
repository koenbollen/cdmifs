
#include "read.h" 
 
#include "../common.h"
 
#include <errno.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 

int cdmifs_read( 
		const char *path, 
		char *buf, 
		size_t size, 
		off_t offset, 
		struct fuse_file_info *fi ) 
{ 
	size_t len; 
	if( strcmp(path, "/"APP_NAME) != 0 ) 
		return -ENOENT; 
	len = strlen( APP_VERSION ); 
	if( offset < len )
	{ 
		if( offset + size > len ) 
			size = len - offset; 
		memcpy( buf, APP_VERSION + offset, size ); 
	} 
	else 
	{ 
		size = 0; 
	} 
	return size; 
} 
 
