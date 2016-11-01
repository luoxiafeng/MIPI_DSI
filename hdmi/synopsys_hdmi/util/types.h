/*
 * types.h
 *
 *  Created on: Jun 25, 2010
 *      Author: klabadi & dlopo
 */

#ifndef TYPES_H_
#define TYPES_H_

/*
 * types
 * Define basic type optimised for use in the API so that it can be
 * platform-independent.
 */
#include <common.h>

typedef void * mutex_t;

#define BIT(n)		(1 << n)

typedef void (*handler_t)(void *);
typedef void* (*thread_t)(void *);

#define TRUE  1
#define FALSE 0

#endif /* TYPES_H_ */
