#ifndef LIST_H
#define LIST_H

/* simple linked list */

typedef struct list_node {
    struct list_node *prev;
    struct list_node *next;
    void *value;
} list_node_t;

typedef struct list {
    list_node_t *head;
    list_node_t *tail;
    void (*free)(void *ptr);
    unsigned int len;
} list_t;

#endif