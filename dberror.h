#ifndef DBERROR_H
#define DBERROR_H

#include "stdio.h"

/* module wide constants */
#define PAGE_SIZE 4096

/* return code definitions */
typedef int RC;

#define RC_OK 0
#define RC_FILE_NOT_FOUND 1
#define RC_FILE_HANDLE_NOT_INIT 2
#define RC_WRITE_FAILED 3
#define RC_READ_NON_EXISTING_PAGE 4
#define RC_FILE_NOT_DELETED 5
#define RC_BLOCK_READ_ERROR 6
#define RC_CURR_PAGE_POS_NULL -1
#define RC_PREV_BLOCK_NOT_EXIST 7
#define RC_BLOCK_APPEND_ERROR 8
#define RC_FILE_NOT_PAGE_SIZE 9
#define RC_NEXT_BLOCK_NOT_EXIST 10
#define RC_WRITE_NON_EXISTING_PAGE 11
#define RC_BUFFER_INITIALIZED 12
#define RC_MARK_DIRTY_ERROR 13
#define RC_UNPIN_PAGE_ERROR 14
#define RC_PAGE_IN_BUFFER_ERROR 15
#define RC_BUFFER_NOTINITIALIZED 16
#define RC_PAGE_ERROR 17
#define RC_FORCE_POOL_ERROR 18
#define RC_PIN_PAGE_ERROR 19


#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN 201
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN 202
#define RC_RM_NO_MORE_TUPLES 203
#define RC_RM_NO_PRINT_FOR_DATATYPE 204
#define RC_RM_UNKOWN_DATATYPE 205

#define RC_IM_KEY_NOT_FOUND 300
#define RC_IM_KEY_ALREADY_EXISTS 301
#define RC_IM_N_TO_LAGE 302
#define RC_TABLE_EXISTS 303

#define RC_TYPE_NOT_FOUND 401

/* holder for error messages */
extern char *RC_message;

/* print a message to standard out describing the error */
extern void printError (RC error);
extern char *errorMessage (RC error);

#define THROW(rc,message) \
  do {			  \
    RC_message=message;	  \
    return rc;		  \
  } while (0)		  \

// check the return code and exit if it is an error
#define CHECK(code)							\
  do {									\
    int rc_internal = (code);						\
    if (rc_internal != RC_OK)						\
      {									\
	char *message = errorMessage(rc_internal);			\
	printf("[%s-L%i-%s] ERROR: Operation returned error: %s\n",__FILE__, __LINE__, __TIME__, message); \
	free(message);							\
	exit(1);							\
      }									\
  } while(0);


#endif
