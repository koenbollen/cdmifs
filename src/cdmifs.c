/* cdmifs, a filesystem interface based on the CDMI standard. 
 * Koen Bollen <meneer koenbollen nl> 
 * 2010 Sara
 */ 
#ifdef HAVE_CONFIG 
# include <config.h> 
#endif 
#include "common.h" 
 
#include <stdio.h> 
#include <curl/curl.h> 

#include "util.h"
 
int main( int argc, char *argv[] ) 
{ 
	printf( "cdmifs\n" ); 
	printf( "%s", download( "http://koen.it/ip" ) ); 
	return 0; 
} 
