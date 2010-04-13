
#include "directory.h" 
 
#include "../common.h"
 
#include <errno.h> 
#include <fcntl.h> 
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
	if(strcmp(path, "/") != 0) 
		return -ENOENT; 
	filler(buf, ".", NULL, 0); 
	filler(buf, "..", NULL, 0);
	filler(buf, APP_NAME, NULL, 0); 
	return 0;
} 
 
