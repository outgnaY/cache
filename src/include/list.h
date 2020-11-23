#ifndef LIST_H
#define LIST_H

#include "common.h"

/* double linked-list */

/**
 * expands to the unnamed type definition of a struct which acts as 
 * the double linked-list base node.
 */

#define LIST_BASE_NODE_T(TYPE)\
struct {\
    ulint count;    /* count of nodes in list */\
    TYPE * start;   /* pointer of list start, NULL if empty */\
    TYPE * end;     /* pointer to list end, NULL if empty */\
}\

/**
 * expands to the unnamed type definition of a struct which should be 
 * embedded in the nodes of the list
 */
#define LIST_NODE_T(TYPE)\
struct {\
    TYPE * prev;    /* pointer to the previous node, NULL if empty */\
    TYPE * next;    /* pointer to the next node, NULL if empty */\
}\

/* initialize the list */
#define LIST_INIT(BASE)\
{\
    (BASE).count = 0;\
    (BASE).start = NULL;\
    (BASE).end = NULL;\
}\

/**
 * add the node to the first of the list
 * NAME: list name
 * BASE: base node
 * N: pointer to the node to be added to the list
 */
#define LIST_ADD_FIRST(NAME, BASE, N)\
{\
    ((BASE).count)++;\
    ((N)->NAME).next = (BASE).start;\
    ((N)->NAME).prev = NULL;\
    if ((BASE).start != NULL) {\
        (((BASE).start)->NAME).prev = (N);\
    }\
    (BASE).start = (N);\
    if ((BASE).end == NULL) {\
        (BASE).end = (N);\
    }\
}\

/**
 * add the node to the end of the list
 * NAME: list name
 * BASE: base node
 * N: pointer to the node to be added to the end
 */
#define LIST_ADD_LAST(NAME, BASE, N)\
{\
    ((BASE).count)++;\
    ((N)->NAME).prev = (BASE).end;\
    ((N)->NAME).next = NULL;\
    if ((BASE).end != NULL) {\
        ((BASE).end)->NAME).next = (N);\
    }\
    (BASE).end = (N);\
    if ((BASE).start == NULL) {\
        (BASE).start = (N);\
    }\
}\

/**
 * inserts NODE2 after NODE1 in a list
 */
#define LIST_INSERT_AFTER(NAME, BASE, NODE1, NODE2)\
{\
    ((BASE).count)++;\
    ((NODE2)->NAME).prev = (NODE1);\
    ((NODE2)->NAME).next = ((NODE1)->NAME).next;\
    if (((NODE1)->NAME).next != NULL) {\
        ((((NODE1)->NAME).next)->NAME).prev = (NODE2);\
    }\
    ((NODE1)->NAME).next = (NODE2);\
    if ((BASE).end == (NODE1)) {\
        (BASE).end = (NODE2);\
    }\
}\

/**
 * remove a node from a list
 * NAME: list name
 * BASE: base node
 * N: pointer to the node to be removed
 */
#define LIST_REMOVE(NAME, BASE, N)\
{\
    ((BASE).count)--;\
    if (((N)->NAME).next != NULL) {\
        ((((N)->NAME).next)->NAME).prev = ((N)->NAME).prev;\
    } else {\
        (BASE).end = ((N)->NAME).prev;\
    }\
    if (((N)->NAME).prev != NULL) {\
        ((((N)->NAME).prev)->NAME).next = ((N)->NAME).next;\
    } else {\
        (BASE).start = ((N)->NAME).next;\
    }\
}\

/* get next node of the list */
#define LIST_GET_NEXT(NAME, N) (((N)->NAME).next)

/* get previous node of the list */
#define LIST_GET_PREV(NAME, N) (((N)->NAME).prev)

/* get length of the list */
#define LIST_GET_LEN(BASE) (BASE).count

/* get first node of the list */
#define LIST_GET_FIRST(BASE) (BASE).start

/* get last node of the list */
#define LIST_GET_LAST(BASE) (BASE).end




#endif  /* LIST_H */