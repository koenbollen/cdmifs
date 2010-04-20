/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 Sara
 */
#ifdef HAVE_CONFIG
# include <config.h>
#endif

#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "util.h"
#include "common.h"

#define INITIAL_SIZE 4096

struct buffer
{
	char *data;
	size_t size;
	size_t pos;
};

static size_t write2buffer( void *ptr, size_t size, size_t nmemb, void *data );

char *download( CURL *curl )
{
	static struct buffer block;
	CURLcode res;

	block.pos = 0;

	curl_easy_setopt( curl, CURLOPT_USERAGENT, USERAGENT );
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)&block ); 
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, write2buffer );
 
	res = curl_easy_perform( curl); 
	if( res != CURLE_OK ) 
	{ 
		long err; 
		curl_easy_getinfo( curl, CURLINFO_OS_ERRNO, &err ); 
		errno = err; 
		return NULL; 
	} 
 
	block.data[block.pos] = 0; 
 
	return block.data; 
} 
 
char *simple( const char *url ) 
{ 
	static CURL *curl_handle = NULL; 
 
	if( curl_handle == NULL ) 
		curl_handle = curl_easy_init(); 
 
	curl_easy_setopt( curl_handle, CURLOPT_URL, url ); 
 
	char *data = download( curl_handle ); 
 
	/* yea, I know I need to clean the curl_handle, not gonna! */ 
 
	return data; 
} 
 
static size_t write2buffer( void *ptr, size_t size, size_t nmemb, void *stream ) 
{ 
	size_t length = size * nmemb; 
	struct buffer *buf = (struct buffer *)stream; 
	//fprintf( stderr, "pos: %d\nsiz: %d\nlen: %d\n", buf->pos, buf->size, length ); 
	if( buf->pos + length + 1 > buf->size ) 
	{ 
		buf->size = buf->pos + length + 1; 
		buf->data = alloc(buf->data, buf->size ); 
	} 
	if( buf->data ) 
	{ 
		memcpy( buf->data+buf->pos, ptr, length ); 
		buf->pos += length; 
	} 
	return length; 
} 
 
#ifdef TEST_NET_C 
 
#include <stdio.h> 
#include <unistd.h> 
 
#ifndef URL 
# define URL "http://localhost/" 
#endif 
 
int main( int argc, char *argv[] ) 
{ 
	int i; 
	char first[512];
	strncmp(first, simple( URL ), 511 ); 
	for( i = 0; i < 0; i++ ) 
	{
		if( strncmp( first, simple( URL ), 511 ) != 0 ) 
		{ 
			fprintf( stderr, "Failed at #%d!\n", i ); 
			break; 
		} 
	} 
	printf( "done.\n" ); 
	return 0; 
} 
 
#endif /* !TEST_CURL */ 
 
