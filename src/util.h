/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl> 
 * 2010 GPL
 */
#ifndef UTIL_H 
#define UTIL_H 1 
 
#include <sys/types.h> 
#include <curl/curl.h> 
 
extern void *alloc( void *ptr, size_t size ) ; 
extern struct curl_slist *slist_append( struct curl_slist * list, const char * format, ... ); 
extern int b64_decode( char *dest, const char *src, size_t len ); 
extern size_t b64_size( size_t len ); 
extern time_t iso8601_decode( const char *isoformat ); 
 
#endif /* !UTIL_H */ 
