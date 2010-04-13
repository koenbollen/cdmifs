
#ifndef CONTROL_H 
#define CONTROL_H 1 

#include <sys/types.h> 
#include "../common.h" 
 
extern int cdmifs_open( 
		const char *path, 
		struct fuse_file_info *fi 
	); 
 
#endif /* !CONTROL_H */ 
 
