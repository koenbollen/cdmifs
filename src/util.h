/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#ifndef UTIL_H
#define UTIL_H 1

#ifdef HAVE_CONFIG
# include <config.h>
#endif

#include <sys/types.h>
#include <curl/curl.h>


#define ISSET( flags, flag ) ((flags & flag) == flag)

#ifndef MAGIC_DEFAULT
# define MAGIC_DEFAULT "application/octet-stream"
#endif

extern void *alloc( void *ptr, size_t size ) ;

/**
 * Set the errno to the given errno and return NULL.
 */
extern void *errnull( int err ) ;

extern struct curl_slist *slist_append( struct curl_slist * list, const char * format, ... );
struct curl_slist *slist_replace( struct curl_slist * list, const char *format, ... );
extern int b64_decode( char *dest, const char *src, size_t len );
extern size_t b64_size( size_t len );
extern time_t iso8601_decode( const char *isoformat );

extern const char *mimetype( const void *buffer, size_t length );

#endif /* !UTIL_H */
