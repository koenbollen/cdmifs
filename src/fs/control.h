/* cdmifs, a filesystem interface based on the CDMI standard.
 * Koen Bollen <meneer koenbollen nl>
 * 2010 GPL
 */
#ifndef CONTROL_H
#define CONTROL_H 1

#include <sys/types.h>
#include "../common.h"

extern int cdmifs_open(
		const char *path,
		struct fuse_file_info *fi
	);

extern int cdmifs_create(
		const char *path,
		mode_t mode,
		struct fuse_file_info *fi
	);

extern int cdmifs_release(
		const char *path,
		struct fuse_file_info *fi
	);

extern int cdmifs_unlink( const char *path );


#endif /* !CONTROL_H */

