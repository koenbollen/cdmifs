
#ifndef ATTR_H 
#define ATTR_H 1 

#include <sys/stat.h> 
 
extern int cdmifs_getattr( 
		const char *path, 
		struct stat *stbuf 
	); 
 
#endif /* !ATTR_H */ 
 
