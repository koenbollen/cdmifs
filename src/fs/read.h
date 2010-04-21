/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#ifndef READ_H 
#define READ_H 1 

#include <sys/types.h> 
#include "../common.h" 
 
extern int cdmifs_read( 
		const char *path, 
		char *buf, 
		size_t size, 
		off_t offset, 
		struct fuse_file_info *fi 
	); 
 
#endif /* !READ_H */ 
 
