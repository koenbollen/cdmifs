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



#define CDMI_NONCDMI     (1<<0)

#define CDMI_CONTAINER   (1<<1)
#define CDMI_DATAOBJECT  (1<<2)

#define CDMI_SINGLE      (1<<3)
#define CDMI_CHECK       (1<<4)
#define CDMI_CONTENTTYPE (1<<5)


 
typedef struct _objectid 
{ 
	uint32_t enterprise; 
	uint16_t length; 
	uint16_t crc; 
	char data[32]; 
} objectid_t; 
 
extern json_t *cdmi_request( const char *path, char **fields, int flags );
extern char *path2url( const char *path ); 
extern int response_code2errno( long response_code ); 
extern objectid_t objectid_decode( const char *b64data ); 
 
#endif /* !CDMI_H */ 
 
