/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl> 
 * 2010 Sara
 */
#ifndef UTIL_H 
#define UTIL_H 1 
 
#include <sys/types.h> 
#include <curl/curl.h> 

extern void *alloc( void *ptr, size_t size ) ; 
extern struct curl_slist *slist_append( struct curl_slist * list, const char * format, ... ); 
 
#endif /* !UTIL_H */ 
