
#include "directory.h" 

#include "../common.h"
#include "../cdmi.h" 

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
	char url[512]; 
	CURLcode res; 
	json_t *root, *children; 
	json_error_t error; 
 
	/* TODO: Fix the 1000 limit. */ 
	snprintf( url, 511, "%s?children:0-1000", path2url( path ) ); 
	res = curl_easy_setopt(curl, CURLOPT_URL, url ); 
 
	char *data = download( curl ); 
	if( ! data ) 
		return -EIO; 
 
	root = json_loads( data , &error); 
	if( !root ) 
	{ 
		fprintf( stderr, "error: json error on line %d: %s\n", error.line, error.text ); 
		return -EPROTO; 
	} 
	children = json_object_get( root, "children" ); 
	if( !children ) 
	{ 
		fprintf( stderr, "error: invalid json response\n" ); 
		return -ENOTDIR; 
	} 
 
	filler(buf, ".", NULL, 0); 
	filler(buf, "..", NULL, 0); 
	for(i = 0; i < json_array_size(children); i++) 
	{ 
		if( json_is_string( json_array_get(children, i) ) ) 
		{ 
			filler( buf, json_string_value(json_array_get(children, i)), NULL, 0 );
		} 
	} 
 
	json_decref( root ); 
 
	return 0; 
} 
 
