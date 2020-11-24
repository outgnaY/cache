#ifndef COMMON_H
#define COMMON_H

#define MEM_ALIGNMENT 8

/**
 * TYPE DEFINITIONS
 */

#define byte unsigned char

typedef unsigned long int ulint;
typedef long int lint;

#define bool ulint
#define TRUE 1
#define FALSE 0

#define ULINT_UNDEFINED ((ulint)(-1))


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sched.h>

#endif  /* COMMON_H */