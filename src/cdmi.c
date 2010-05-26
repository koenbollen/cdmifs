/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "cdmi.h"
#include "common.h"

#include "mime.h"
#include "net.h"
#include "util.h"
#include "b64/cdecode.h"
#include "b64/cencode.h"

#include <arpa/inet.h>
#include <assert.h>
#include <curl/curl.h>
#include <errno.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cdmi_get( cdmi_request_t *request, const char *path )
{
	static CURL *curl = NULL;
	static struct curl_slist *headers = NULL;
	static struct curl_slist *noncdmi_headers = NULL;
	static int cdmitype = 0;

	uint32_t flags = request->flags;

	CURLcode res;
	char *cp, **cpp;
	char *url;
	double contentlength;
	size_t urlsize;
	long code;

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
	if( ISSET(flags, CDMI_CONTAINER) )
	{
		if( cdmitype != CDMI_CONTAINER )
		{
			headers = slist_replace( headers, "Accept: %s", mime[M_CONTAINER] );
			cdmitype = CDMI_CONTAINER;
		}
	}
	else if( ISSET(flags, CDMI_DATAOBJECT) )
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


	if( request->cdmi == 0 && request->length > 0 )
	{
		noncdmi_headers= slist_replace(
				noncdmi_headers, "Range: bytes=%d-%d",
				request->offset, request->offset+request->length-1
			);
	}

	if( request->cdmi )
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );
	else
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, noncdmi_headers );


	/* Build the url from the path and append the field
	 * to it.
	 */
	url = path2url( path );
	urlsize = strlen( url ) + 1;
	if( request->fields != NULL )
		for( cpp = request->fields, cp = *cpp; *cpp; cpp++, cp = *cpp )
			urlsize += strlen( cp ) + 1;

	url = strdup( url );

	if( request->fields != NULL )
	{
		url = realloc( url, urlsize );
		if( url == NULL )
			return -1;
		for( cpp = request->fields, cp = *cpp; *cpp; cpp++, cp = *cpp )
		{
			strcat( url, cpp == request->fields ? "?" : ";" );
			strcat( url, cp );
			if( ISSET(flags, CDMI_SINGLE) )
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
	request->rawdata = download( curl );
	if( !request->rawdata )
		return -1;
	/*puts( data );
	 */

	errno = 0;
	res = curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &code );
	if( res != CURLE_OK )
		return -1;

	code = response_code2errno( code );
	if( code != SUCCESS )
		return rerrno( code );

	res = curl_easy_getinfo( curl, CURLINFO_CONTENT_TYPE, &(request->contenttype) );
	if( res != CURLE_OK || request->contenttype == NULL )
		return -1;
	if( request->cdmi )
	{
		if( cdmitype == CDMI_CONTAINER && strcmp( request->contenttype, mime[M_CONTAINER] ) != 0 )
			return rerrno( ENOTDIR );
		else if( cdmitype == CDMI_DATAOBJECT
				&& strcmp( request->contenttype, mime[M_DATAOBJECT] ) != 0 )
			return rerrno( EISDIR );
	}

	res = curl_easy_getinfo( curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentlength );
	if( res != CURLE_OK )
		return -1;
	request->length = ( contentlength < 0 ) ? 0 : (unsigned int)contentlength;
	request->offset = 0;

	/* Load json data and check if it's correct, if CDMI_SINGLE is set
	 * select the first item.
	 */
	if( request->cdmi || strcmp( request->contenttype, mime[M_JSON] ) == 0 )
	{
		request->root = json_loads( request->rawdata, &(request->json_error) );
		if( request->root == NULL )
		{
			DEBUGV( "error: json error on line %d: %s\n",
					(request->json_error).line,
					(request->json_error).text );
			errno = EPROTO;
			return -1;
		}
	}

	if( ISSET(flags, CDMI_CHECK) )
	{
		if( !json_is_object( request->root ) )
		{
			json_decref( request->root );
			errno = EPROTO;
			return -1;
		}
		for( cpp = request->fields, cp = *cpp; *cpp; cpp++, cp = *cpp )
		{
			char *field = strdup( cp );
			cp = index(field, ':' );
			if( cp != NULL)
				*cp = 0;
			if( json_object_get( request->root, field ) == NULL )
			{
				free( field );
				DEBUGV( "error: missing json element: %s\n", cp );
				errno = EPROTO;
				return -1;
			}
			free( field );
		}
	}

	/* Select the first fields.
	 */
	if( ISSET(flags, CDMI_SINGLE) && ( request->fields != NULL || cdmitype == CDMI_DATAOBJECT ) )
	{
		if( json_is_object( request->root ) )
		{
			json_t *parent = request->root;
			request->root = json_object_get( request->root, request->fields == NULL ? "value" : request->fields[0] );
			if( request->root != NULL )
				json_incref( request->root );
			json_decref( parent );
		}
	}

	return 1;
}


