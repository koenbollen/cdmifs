/* cdmifs, a filesystem interface based on the CDMI standard. 
 * Koen Bollen <meneer koenbollen nl> 
 * 2010 Sara
 */ 
#ifdef HAVE_CONFIG 
# include <config.h> 
#endif 
#include "common.h" 
 
#include <assert.h> 
#include <stdio.h> 
#include <curl/curl.h> 
#include <jansson.h> 
 
#include "net.h" 
#include "util.h" 
 
int main( int argc, char *argv[] ) 
{ 
	printf( "cdmifs\n" ); 
	printf( "%s", download( "http://koen.it/ip" ) ); 
 
	const char* json_data = "{\n\t\"name\": \"Koen Bollen\",\n\t\"age\": 23,\n\t\"perms\":\n\t\t[ 3, 2, 1 ]\n}\n";
	printf( "%s", json_data ); 
	json_error_t error; 
	json_t *json; 
	json = json_loads( json_data, &error ); 
	if( !json ) 
	{ 
		fprintf( stderr, "unable to parse json at line %d: %s\n", error.line, error.text ); 
	} 
	else 
	{ 
		assert( json_is_object( json ) ); 
		json_t *name = json_object_get( json, "name" ); 
		if( name && json_is_string( name ) ) 
			printf( "name : %s\n", json_string_value( name ) ); 
		json_t *age = json_object_get( json, "age" ); 
		if( age && json_is_integer( age ) ) 
			printf( "age  : %d\n", json_integer_value( age ) ); 
		json_t *perms = json_object_get( json, "perms" ); 
		if( perms && json_is_array( perms ) ) 
		{ 
			printf( "perms:" ); 
			int i; 
			for( i = 0; i < json_array_size(perms); i++ ) 
			{ 
				json_t *e = json_array_get(perms, i); 
				if( json_is_integer(e) ) 
					printf( " %d", json_integer_value(e) ); 
			} 
			printf( "\n" ); 
		} 
	} 
 
	return 0; 
} 
