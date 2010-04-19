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

char *path2url( const char *path ) 
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
 
