SUBDIRS = src
dist_doc_DATA = README 

CDMI_URI="cdmi://localhost/data"
MOUNT_POINT=/tmp/cdmi 

mount: all 
	@mkdir -p ${MOUNT_POINT} 
	-fusermount -q -u ${MOUNT_POINT} 
	./src/cdmifs -s ${CDMI_URI} ${MOUNT_POINT} 
 
umount: 
	fusermount -u ${MOUNT_POINT} 
	@rmdir ${MOUNT_POINT} 
 
run: all 
	@mkdir -p ${MOUNT_POINT} 
	-fusermount -q -u ${MOUNT_POINT} 
	./src/cdmifs -f -s -o cdmifs_debug ${CDMI_URI} ${MOUNT_POINT} 
	@rmdir ${MOUNT_POINT} 
 
