bin_PROGRAMS = cdmifs
 
AM_CFLAGS = -Wall -D_GNU_SOURCE $(FUSE_CFLAGS) $(JANSSON_CFLAGS) @LIBCURL_CPPFLAGS@

cdmifs_SOURCES = cdmifs.c util.c net.c 
cdmifs_LDFLAGS = @LIBCURL@ 
cdmifs_LDADD = $(FUSE_LIBS) $(JANSSON_LIBS) 
