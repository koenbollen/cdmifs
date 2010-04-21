/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "attr.h"

#include "../cdmi.h"
#include "../common.h"
#include "../mime.h"
#include "../net.h"
#include "../util.h"

#include "directory.h"

#include <curl/curl.h>
#include <errno.h>
#include <fcntl.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int parse_metadata( json_t *metadata, struct stat *stbuf );

int cdmifs_getattr(
		const char *path,
		struct stat *stbuf )
{
	static CURL *curl = NULL;
	if( curl == NULL )
	{
		struct curl_slist *chunk = NULL;

		curl = curl_easy_init();
		if( options.debug )
			curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L );

		chunk = slist_append( chunk, "X-CDMI-Specification-Version: %s", CDMI_SPEC_VERSION );
		chunk = slist_append( chunk, "Accept: %s", mime[M_OBJECT] );
		chunk = slist_append( chunk, "Content-Type: %s", mime[M_OBJECT] );
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, chunk );
	}

	CURLcode res;
	int i, ret;
	long code;
	char *data, *ct, url[URLSIZE+1];
	json_t *root;
	json_error_t error;

	memset( stbuf, 0, sizeof(struct stat) );

	snprintf( url, URLSIZE, "%s?objectID;metadata", path2url( path ) );
	curl_easy_setopt(curl, CURLOPT_URL, url );

	data = download( curl );
	if( !data )
		return -EIO;

	errno = 0;
	res = curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &code );
	if( res != CURLE_OK )
		return errno == 0 ? -EIO : -errno;
	code = response_code2errno( code );
	if( code != SUCCESS )
		return -code;

	res = curl_easy_getinfo( curl, CURLINFO_CONTENT_TYPE, &ct );
	if( res != CURLE_OK )
		return -EIO;
	if( strcmp( ct, mime[M_CONTAINER] ) == 0 )
		stbuf->st_mode = S_IFDIR;
	else if( strcmp( ct, mime[M_DATAOBJECT] ) == 0 )
		stbuf->st_mode = S_IFREG;
	else
		return -EPROTO;

	root = json_loads( data, &error );
	if( !root )
	{
		DEBUGV( "error: json error on line %d: %s\n", error.line, error.text );
		return -EPROTO;
	}
	if( !json_is_string(json_object_get(root, "objectID")) )
	{ 
		DEBUG( "error: invalid json object\n" );
		return -EPROTO;
	} 

	if( parse_metadata( json_object_get(root, "metadata"), stbuf ) < 0 ) 
	{ 
		DEBUG( "error: unable to parse metadata\n" );
		return -EPROTO; 
	} 

	json_decref( root ); 
 
	if( S_ISDIR( stbuf->st_mode ) ) 
	{ 
		stbuf->st_mode |= 0755;
		ret = getchildren( curl, path, &root ); 
		if( ret < 0 ) 
			return ret;

		stbuf->st_nlink = 2; 
		for(i = 0; i < json_array_size(root); i++)
		{ 
			if( json_is_string( json_array_get(root, i) ) )
			{ 
				const char *cp = json_string_value(json_array_get(root, i)); 
				if( cp[strlen(cp)-1] == '/' )
					(stbuf->st_nlink)++; 
			} 
		} 
 
		json_decref( root ); 
 
		return 0; 
	} 
	else 
	{ 
		stbuf->st_mode |= 0444; 
		stbuf->st_nlink = 1; 
 
		return 0; 
	} 
 
	return -ENOENT; 
} 
 
static int parse_metadata( json_t *metadata, struct stat *stbuf ) 
{ 
	if( !json_is_object( metadata ) ) 
		return -1; 

	if( json_is_integer( json_object_get(metadata, "cdmi_size") ) ) 
		stbuf->st_size = json_integer_value( json_object_get(metadata, "cdmi_size") ); 

	if( json_is_string( json_object_get(metadata, "cdmi_ctime") ) ) 
	{
		stbuf->st_ctime = iso8601_decode( 
				json_string_value( json_object_get(metadata, "cdmi_ctime") ) 
			); 
	} 
 
	if( json_is_string( json_object_get(metadata, "cdmi_atime") ) ) 
	{
		stbuf->st_atime = iso8601_decode( 
				json_string_value( json_object_get(metadata, "cdmi_atime") ) 
			); 
	} 

	if( json_is_string( json_object_get(metadata, "cdmi_mtime") ) ) 
	{
		stbuf->st_mtime = iso8601_decode( 
				json_string_value( json_object_get(metadata, "cdmi_mtime") )
			); 
	} 

	return 0; 
} 
 

