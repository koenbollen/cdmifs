
#include "attr.h" 
#include "../common.h"

#include <errno.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
 
int cdmifs_getattr( 
		const char *path, 
		struct stat *stbuf ) 
{ 
	int res = 0; 
 
	memset( stbuf, 0, sizeof(struct stat) ); 
 
	if( strcmp(path, "/") == 0 ) 
	{ 
		stbuf->st_mode = S_IFDIR | 0755; 
		stbuf->st_nlink = 2; 
	} 
	else if( strcmp(path, "/"APP_NAME) == 0 ) 
	{ 
		stbuf->st_mode = S_IFREG | 0444; 
		stbuf->st_nlink = 1; 
		stbuf->st_size = strlen( APP_VERSION ); 
	} 
	else 
	{ 
		res = -ENOENT; 
	} 
 
	return res; 
} 

