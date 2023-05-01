#ifndef __OPERATIONS_H__
#define __OPERATIONS_H__
#include "structs.h"
#include <stdio.h>
#include <errno.h>

/* useful macro for handling error codes */
#define DIE(assertion, call_description)				\
	do {												\
		if (assertion) {								\
			fprintf(stderr, "(%s, %d): ",				\
					__FILE__, __LINE__);				\
			perror(call_description);					\
			exit(errno);								\
		}												\
	} while (0)

void get_operations(void **operations);

#endif // __OPERATIONS_H__

