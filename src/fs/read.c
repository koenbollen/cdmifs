
#include "read.h"

#include "../common.h"
#include "../cdmi.h"
#include "../net.h"
#include "../mime.h"

#include <errno.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <curl/curl.h> 
#include <jansson.h> 
 
int cdmifs_read( 
		const char *path, 
		char *buf, 
		size_t size, 
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
 
	CURLcode res; 
	int ret; 
	char url[URLSIZE+1]; 
	char *data, *ct; 
	long code; 
	json_t *root; 
	json_error_t error;
 
	snprintf( url, URLSIZE, "%s?value:%d-%d;valuerange", path2url( path ), (int)offset, (int)offset+size ); 
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
 
	res = curl_easy_getinfo( curl, CURLINFO_CONTENT_TYPE, &ct ); 
	if( res != CURLE_OK ) 
		return -EIO; 
 
	if( strcmp( ct, mime[M_JSON] ) != 0 ) 
		return -EPROTO; 
 
	root = json_loads( data, &error ); 
	if( !root )
	{ 
		DEBUGV( "error: json error on line %d: %s\n", error.line, error.text ); 
		return -EPROTO; 
	} 
 
	int start=0, end=0;
	ret = sscanf( json_string_value( json_object_get(root,"valuerange") ), "%u-%u", &start, &end ); 
	if( ret != 2 )
	{ 
		json_decref(root); 
		return -EIO; 
	} 
 
	if( !json_is_string( json_object_get(root,"value") ) )
	{ 
		json_decref(root);
		return -EIO; 
	} 
 
	memcpy( buf, json_string_value(json_object_get(root,"value")), end-start ); 
	json_decref(root);
	return end-start; 
} 
 
