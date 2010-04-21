/* cdmifs, a filesystem interface based on the CDMI standard. 
 * Koen Bollen <meneer koenbollen nl> 
 * 2010 GPL
 */ 
#ifndef MIME_H 
#define MIME_H
 
#define M_JSON       0
#define M_OBJECT     1
#define M_DATAOBJECT 2 
#define M_CONTAINER  3 

static char *mime[] = { 
	"text/json", 
	"application/vnd.org.snia.cdmi.object+json", 
	"application/vnd.org.snia.cdmi.dataobject+json", 
	"application/vnd.org.snia.cdmi.container+json",
	NULL 
}; 
 
#endif /* !MIME_H */ 
 
