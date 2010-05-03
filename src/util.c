/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <curl/curl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE
#endif
#include <time.h>

#include <magic.h>

#ifndef MAGIC_FILE
# define MAGIC_FILE NULL
#endif

inline void *long2pointer( unsigned long l )
{
	return (void*)((uintptr_t)l);
}

void *errnull( int err )
{
	errno = err;
	return NULL;
}

int rerrno( int err )
{
	errno = err;
	return -1;
}

const char *mimetype( const void *buffer, size_t length, const char *def )
{
	static magic_t handle = NULL;
	if( def == NULL )
		def = MAGIC_DEFAULT;
	if( handle == NULL )
	{
		int ret;
		handle = magic_open( MAGIC_MIME_TYPE );
		ret = magic_load( handle, MAGIC_FILE );
		if( ret != 0 )
		{
			fprintf( stderr, "error: %s\n", magic_error( handle ) );
			return def;
		}
	}

	const char *mime;
	mime = magic_buffer( handle, buffer, length );
	if( mime == NULL )
		return def;
	return mime;
}

int startswith( const char *data, const char *prefix )
{
	return strncmp( data, prefix, strlen(prefix) ) == 0;
}

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

struct curl_slist *slist_replace( struct curl_slist * list, const char *format, ... )
{
	size_t size = strlen(format)+512;
	char buf[size], *cp;
	struct curl_slist *item;
	va_list ap;
	int colon;

	va_start(ap, format);
	vsnprintf( buf, size, format, ap );
	va_end(ap);

	if( list == NULL )
		return curl_slist_append( list, buf );

	// Find the : char
	cp = index( buf, ':' );
	if( !cp ) return list;
	colon = (int)(cp-buf);

	for( item = list; item; item = item->next )
	{
		if( strncmp( item->data, buf, colon ) == 0 )
		{
			free(item->data);
			item->data = strdup( buf );
			return list;
		}
	}

	// append if not found:
	return curl_slist_append( list, buf );
}

time_t iso8601_decode( const char *isoformat )
{
	struct tm td;
	memset( &td, 0, sizeof(struct tm) );
	strptime( isoformat, "%FT%T", &td );

	return mktime( &td );
}

void *alloc( void *ptr, size_t size )
{
	assert( size > 0 );
	if( ptr )
		return realloc( ptr, size );
	return malloc( size );
}

size_t b64_dsize( size_t len )
{
	/*
	 * (len + 2 - ((len + 2) % 3)) / 3 * 4;
	 */
	return len;//((len * 4)  / 3) + 10;
}
size_t b64_esize( size_t len )
{
	return len * 2;//(((len) * 3) / 4) + 10;
}

