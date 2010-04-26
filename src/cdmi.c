/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "cdmi.h"
#include "common.h"

#include "mime.h"
#include "net.h"
#include "util.h"

#include <arpa/inet.h>
#include <assert.h>
#include <curl/curl.h>
#include <errno.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

json_t *cdmi_get( const char *path, char **fields, int flags )
{
	static CURL *curl = NULL;
	static struct curl_slist *headers = NULL;
	static int cdmitype = 0;

	CURLcode res;
	char *cp, **cpp;
	char *url, *data;
	size_t urlsize;
	long code;
	json_t *root;
	json_error_t jsonerr;


	/* Setup curl object and the header.
	 */
	if( curl == NULL )
	{
		curl = curl_easy_init();
		curl_defaults( curl, 0 );

		headers = slist_append(
				headers,
				"X-CDMI-Specification-Version: %s", CDMI_SPEC_VERSION );
		headers = slist_append( headers, "Accept: %s", mime[M_OBJECT] );
		headers = slist_append( headers, "Content-Type: %s", mime[M_OBJECT] );
	}


	/* See what kind of object is requested and set the
	 * appropriate Accept: header and cdmitype.
	 */
	if( (flags & CDMI_CONTAINER) == CDMI_CONTAINER )
	{
		if( cdmitype != CDMI_CONTAINER )
		{
			headers = slist_replace( headers, "Accept: %s", mime[M_CONTAINER] );
			cdmitype = CDMI_CONTAINER;
		}
	}
	else if( (flags & CDMI_DATAOBJECT) == CDMI_DATAOBJECT )
	{
		if( cdmitype != CDMI_DATAOBJECT )
		{
			headers = slist_replace( headers, "Accept: %s", mime[M_DATAOBJECT] );
			cdmitype = CDMI_DATAOBJECT;
		}
	}
	else if( cdmitype != 0 )
	{
		headers = slist_replace( headers, "Accept: %s", mime[M_OBJECT] );
		cdmitype = 0;
	}


	/* Place heaaders on the curl object if a cdmi request is
	 * not noncdmi.
	 */
	if( (flags & CDMI_NONCDMI) == CDMI_NONCDMI )
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, NULL );
	else
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );


	/* Build the url from the path and append the field
	 * to it.
	 */
	url = path2url( path );
	urlsize = strlen( url ) + 1;
	if( fields != NULL )
		for( cpp = fields, cp = *cpp; *cpp; cpp++, cp = *cpp )
			urlsize += strlen( cp ) + 1;

	url = strdup( url );

	if( fields != NULL )
	{
		url = realloc( url, urlsize );
		if( url == NULL )
			return NULL;
		for( cpp = fields, cp = *cpp; *cpp; cpp++, cp = *cpp )
		{
			strcat( url, cpp == fields ? "?" : ";" );
			strcat( url, cp );
			if( (flags & CDMI_SINGLE) == CDMI_SINGLE )
				break;
		}
	}


	/* Set url to the curl object, download the data and
	 * do error checks.
	 */
	DEBUGV( "info: cdmi_get %s\n", url );
	curl_easy_setopt( curl, CURLOPT_URL, url );
	free( url ); /* curl's setopt does a strdup. */

	errno = 0;
	data = download( curl );
	if( !data )
		return errnull( errno == 0 ? EIO : errno );
	//puts( data );

	errno = 0;
	res = curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &code );
	if( res != CURLE_OK )
		return errnull( EIO );
	code = response_code2errno( code );
	if( code != SUCCESS )
		return errnull( code );

	res = curl_easy_getinfo( curl, CURLINFO_CONTENT_TYPE, &cp );
	if( res != CURLE_OK || cp == NULL )
		return errnull( EPROTO );
	if( (flags & CDMI_NONCDMI) != CDMI_NONCDMI )
	{
		if( cdmitype == CDMI_CONTAINER && strcmp( cp, mime[M_CONTAINER] ) != 0 )
			return errnull( ENOTDIR );
		else if( cdmitype == CDMI_DATAOBJECT
				&& strcmp( cp, mime[M_DATAOBJECT] ) != 0 )
			return errnull( EISDIR );
	}

	/* Load json data and check if it's correct, if CDMI_SINGLE is set
	 * select the first item.
	 */
	if( (flags & CDMI_NONCDMI) == CDMI_NONCDMI && strcmp( cp, mime[M_JSON] ) != 0 )
	{
		root = json_object();
		json_object_set( root, "value", json_string( data ) );
	}
	else
	{
		root = json_loads( data, &jsonerr );
		if( root == NULL )
		{
			DEBUGV( "error: json error on line %d: %s\n", jsonerr.line, jsonerr.text );
			return errnull( EPROTO );
		}
	}

	if( (flags & CDMI_CONTENTTYPE) == CDMI_CONTENTTYPE && json_is_object(root) )
	{
		json_object_set( root, "_contenttype", json_string( cp ) );
	}

	if( (flags & CDMI_CHECK) == CDMI_CHECK )
	{
		if( !json_is_object( root ) )
		{
			json_decref( root );
			return errnull( EPROTO );
		}
		for( cpp = fields, cp = *cpp; *cpp; cpp++, cp = *cpp )
		{
			char *field = strdup( cp );
			cp = index(field, ':' );
			if( cp != NULL)
				*cp = 0;
			if( json_object_get( root, field ) == NULL )
			{
				free( field );
				json_decref( root );
				DEBUGV( "error: missing json element: %s\n", cp );
				return errnull( EPROTO );
			}
			free( field );
		}
	}

	/* Select the first fields.
	 */
	if( (flags & CDMI_SINGLE) == CDMI_SINGLE && ( fields != NULL || cdmitype == CDMI_DATAOBJECT ) )
	{
		if( json_is_object( root ) )
		{
			json_t *parent = root;
			root = json_object_get( root, fields == NULL ? "value" : *fields );
			if( root != NULL )
				json_incref( root );
			json_decref( parent );
		}
	}

	return root;
}


