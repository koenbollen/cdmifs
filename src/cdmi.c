/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#include "cdmi.h"
#include "common.h"
 
#include "util.h" 
 
#include <assert.h> 
#include <errno.h> 
#include <stdio.h> 
#include <string.h> 
#include <arpa/inet.h> 
 
char *path2url( const char *path ) 
{ 
	static char url[512]; 
	if( path[0] == '/' ) 
		path++; 
	sprintf( url, "%s://%s:%s%s/%s", 
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
 
