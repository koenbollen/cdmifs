/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#ifndef DIRECTORY_H 
#define DIRECTORY_H 1 

#include <sys/types.h>
#include <curl/curl.h>
#include <jansson.h>
#include "../common.h" 
 
extern int cdmifs_readdir( 
		const char *path, 
		void *buf, 
		fuse_fill_dir_t filler, 
		off_t offset, 
		struct fuse_file_info *fi 
	); 

extern int getchildren( CURL *curl, const char *path, json_t **result );

#endif /* !DIRECTORY_H */ 

