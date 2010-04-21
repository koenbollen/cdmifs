/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "directory.h"

#include "../common.h"
#include "../cdmi.h"
#include "../net.h"

#include <curl/curl.h>
#include <errno.h>
#include <fcntl.h>
#include <jansson.h>
#include <net.h>
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
	static CURL *curl = NULL; 
	if( curl == NULL ) 
	{ 
		curl = curl_easy_init(); 
		if( options.debug ) 
			curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L ); 
	} 
 
	int i; 
	const char *cp; 
	char childbuf[PATH_MAX]; 
	json_t *children; 
 
	i = getchildren( curl, path, &children ); 
	if( i < 0 || !children ) 
		return i; 
 
	filler(buf, ".", NULL, 0); 
	filler(buf, "..", NULL, 0); 
	for(i = 0; i < json_array_size(children); i++) 
	{ 
		if( json_is_string( json_array_get(children, i) ) ) 
		{ 
			cp = json_string_value(json_array_get(children, i)); 
			if( cp[strlen(cp)-1] == '/' ) 
			{ 
 				strcpy( childbuf, cp ); 
				childbuf[strlen(cp)-1] = 0; 
				filler( buf, childbuf, NULL, 0 ); 
			}
			else
				filler( buf, cp, NULL, 0 ); 
		} 
	} 
 
	json_decref( children ); 
 
	return 0; 
} 
 
int getchildren( CURL *curl, const char *path, json_t **result ) 
{ 
	char url[URLSIZE+1]; 
	CURLcode res; 
	json_t *root, *children; 
	json_error_t error; 

	*result = NULL; 
 
	/* TODO: Fix the 1000 limit. */ 
	snprintf( url, URLSIZE, "%s?children:0-1000", path2url( path ) ); 
	res = curl_easy_setopt(curl, CURLOPT_URL, url ); 

	/* TODO: Check res and response_code */ 
 
	char *data = download( curl ); 
	if( ! data ) 
		return -EIO; 

	long code;
	errno = 0;
	res = curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &code );
	if( res != CURLE_OK )
		return errno == 0 ? -EIO : -errno;
	code = response_code2errno( code );
	if( code != SUCCESS )
		return -code;
 
	root = json_loads( data , &error); 
	if( !root ) 
	{ 
		DEBUGV( "error: json error on line %d: %s\n", error.line, error.text ); 
		return -EPROTO; 
	} 
	children = json_object_get( root, "children" ); 
	if( !children ) 
	{ 
		DEBUG( "error: invalid json response\n" ); 
		json_decref( root ); 
		return -ENOTDIR; 
	} 
 
	*result = children;
 
	return 0; 
} 

