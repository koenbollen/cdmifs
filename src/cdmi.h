/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#ifndef CDMI_H
#define CDMI_H 1

#ifndef SUCCESS
# define SUCCESS 0
#endif

#include <stdint.h>
#include <jansson.h>




#define CDMI_CONTAINER    (1<<1)
#define CDMI_DATAOBJECT   (1<<2)
#define CDMI_CAPABILITIES (1<<5)

#define CDMI_SINGLE      (1<<3)
#define CDMI_CHECK       (1<<4)
#define CDMI_NORESOLVE   (1<<6)

typedef enum _cdmi_request_type
{
	GET, PUT, DELETE, MOVE
} cdmi_request_type_t;

typedef struct _cdmi_request
{
	cdmi_request_type_t type;
	int cdmi;

	const char *src;

	char **fields;

	json_t *root;
	json_error_t json_error;

	const char *rawdata;
	unsigned int offset;
	unsigned int length;
	const char *contenttype;

	uint32_t flags;
	/* CURL *curl; for threadsafe */
} cdmi_request_t;

typedef struct _objectid
{
	uint32_t enterprise;
	uint16_t length;
	uint16_t crc;
	char data[32];
} objectid_t;

extern int cdmi_get( cdmi_request_t *request, const char *path );
extern int cdmi_put( cdmi_request_t *request, const char *path );
extern int cdmi_delete( cdmi_request_t *request, const char *path );
extern void cdmi_free( cdmi_request_t *request );

extern json_t *getcapabilities( const char *path );
extern json_t *getmetadata( const char *path );
extern int setmetadata( const char *path, json_t *metadata );

extern char *path2url( const char *path );
extern char *path2unresolved( const char *path );
extern char *path2path( const char *path );
extern int response_code2errno( long response_code );
extern objectid_t objectid_decode( const char *b64data );

#endif /* !CDMI_H */