int cdmi_put( const char *path, json_t *data, int flags )
{
	static CURL *curl = NULL;
	static struct curl_slist *headers = NULL;
	static struct curl_slist *noncdmi_headers = NULL;
	static int cdmitype = 0;

	CURLcode res;
	long code;
	const char *rawdata = NULL;
	size_t size = 0;

	if( curl == NULL )
	{
		curl = curl_easy_init();
		curl_defaults( curl, 0 );

		headers = slist_append(
				headers,
				"X-CDMI-Specification-Version: %s", CDMI_SPEC_VERSION );

		noncdmi_headers = slist_append( noncdmi_headers, "Expect:" );
	}

	assert( ISSET(flags, CDMI_CONTAINER) || ISSET(flags, CDMI_DATAOBJECT) );
	assert( ISSET(flags, CDMI_NONCDMI) ); /* only supported */

#ifndef NDEBUG
	if( ISSET(flags, CDMI_DATAOBJECT) && ISSET(flags, CDMI_NONCDMI) )
	{
		if( !json_is_object( data ) )
			abort();
		if( !json_is_string( json_object_get( data, "mimetype" ) ) )
			abort();
		if( !json_is_string( json_object_get( data, "value" ) ) )
			abort();
	}
#endif /* !NDEBUG */

	if( ISSET(flags, CDMI_CONTAINER) && cdmitype != CDMI_CONTAINER )
	{
		headers = slist_replace( headers, "Accept: %s", mime[M_CONTAINER] );
		headers = slist_replace( headers, "Content-Type: %s", mime[M_CONTAINER] );
		cdmitype = CDMI_CONTAINER;
	}
	else if( ISSET(flags, CDMI_DATAOBJECT) && cdmitype != CDMI_DATAOBJECT )
	{
		headers = slist_replace( headers, "Accept: %s", mime[M_DATAOBJECT] );
		headers = slist_replace( headers, "Content-Type: %s", mime[M_DATAOBJECT] );
		cdmitype = CDMI_DATAOBJECT;
	}

	if( ISSET(flags, CDMI_NONCDMI) )
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, noncdmi_headers );
	else
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );

	DEBUGV( "info: cdmi_put %s\n", path2url( path ) );
	curl_easy_setopt( curl, CURLOPT_URL, path2url( path ) );

	if( cdmitype == CDMI_CONTAINER )
	{
		if( ISSET(flags, CDMI_NONCDMI) )
		{
			noncdmi_headers = slist_replace( noncdmi_headers, "Content-Type:" );
		}
		else
		{
			/* stub */
		}
	}
	else
	{
		if( ISSET(flags, CDMI_NONCDMI) )
		{
			json_t *value = json_object_get(data, "value");
			json_t *mimetype = json_object_get(data, "mimetype");

			size = json_integer_value( json_object_get(data, "length") );
			if( size < 1 )
				size = strlen(json_string_value( value ));

			noncdmi_headers = slist_replace(
					noncdmi_headers, "Content-Type: %s",
					json_string_value(mimetype)
				);

			rawdata = json_string_value( value );

			curl_easy_setopt( curl, CURLOPT_HTTPHEADER, noncdmi_headers );
		}
		else
		{
			/* stub */
		}
	}

	res = upload( curl, rawdata, size );
	if( res != CURLE_OK )
	{
		if( errno == 0 )
			errno = EIO;
		return -1;
	}

	errno = 0;
	res = curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &code );
	if( res != CURLE_OK )
	{
		errno = EIO;
		return -1;
	}
	code = response_code2errno( code );
	if( code != SUCCESS )
	{
		errno = code;
		return -1;
	}

	return 1;
}

