/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "control.h" 
 
#include "../cdmi.h" 
#include "../common.h" 
#include "../net.h" 
#include "../util.h" 
 
#include <errno.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <curl/curl.h> 
 
extern int cdmifs_open( 
		const char *path, 
		struct fuse_file_info *fi ) 
{ 
	json_t *root;
	root = cdmi_request( path, (char*[]){"objectID",NULL}, CDMI_DATAOBJECT | CDMI_CHECK );
	if( root == NULL )
		return errno == 0 ? -EIO : -errno; 

	return 0; 
} 
 
