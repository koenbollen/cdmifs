/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "control.h" 
 
#include "../cdmi.h" 
#include "../common.h" 
#include "../mime.h" 
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
	static CURL *curl = NULL; 
	if( curl == NULL ) 
	{ 
		struct curl_slist *chunk = NULL; 
 
		curl = curl_easy_init(); 
		if( options.debug ) 
			curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L ); 
 
		chunk = slist_append( chunk, "X-CDMI-Specification-Version: %s", CDMI_SPEC_VERSION ); 
		chunk = slist_append( chunk, "Accept: %s", mime[M_DATAOBJECT] ); 
		chunk = slist_append( chunk, "Content-Type: %s", mime[M_OBJECT] ); 
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, chunk );
	} 
 
	CURLcode res; 
	char *data, url[URLSIZE+1];
	long code; 
 
	snprintf( url, URLSIZE, "%s?objectID", path2url( path ) ); 
	curl_easy_setopt(curl, CURLOPT_URL, url ); 
 
	data = download( curl ); 
	if( !data ) 
		return -EIO; 
 
	errno = 0; 
	res = curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &code ); 
	if( res != CURLE_OK ) 
		return errno == 0 ? -EIO : -errno; 
	code = response_code2errno( code ); 
	if( code != SUCCESS ) 
		return -code; 
 
 
	return 0; 
} 
 
