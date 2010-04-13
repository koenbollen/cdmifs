
#ifndef OPERATIONS_H 
#define OPERATIONS_H 1 

#include "attr.h" 
#include "control.h" 
#include "directory.h" 
#include "read.h" 
 
struct fuse_operations cdmifs_operations = { 
	 .getattr  = cdmifs_getattr, 
	 .readdir  = cdmifs_readdir, 
	 .open     = cdmifs_open, 
	 .read     = cdmifs_read, 
}; 
 
#endif /* !OPERATIONS_H */ 
 
