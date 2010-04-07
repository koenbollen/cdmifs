/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 Sara
 */
#ifdef HAVE_CONFIG
# include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "util.h"

#define INITIAL_SIZE 4096

struct buffer 
{ 
	char *data; 
	size_t size; 
	size_t pos; 
}; 
 
static size_t write2buffer( void *ptr, size_t size, size_t nmemb, void *data ); 
 
char *download( const char *url ) 
{ 
	static struct buffer block; 
	static CURL *curl_handle = NULL; 
 
	if( curl_handle == NULL ) 
	{ 
		curl_handle = curl_easy_init(); 
		curl_easy_setopt( curl_handle, CURLOPT_WRITEFUNCTION, write2buffer ); 
	} 
 
 
	block.pos = 0; 
 
	curl_easy_setopt( curl_handle, CURLOPT_WRITEDATA, (void *)&block ); 
	curl_easy_setopt( curl_handle, CURLOPT_URL, url ); 
 
	curl_easy_perform( curl_handle ); 
 
	/* yea, I know I need to clean the curl_handle, not gonna! */ 
 
	return block.data; 
} 
 
static size_t write2buffer( void *ptr, size_t size, size_t nmemb, void *data ) 
{ 
	size_t length = size * nmemb; 
	struct buffer *buf = (struct buffer *)data; 
	if( buf->pos + length + 1 > buf->size ) 
	{
		buf->size = buf->pos + length + 1; 
		buf->data = alloc(buf->data, buf->size ); 
	} 
	if( buf->data ) 
	{ 
		memcpy( buf->data+buf->pos, ptr, length ); 
		buf->pos += length; 
		buf->data[buf->pos] = 0; 
	} 
	return size; 
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
	strcpy(first, download( URL ) ); 
	for( i = 0; i < 100000; i++ ) 
	{
		if( strcmp( first, download( URL ) ) != 0 ) 
		{ 
			fprintf( stderr, "Failed at #%d!\n", i ); 
			break; 
		} 
	} 
	printf( "done.\n" ); 
	return 0; 
} 
 
#endif /* !TEST_CURL */ 
 
