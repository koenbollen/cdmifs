/* cdmifs, a filesystem interface based on the CDMI standard. 
 * Koen Bollen <meneer koenbollen nl> 
 * 2010 Sara
 */ 
#ifndef COMMON_H 
#define COMMON_H 1 
 
#ifdef HAVE_CONFIG 
# include <config.h>
#endif 
 
#define APP_NAME "cdmifs" 
#define APP_VERSION "0.0" 
 
#ifndef USERAGENT 
# define USERAGENT APP_NAME"/"APP_VERSION 
#endif
 
/*#define USE_SSL 1*/ 
 
#ifndef FUSE_USE_VERSION 
# define FUSE_USE_VERSION 26 
#endif
#include <fuse.h> 
#include <fuse_opt.h> 
 
struct options 
{ 
	char *uri; 
	char *host; 
	char *port; 
	int ssl; 
	char *root; 
	int debug; 
}; 
extern struct options options; 
 
#define DEFAULT_PORT "2364" 
 
#define DEBUG(format, args...) do { if(options.debug) fprintf( stderr, format, args ); } while(0)
 
#endif /* ! COMMON_H */ 
