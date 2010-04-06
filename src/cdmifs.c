
#ifdef HAVE_CONFIG 
# include <config.h>
#endif 
 
#include <stdio.h> 
#include <curl/curl.h>

int main( int argc, char *argv[] ) 
{ 
	printf( "cdmifs\n" ); 
	CURL *curl; 
	CURLcode res; 
 
	curl = curl_easy_init(); 
	if(curl) { 
		curl_easy_setopt(curl, CURLOPT_URL, "koen.it/ip/curltest");
		res = curl_easy_perform(curl); 
 
		curl_easy_cleanup(curl); 
	} 
	return 0; 
} 
