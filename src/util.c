/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl> 
 * 2010 Sara
 */
#include "util.h" 

#include <assert.h>
#include <stdlib.h>
 
void *alloc( void *ptr, size_t size ) 
{ 
	assert( size > 0 ); 
	if( ptr ) 
		return realloc( ptr, size ); 
	return malloc( size ); 
} 
 
