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

static const char b64table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline char isbase64( char c );
static inline char b64value( char c );

void *errnull( int err )
{
	errno = err;
	return NULL;
}

const char *mimetype( const void *buffer, size_t length )
{
	static magic_t handle = NULL;
	if( handle == NULL )
	{
		int ret;
		handle = magic_open( MAGIC_MIME_TYPE );
		ret = magic_load( handle, MAGIC_FILE );
		if( ret != 0 )
		{
			fprintf( stderr, "error: %s\n", magic_error( handle ) );
			return MAGIC_DEFAULT;
		}
	}

	const char *mime;
	mime = magic_buffer( handle, buffer, length );
	if( mime == NULL )
		return MAGIC_DEFAULT;
	return mime;
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

size_t b64_size( size_t len )
{
	return (len + 2 - ((len + 2) % 3)) / 3 * 4;
}

int b64_decode( char *dest, const char *src, size_t len )
{
	*dest = 0;
	if(*src == 0)
	{
		return 0;
	}

	char *p = dest;

	do
	{
		char a = b64value(src[0]);
		char b = b64value(src[1]);
		char c = b64value(src[2]);
		char d = b64value(src[3]);
		*p++ = (a << 2) | (b >> 4);
		*p++ = (b << 4) | (c >> 2);
		*p++ = (c << 6) | d;
		if(!isbase64(src[1]))
		{
			p -= 2;
			break;
		}
		else if(!isbase64(src[2]))
		{
			p -= 2;
			break;
		}
		else if(!isbase64(src[3]))
		{
			p--;
			break;
		}
		src += 4;
		while(*src && (*src == 13 || *src == 10)) src++;
	} while( len-= 4 );
	*p = 0;
	return p-dest;
}


static inline char isbase64(char c)
{
	return c && strchr(b64table, c) != NULL;
}


static inline char b64value(char c)
{
	const char *p = strchr(b64table, c);
	if(p) {
		return p-b64table;
	} else {
		return 0;
	}
}

