/* 
 *   $Id: mpio.h,v 1.1 2001/12/03 23:42:23 ashton Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

/* user include file for MPI-IO programs */

#ifndef MPIO_INCLUDE
#define MPIO_INCLUDE

#include "mpi.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define ROMIO_VERSION 102      /* Version 1.0.2 */

/* define MPI-IO datatypes and constants */

typedef struct ADIOI_FileD *MPI_File;

/* no generalized requests as yet */
typedef struct ADIOI_RequestD *MPIO_Request;  

#ifdef ROMIO_NTFS
#ifdef HAVE_INT64
typedef __int64 MPI_Offset;
#else
typedef long MPI_Offset;
#endif
#else
typedef long long MPI_Offset;
#endif

#ifndef NEEDS_MPI_FINT

#endif
#ifdef NEEDS_MPI_FINT
typedef int MPI_Fint; 
#endif

#ifndef HAVE_MPI_INFO
#define HAVE_MPI_INFO
#endif
#ifndef HAVE_MPI_INFO
  typedef struct MPIR_Info *MPI_Info;
# define MPI_INFO_NULL         ((MPI_Info) 0)
# define MPI_MAX_INFO_KEY       255
# define MPI_MAX_INFO_VAL      1024
#endif

#define MPI_MODE_RDONLY              2  /* ADIO_RDONLY */
#define MPI_MODE_RDWR                8  /* ADIO_RDWR  */
#define MPI_MODE_WRONLY              4  /* ADIO_WRONLY  */
#define MPI_MODE_CREATE              1  /* ADIO_CREATE */ 
#define MPI_MODE_EXCL               64  /* ADIO_EXCL */
#define MPI_MODE_DELETE_ON_CLOSE    16  /* ADIO_DELETE_ON_CLOSE */
#define MPI_MODE_UNIQUE_OPEN        32  /* ADIO_UNIQUE_OPEN */
#define MPI_MODE_APPEND            128  /* ADIO_APPEND */
#define MPI_MODE_SEQUENTIAL        256  /* ADIO_SEQUENTIAL */

#define MPI_DISPLACEMENT_CURRENT   -54278278

#define MPI_FILE_NULL           ((MPI_File) 0)
#define MPIO_REQUEST_NULL       ((MPIO_Request) 0)

#define MPI_SEEK_SET            600
#define MPI_SEEK_CUR            602
#define MPI_SEEK_END            604

#define MPI_MAX_DATAREP_STRING  128

#ifndef HAVE_MPI_DARRAY_SUBARRAY
#define HAVE_MPI_DARRAY_SUBARRAY
#endif
#ifndef HAVE_MPI_DARRAY_SUBARRAY
#  define MPI_ORDER_C             56
#  define MPI_ORDER_FORTRAN       57
#  define MPI_DISTRIBUTE_BLOCK    121
#  define MPI_DISTRIBUTE_CYCLIC   122
#  define MPI_DISTRIBUTE_NONE     123
#  define MPI_DISTRIBUTE_DFLT_DARG -49767
#endif


/* MPI-IO function prototypes */

/* The compiler must support ANSI C style prototypes, otherwise 
   long long constants (e.g. 0) may get passed as ints. */

#ifndef HAVE_PRAGMA_HP_SEC_DEF

/* Section 9.2 */
int MPI_File_open(MPI_Comm comm, char *filename, int amode, 
                          MPI_Info info, MPI_File *fh);
int MPI_File_close(MPI_File *fh);
int MPI_File_delete(char *filename, MPI_Info info);
int MPI_File_set_size(MPI_File fh, MPI_Offset size);
int MPI_File_preallocate(MPI_File fh, MPI_Offset size);
int MPI_File_get_size(MPI_File fh, MPI_Offset *size);
int MPI_File_get_group(MPI_File fh, MPI_Group *group);
int MPI_File_get_amode(MPI_File fh, int *amode);
int MPI_File_set_info(MPI_File fh, MPI_Info info);
int MPI_File_get_info(MPI_File fh, MPI_Info *info_used);

