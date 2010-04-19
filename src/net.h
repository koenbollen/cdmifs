/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl> 
 * 2010 Sara
 */
#ifndef NET_H 
#define NET_H 1 

#include <curl/curl.h>
 
extern char *download( CURL *curl );
extern char *simple( const char *url ); 
extern char *request( const char *url, char **headers, char *data ); 
 
#endif /* !NET_H */ 
 