char *path2url( const char *path )
{
	static char url[URLSIZE+1];
	if( path[0] == '/' )
		path++;
	snprintf( url, URLSIZE, "%s://%s:%s%s/%s",
			options.ssl?"https":"http",
			options.host, options.port,
			options.root, path
		);
	return url;
}

int response_code2errno( long response_code )
{
	switch( response_code )
	{
		case 200: /* OK Resource retrieved successfully */
			return SUCCESS;
		case 201: /* Created - Resource created successfully */
			return SUCCESS;
		case 202: /* Accepted - Long running operation accepted for processing*/
			return EINPROGRESS;
		case 204: /* No Content - Operation successful, no data */
			return SUCCESS;
		case 400: /* Bad Request - Missing or invalid request contents */
			return EPROTO;
		case 401: /* Unauthorized - Invalid authentication/authorization credentials */
			return EPERM;
		case 403: /* Forbidden - This user is not allowed to perform this request */
			return EACCES;
		case 404: /* Not Found - Requested resource not found */
			return ENOENT;
		case 405: /* Method Not Allowed - Requested HTTP verb not allowed on this resource */
			return ENOTSUP;
		case 406: /* Not Acceptable - No content type can be produced at this URI that matches the request */
			return ENOENT;
		case 409: /* Conflict - The operation conflicts with a non-CDMI access protocol lock, or could cause a state transition error on the server. */
			return EEXIST;
		case 500: /* Internal Server - Error An unexpected vendor specific error */
			return EREMOTEIO;
		case 501: /* Not Implemented - A CDMI operation or metadata value was attempted that is not implemented. */
			return ENOSYS;
		default:
			return EIO;
	}
}

objectid_t objectid_decode( const char *b64data )
{
	int ret;
	size_t real_len;
	char *real;

	real_len = b64_size( strlen( b64data ) );
	real = alloc( NULL, real_len );
	assert( real != NULL );

	ret = b64_decode( real, b64data, strlen(b64data) );
	assert( ret == real_len );

	objectid_t objectid;
	memset( &objectid, 0, sizeof( objectid_t ) );
	objectid.enterprise = ntohl( *((uint32_t*)real) );
	objectid.length = ntohs( *((uint16_t*)real+4) );
	objectid.crc = ntohs( *((uint16_t*)real+6) );
	memcpy( objectid.data, real+8, objectid.length-8 );

	return objectid;
}

