
#include "control.h" 
 
#include "../common.h"
 
#include <errno.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
 
extern int cdmifs_open( 
		const char *path, 
		struct fuse_file_info *fi ) 
{ 
	if( strcmp(path, "/"APP_NAME) != 0 ) 
		return -ENOENT; 
	if((fi->flags & 3) != O_RDONLY ) 
		return -EACCES; 
	return 0; 
} 
 
