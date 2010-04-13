
#ifndef DIRECTORY_H 
#define DIRECTORY_H 1 

#include <sys/types.h>
#include "../common.h" 
 
extern int cdmifs_readdir( 
		const char *path, 
		void *buf, 
		fuse_fill_dir_t filler, 
		off_t offset, 
		struct fuse_file_info *fi 
	); 

#endif /* !DIRECTORY_H */ 

