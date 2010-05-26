/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#ifndef ATTR_H
#define ATTR_H 1

#include <sys/stat.h>

extern int cdmifs_getattr(
		const char *path,
		struct stat *stbuf
	);

extern int cdmifs_utimens( const char *path, const struct timespec tv[2] );

#endif /* !ATTR_H */

