/* cdmifs, a filesystem interface based on the cdmi standard. 
 * koen bollen <meneer koenbollen nl> 
 * 2010 sara
 */ 
#ifndef CDMI_H 
#define CDMI_H 1
 
#ifndef SUCCESS 
# define SUCCESS 0
#endif 
 
#include <stdint.h>
 
typedef struct _objectid 
{ 
	uint32_t enterprise; 
	uint16_t length; 
	uint16_t crc; 
	char data[32]; 
} objectid_t; 
 
extern char *path2url( const char *path ); 
extern int response_code2errno( long response_code ); 
extern objectid_t objectid_decode( const char *b64data ); 
 
#endif /* !CDMI_H */ 
 