/* Section 9.3 */
int MPI_File_set_view(MPI_File fh, MPI_Offset disp, MPI_Datatype etype,
	         MPI_Datatype filetype, char *datarep, MPI_Info info);
int MPI_File_get_view(MPI_File fh, MPI_Offset *disp, 
                 MPI_Datatype *etype, MPI_Datatype *filetype, char *datarep);

/* Section 9.4.2 */
int MPI_File_read_at(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_read_at_all(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write_at(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write_at_all(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPI_Status *status);

/* nonblocking calls currently use MPIO_Request, because generalized
   requests not yet implemented. For the same reason, MPIO_Test and 
   MPIO_Wait are used to test and wait on nonblocking I/O requests */ 

int MPI_File_iread_at(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPIO_Request *request);
int MPI_File_iwrite_at(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPIO_Request *request);

/* Section 9.4.3 */
int MPI_File_read(MPI_File fh, void *buf, int count, MPI_Datatype
	     datatype, MPI_Status *status); 
int MPI_File_read_all(MPI_File fh, void *buf, int count, MPI_Datatype
	     datatype, MPI_Status *status); 
int MPI_File_write(MPI_File fh, void *buf, int count, MPI_Datatype
	      datatype, MPI_Status *status);
int MPI_File_write_all(MPI_File fh, void *buf, int count, MPI_Datatype
	      datatype, MPI_Status *status);

/* nonblocking calls currently use MPIO_Request, because generalized
   requests not yet implemented. For the same reason, MPIO_Test and 
   MPIO_Wait are used to test and wait on nonblocking I/O requests */ 

int MPI_File_iread(MPI_File fh, void *buf, int count, MPI_Datatype
	     datatype, MPIO_Request *request); 
int MPI_File_iwrite(MPI_File fh, void *buf, int count, MPI_Datatype
	      datatype, MPIO_Request *request);

int MPI_File_seek(MPI_File fh, MPI_Offset offset, int whence);
int MPI_File_get_position(MPI_File fh, MPI_Offset *offset);
int MPI_File_get_byte_offset(MPI_File fh, MPI_Offset offset, 
                                     MPI_Offset *disp);

/* Section 9.4.4 */
int MPI_File_read_shared(MPI_File fh, void *buf, int count, 
                         MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write_shared(MPI_File fh, void *buf, int count, 
                          MPI_Datatype datatype, MPI_Status *status);
int MPI_File_iread_shared(MPI_File fh, void *buf, int count, 
                          MPI_Datatype datatype, MPIO_Request *request);
int MPI_File_iwrite_shared(MPI_File fh, void *buf, int count, 
                           MPI_Datatype datatype, MPIO_Request *request);
int MPI_File_read_ordered(MPI_File fh, void *buf, int count, 
                          MPI_Datatype datatype, MPI_Status *status);
int MPI_File_write_ordered(MPI_File fh, void *buf, int count, 
                           MPI_Datatype datatype, MPI_Status *status);
int MPI_File_seek_shared(MPI_File fh, MPI_Offset offset, int whence);
int MPI_File_get_position_shared(MPI_File fh, MPI_Offset *offset);

/* Section 9.4.5 */
int MPI_File_read_at_all_begin(MPI_File fh, MPI_Offset offset, void *buf,
                               int count, MPI_Datatype datatype);
int MPI_File_read_at_all_end(MPI_File fh, void *buf, MPI_Status *status);
int MPI_File_write_at_all_begin(MPI_File fh, MPI_Offset offset, void *buf,
                                int count, MPI_Datatype datatype);
int MPI_File_write_at_all_end(MPI_File fh, void *buf, MPI_Status *status);
int MPI_File_read_all_begin(MPI_File fh, void *buf, int count, 
                            MPI_Datatype datatype);
int MPI_File_read_all_end(MPI_File fh, void *buf, MPI_Status *status);
int MPI_File_write_all_begin(MPI_File fh, void *buf, int count, 
                             MPI_Datatype datatype);
int MPI_File_write_all_end(MPI_File fh, void *buf, MPI_Status *status);
int MPI_File_read_ordered_begin(MPI_File fh, void *buf, int count, 
                                MPI_Datatype datatype);
int MPI_File_read_ordered_end(MPI_File fh, void *buf, MPI_Status *status);
int MPI_File_write_ordered_begin(MPI_File fh, void *buf, int count, 
                                 MPI_Datatype datatype);
int MPI_File_write_ordered_end(MPI_File fh, void *buf, MPI_Status *status);

/* Section 9.5.1 */
int MPI_File_get_type_extent(MPI_File fh, MPI_Datatype datatype, 
                                     MPI_Aint *extent);

/* Section 9.6.1 */
int MPI_File_set_atomicity(MPI_File fh, int flag);
int MPI_File_get_atomicity(MPI_File fh, int *flag);
int MPI_File_sync(MPI_File fh);

#ifndef HAVE_MPI_DARRAY_SUBARRAY
/* Section 4.14.4 */
int MPI_Type_create_subarray(int ndims, int *array_of_sizes, int
                      *array_of_subsizes, int *array_of_starts, int order, 
                      MPI_Datatype oldtype, MPI_Datatype *newtype);

/* Section 4.14.5 */
int MPI_Type_create_darray(int size, int rank, int ndims, 
                    int *array_of_gsizes, int *array_of_distribs, 
                    int *array_of_dargs, int *array_of_psizes, 
                    int order, MPI_Datatype oldtype, MPI_Datatype *newtype);
#endif

/* Section 4.12.4 */
#ifdef MPI_File_f2c
#undef MPI_File_f2c
#endif
#ifdef MPI_File_c2f
#undef MPI_File_c2f
#endif
/* above needed for some versions of mpi.h in MPICH!! */
MPI_File MPI_File_f2c(MPI_Fint fh);
MPI_Fint MPI_File_c2f(MPI_File fh);


/* The foll. functions are required, because an MPIO_Request object
   is currently used for nonblocking I/O. These functions will go away
   after generalized requests are implemented. */
int MPIO_Test(MPIO_Request *request, int *flag, MPI_Status *status);
int MPIO_Wait(MPIO_Request *request, MPI_Status *status);
MPI_Fint MPIO_Request_c2f(MPIO_Request request);
MPIO_Request MPIO_Request_f2c(MPI_Fint request);


/* info functions if not defined in the MPI implementation */
#ifndef HAVE_MPI_INFO

int MPI_Info_create(MPI_Info *info);
int MPI_Info_set(MPI_Info info, char *key, char *value);
int MPI_Info_delete(MPI_Info info, char *key);
int MPI_Info_get(MPI_Info info, char *key, int valuelen, 
                         char *value, int *flag);
int MPI_Info_get_valuelen(MPI_Info info, char *key, int *valuelen, 
                                  int *flag);
int MPI_Info_get_nkeys(MPI_Info info, int *nkeys);
int MPI_Info_get_nthkey(MPI_Info info, int n, char *key);
int MPI_Info_dup(MPI_Info info, MPI_Info *newinfo);
int MPI_Info_free(MPI_Info *info);

#ifdef MPI_Info_f2c
#undef MPI_Info_f2c
#endif
#ifdef MPI_Info_c2f
#undef MPI_Info_c2f
#endif
/* above needed for some versions of mpi.h in MPICH!! */
MPI_Fint MPI_Info_c2f(MPI_Info info);
MPI_Info MPI_Info_f2c(MPI_Fint info);
#endif

#endif   /* HAVE_PRAGMA_HP_SEC_DEF */


/**************** BINDINGS FOR THE PROFILING INTERFACE ***************/


/* Section 9.2 */
int PMPI_File_open(MPI_Comm comm, char *filename, int amode, 
                           MPI_Info info, MPI_File *fh);
int PMPI_File_close(MPI_File *fh);
int PMPI_File_delete(char *filename, MPI_Info info);
int PMPI_File_set_size(MPI_File fh, MPI_Offset size);
int PMPI_File_preallocate(MPI_File fh, MPI_Offset size);
int PMPI_File_get_size(MPI_File fh, MPI_Offset *size);
int PMPI_File_get_group(MPI_File fh, MPI_Group *group);
int PMPI_File_get_amode(MPI_File fh, int *amode);
int PMPI_File_set_info(MPI_File fh, MPI_Info info);
int PMPI_File_get_info(MPI_File fh, MPI_Info *info_used);

/* Section 9.3 */
int PMPI_File_set_view(MPI_File fh, MPI_Offset disp, 
    MPI_Datatype etype, MPI_Datatype filetype, char *datarep, MPI_Info info);
int PMPI_File_get_view(MPI_File fh, MPI_Offset *disp, 
      MPI_Datatype *etype, MPI_Datatype *filetype, char *datarep);

/* Section 9.4.2 */
int PMPI_File_read_at(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPI_Status *status);
int PMPI_File_read_at_all(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPI_Status *status);
int PMPI_File_write_at(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPI_Status *status);
int PMPI_File_write_at_all(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPI_Status *status);

/* nonblocking calls currently use MPIO_Request, because generalized
   requests not yet implemented. For the same reason, MPIO_Test and 
   MPIO_Wait are used to test and wait on nonblocking I/O requests */ 

int PMPI_File_iread_at(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPIO_Request *request);
int PMPI_File_iwrite_at(MPI_File fh, MPI_Offset offset, void *buf,
	      int count, MPI_Datatype datatype, MPIO_Request *request);

/* Section 9.4.3 */
int PMPI_File_read(MPI_File fh, void *buf, int count, MPI_Datatype
                           datatype, MPI_Status *status); 
int PMPI_File_read_all(MPI_File fh, void *buf, int count, MPI_Datatype
	                       datatype, MPI_Status *status); 
int PMPI_File_write(MPI_File fh, void *buf, int count, MPI_Datatype
	                    datatype, MPI_Status *status);
int PMPI_File_write_all(MPI_File fh, void *buf, int count, MPI_Datatype
	                        datatype, MPI_Status *status);

/* nonblocking calls currently use MPIO_Request, because generalized
   requests not yet implemented. For the same reason, MPIO_Test and 
   MPIO_Wait are used to test and wait on nonblocking I/O requests */ 

int PMPI_File_iread(MPI_File fh, void *buf, int count, MPI_Datatype
	                    datatype, MPIO_Request *request); 
int PMPI_File_iwrite(MPI_File fh, void *buf, int count, MPI_Datatype
	                     datatype, MPIO_Request *request);

int PMPI_File_seek(MPI_File fh, MPI_Offset offset, int whence);
int PMPI_File_get_position(MPI_File fh, MPI_Offset *offset);
int PMPI_File_get_byte_offset(MPI_File fh, MPI_Offset offset, 
                                      MPI_Offset *disp);

/* Section 9.4.4 */
int PMPI_File_read_shared(MPI_File fh, void *buf, int count, 
                         MPI_Datatype datatype, MPI_Status *status);
int PMPI_File_write_shared(MPI_File fh, void *buf, int count, 
                          MPI_Datatype datatype, MPI_Status *status);
int PMPI_File_iread_shared(MPI_File fh, void *buf, int count, 
                          MPI_Datatype datatype, MPIO_Request *request);
int PMPI_File_iwrite_shared(MPI_File fh, void *buf, int count, 
                           MPI_Datatype datatype, MPIO_Request *request);
int PMPI_File_read_ordered(MPI_File fh, void *buf, int count, 
                          MPI_Datatype datatype, MPI_Status *status);
int PMPI_File_write_ordered(MPI_File fh, void *buf, int count, 
                           MPI_Datatype datatype, MPI_Status *status);
int PMPI_File_seek_shared(MPI_File fh, MPI_Offset offset, int whence);
int PMPI_File_get_position_shared(MPI_File fh, MPI_Offset *offset);

/* Section 9.4.5 */
int PMPI_File_read_at_all_begin(MPI_File fh, MPI_Offset offset, void *buf,
                               int count, MPI_Datatype datatype);
int PMPI_File_read_at_all_end(MPI_File fh, void *buf, MPI_Status *status);
int PMPI_File_write_at_all_begin(MPI_File fh, MPI_Offset offset, void *buf,
                                int count, MPI_Datatype datatype);
int PMPI_File_write_at_all_end(MPI_File fh, void *buf, MPI_Status *status);
int PMPI_File_read_all_begin(MPI_File fh, void *buf, int count, 
                            MPI_Datatype datatype);
int PMPI_File_read_all_end(MPI_File fh, void *buf, MPI_Status *status);
int PMPI_File_write_all_begin(MPI_File fh, void *buf, int count, 
                             MPI_Datatype datatype);
int PMPI_File_write_all_end(MPI_File fh, void *buf, MPI_Status *status);
int PMPI_File_read_ordered_begin(MPI_File fh, void *buf, int count, 
                                MPI_Datatype datatype);
int PMPI_File_read_ordered_end(MPI_File fh, void *buf, MPI_Status *status);
int PMPI_File_write_ordered_begin(MPI_File fh, void *buf, int count, 
                                 MPI_Datatype datatype);
int PMPI_File_write_ordered_end(MPI_File fh, void *buf, MPI_Status *status);

/* Section 9.5.1 */
int PMPI_File_get_type_extent(MPI_File fh, MPI_Datatype datatype, 
                                      MPI_Aint *extent);

/* Section 9.6.1 */
int PMPI_File_set_atomicity(MPI_File fh, int flag);
int PMPI_File_get_atomicity(MPI_File fh, int *flag);
int PMPI_File_sync(MPI_File fh);

#ifndef HAVE_MPI_DARRAY_SUBARRAY
/* Section 4.14.4 */
int PMPI_Type_create_subarray(int ndims, int *array_of_sizes, int
                      *array_of_subsizes, int *array_of_starts, int order, 
                      MPI_Datatype oldtype, MPI_Datatype *newtype);

/* Section 4.14.5 */
int PMPI_Type_create_darray(int size, int rank, int ndims, 
                    int *array_of_gsizes, int *array_of_distribs, 
                    int *array_of_dargs, int *array_of_psizes, 
                    int order, MPI_Datatype oldtype, MPI_Datatype *newtype);
#endif

/* Section 4.12.4 */
MPI_File PMPI_File_f2c(MPI_Fint fh);
MPI_Fint PMPI_File_c2f(MPI_File fh);

/* Section 4.13.3 */
int MPI_File_set_errhandler(MPI_File fh, MPI_Errhandler errhandler);
int MPI_File_get_errhandler(MPI_File fh, MPI_Errhandler *errhandler);


/* The foll. functions are required, because an MPIO_Request object
   is currently used for nonblocking I/O. These functions will go away
   after generalized requests are implemented. */
int PMPIO_Test(MPIO_Request *request, int *flag, MPI_Status *status);
int PMPIO_Wait(MPIO_Request *request, MPI_Status *status);
MPI_Fint PMPIO_Request_c2f(MPIO_Request request);
MPIO_Request PMPIO_Request_f2c(MPI_Fint request);


/* info functions if not defined in the MPI implementation */
#ifndef HAVE_MPI_INFO

int PMPI_Info_create(MPI_Info *info);
int PMPI_Info_set(MPI_Info info, char *key, char *value);
int PMPI_Info_delete(MPI_Info info, char *key);
int PMPI_Info_get(MPI_Info info, char *key, int valuelen, 
                         char *value, int *flag);
int PMPI_Info_get_valuelen(MPI_Info info, char *key, int *valuelen, 
                                  int *flag);
int PMPI_Info_get_nkeys(MPI_Info info, int *nkeys);
int PMPI_Info_get_nthkey(MPI_Info info, int n, char *key);
int PMPI_Info_dup(MPI_Info info, MPI_Info *newinfo);
int PMPI_Info_free(MPI_Info *info);

MPI_Fint PMPI_Info_c2f(MPI_Info info);
MPI_Info PMPI_Info_f2c(MPI_Fint info);
#endif

#if defined(__cplusplus)
}
#endif

#endif
