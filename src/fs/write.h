/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#ifndef WRITE_H
#define WRITE_H 1

#include "../common.h"
#include <sys/types.h>

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.	 An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */
extern int cdmifs_write(
		const char *path,
		const char *data, size_t size, off_t offset,
		struct fuse_file_info *fi );


/** Change the size of a file */
extern int cdmifs_truncate( const char *path, off_t offset );

#endif /* !WRITE_H */


