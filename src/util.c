/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl> 
 * 2010 Sara
 */
#include "util.h" 

#include <assert.h> 
#include <curl/curl.h> 
#include <stdarg.h>
#include <stdlib.h> 
#include <string.h> 
 
struct curl_slist *slist_append( struct curl_slist * list, const char * format, ... ) 
{ 
	size_t size = strlen(format)+512; 
	char buf[size]; 
	struct curl_slist *res; 
	va_list ap; 
 
	va_start(ap, format); 
	vsnprintf( buf, size, format, ap ); 
	va_end(ap);
	res = curl_slist_append( list, buf ); 
	return res; 
} 
 
void *alloc( void *ptr, size_t size ) 
{ 
	assert( size > 0 ); 
	if( ptr ) 
		return realloc( ptr, size ); 
	return malloc( size ); 
} 
 