int cdmi_put( cdmi_request_t *request, const char *path )
{
	static CURL *curl = NULL;
	static struct curl_slist *headers = NULL;
	static struct curl_slist *noncdmi_headers = NULL;
	static int cdmitype = 0;

	uint32_t flags = request->flags;

	CURLcode res;
	long code;
	char *data = NULL;

	if( curl == NULL )
	{
		curl = curl_easy_init();
		curl_defaults( curl, 0 );

		headers = slist_append(
				headers,
				"X-CDMI-Specification-Version: %s", CDMI_SPEC_VERSION );

		noncdmi_headers = slist_append( noncdmi_headers, "Expect:" );
	}

#ifndef NDEBUG
	if( request->type != MOVE )
	{
		assert( ISSET(flags, CDMI_CONTAINER) || ISSET(flags, CDMI_DATAOBJECT) );
		assert( request->cdmi == 0 ); /* only supported */
	}

	if( ISSET(flags, CDMI_DATAOBJECT) && !request->cdmi )
	{
		if( request->length > 0 && request->rawdata == NULL )
		{
			fprintf( stderr, "error: no data\n" );
			abort();
		}
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

	if( request->cdmi )
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );
	else
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, noncdmi_headers );

	if( cdmitype == CDMI_CONTAINER )
	{
		if( !request->cdmi )
		{
			noncdmi_headers = slist_replace( noncdmi_headers, "Content-Type:" );
		}
		else
		{
			if( request->type == MOVE )
			{
				json_t *root = json_object();
				json_object_set( root, "move", json_string( path2path(request->src) ) );
				data = json_dumps( root, JSON_INDENT(1) );
				json_decref( root );
				data = realloc( data, strlen(data)+2 );
				strcat( data, "\n" );
				puts( data );
				headers = slist_replace( headers, "Expect:" );
			}
		}
	}
	else
	{
		if( !request->cdmi )
		{
			unsigned int offset = request->offset;
			unsigned int length = request->length;

			noncdmi_headers = slist_replace(
					noncdmi_headers, "Content-Type: %s", request->contenttype
				);

			if( offset != 0 )
			{
				if( length == 0 )
				{
					noncdmi_headers = slist_replace(
							noncdmi_headers, "Content-Range: bytes=%d-",
							(int)offset
						);
				}
				else
				{
					noncdmi_headers = slist_replace(
							noncdmi_headers, "Content-Range: bytes=%d-%d",
							(int)offset, (int)offset+length-1 /* RFC2616 14.35.1, endpos is inclusive, hence the -1 */
						);
				}
			}
			else
			{
				noncdmi_headers = slist_replace( noncdmi_headers, "Content-Range:" );
			}

			//printf( "D in cdmi_put offset: %d; size: %d\n", (int)offset, length );
			curl_easy_setopt( curl, CURLOPT_HTTPHEADER, noncdmi_headers );
		}
		else
		{
			if( request->type == MOVE )
			{
				json_t *root = json_object();
				json_object_set( root, "move", json_string( path2path(request->src) ) );
				data = json_dumps( root, JSON_INDENT(1) );
				json_decref( root );
				data = realloc( data, strlen(data)+2 );
				strcat( data, "\n" );
				puts( data );
				headers = slist_replace( headers, "Expect:" );
			}
		}
	}

	DEBUGV( "info: cdmi_put %s\n", path2url( path ) );
	curl_easy_setopt( curl, CURLOPT_URL, path2url( path ) );

	res = upload( curl, data ? data : request->rawdata, data ? strlen(data) : request->length );
	if( data )
		free( data );
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

int cdmi_delete( cdmi_request_t *request, const char *path )
{
	static CURL *curl;
	//static struct curl_slist *headers = NULL;

	assert( request->cdmi == 0 ); /* only supported in this function */

	if( curl == NULL )
	{
		curl = curl_easy_init();
		curl_defaults( curl, 0 );
	}

	CURLcode res;
	long code;

	DEBUGV( "info: cdmi_delete %s\n", path2url( path ) );
	curl_easy_setopt( curl, CURLOPT_URL, path2url( path ) );

	curl_easy_setopt( curl, CURLOPT_NOBODY, 1L );
	curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, "DELETE" );

	res = curl_easy_perform( curl );
	if( res != CURLE_OK )
	{
		long err;
		curl_easy_getinfo( curl, CURLINFO_OS_ERRNO, &err );
		errno = err == 0 ? EIO : err;
		return -1;
	}

	errno = 0;
	res = curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &code );
	if( res != CURLE_OK )
	{
		errno = EIO;
		return -1;
	}
	errno = response_code2errno( code );
	if( errno != SUCCESS )
	{
		if( code == 409 )
			errno = ENOTEMPTY;
		return -1;
	}

	return 1;
}

void cdmi_free( cdmi_request_t *request )
{
	if( request->root != NULL )
		json_decref( request->root );
}

json_t *getmetadata( const char *path )
{
	int ret;
	cdmi_request_t request;

	memset( &request, 0, sizeof( cdmi_request_t ) );
	request.type = GET;
	request.cdmi = 1;
	request.fields = (char*[]){"metadata",NULL};
	request.flags = CDMI_SINGLE | CDMI_CHECK;

	ret = cdmi_get( &request, path );
	if( ret == -1 )
		return NULL;
	return request.root;
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

char *path2path( const char *path )
{
	static char url[URLSIZE+1];
	if( startswith( path, options.root ) )
		return strcpy( url, path );
	if( path[0] == '/' )
		path++;
	url[0] = 0;
	snprintf( url, URLSIZE, "%s/%s",
			options.root, path );
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
		case 206: /* Partial Content - Part read */
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
	int real_len;
	char *real;

	real_len = b64_dsize( strlen( b64data ) );
	real = alloc( NULL, real_len );
	assert( real != NULL );

	base64_decodestate b64state;
	base64_init_decodestate( &b64state );
	ret = base64_decode_block( b64data, strlen(b64data), real, &b64state );
	assert( ret == real_len );

	objectid_t objectid;
	memset( &objectid, 0, sizeof( objectid_t ) );
	objectid.enterprise = ntohl( *((uint32_t*)real) );
	objectid.length = ntohs( *((uint16_t*)real+4) );
	objectid.crc = ntohs( *((uint16_t*)real+6) );
	memcpy( objectid.data, real+8, objectid.length-8 );

	return objectid;
}

