/* cdmifs, a filesystem interface based on the cdmi standard. 
 * koen bollen <meneer koenbollen nl> 
 * 2010 sara
 */ 
#ifndef CDMI_H 
#define CDMI_H 1
 
extern char *cdmi_path2url( const char *path ); 
extern char ** cdmi_listpath( const char *path, int *count ); 
 
#endif /* !CDMI_H */ 
 
