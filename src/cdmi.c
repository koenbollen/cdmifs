/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 Sara
 */
#include "cdmi.h"
#include "common.h"

#include "net.h"

#include <curl/curl.h>
#include <errno.h>
#include <jansson.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

char *cdmi_path2url( const char *path )
{
	static char url[512];
	if( path[0] == '/' )
		path++;
	sprintf( url, "%s://%s:%s%s%s",
			options.ssl?"https":"http",
			options.host, options.port,
			options.root, path
		);
	return url;
}

char ** cdmi_listpath( const char *path, int *count )
{
	static CURL *curl = NULL;
	CURLcode res;
	int lcount = *count;
	if( lcount < 1 )
		lcount = 1000;
	if( curl == NULL )
	{
		curl = curl_easy_init();
		if( options.debug )
			curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L );
	}
	char url[512];
	snprintf( url, 511, "%s?children:0-%d", cdmi_path2url( path ), lcount );
	res = curl_easy_setopt(curl, CURLOPT_URL, url );

	*count = 0;

	char *data = download( curl ); 
	if( ! data ) 
	{ 
		errno = EIO; 
		return NULL; 
	} 
	json_t *root, *children; 
	json_error_t error; 
	root = json_loads( data , &error); 
	if( !root ) 
	{ 
		fprintf( stderr, "error: json error on line %d: %s\n", error.line, error.text ); 
		errno = EPROTO; 
		return NULL; 
	} 
	children = json_object_get( root, "children" ); 
	if( !children ) 
	{ 
		fprintf( stderr, "error: invalid json response\n" ); 
		errno = EPROTO; 
		return NULL; 
	} 
	char **result = calloc( sizeof(char*), json_array_size(children) ); 
	int i; 
	for(i = 0; i < json_array_size(children); i++) 
	{ 
		if( json_is_string( json_array_get(children, i) ) ) 
		{ 
			result[(*count)] = strdup( json_string_value(json_array_get(children, i) ) ); 
			(*count)++; 
		} 
	} 
 
	json_decref( root ); 
 
	return result; 
} 
 
int cdmi_stat( const char *path, struct stat *buf ) 
{ 
	errno = ENOSYS;
	return -1; 
} 
 
#ifdef TEST_CDMI_C 
 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
 
struct options options; 
 
int main( int argc, char *argv[] ) 
{ 
	options.uri = "cdmi://localhost/data"; 
	options.host = "localhost"; 
	options.port = DEFAULT_PORT; 
	options.root = "/data/"; 
	options.debug = 1; 
 
	printf( "uri: %s\n", options.uri ); 
 
	int i, count=0; 
	char ** childs = cdmi_listpath( "/", &count ); 
	if( childs == NULL ) 
	{ 
		perror( "listpath" ); 
		return 1; 
	} 
	for( i = 0; i < count; i++ ) 
	{
		printf( " > %s\n", childs[i] ); 
		free( childs[i] );
	} 
	free( childs ); 
	return 0;
} 
 
#endif /* TEST_CDMI_C */ 

