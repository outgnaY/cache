#include "cache.h"

// create a new list 
list_t *list_create() {
    list_t *list;
    if ((list = malloc(sizeof(*list))) == NULL) {
        fprintf(stderr, "allocate memory for list failed\n");
        return NULL;
    }
    list->head = list->tail = NULL;
    list->len = 0;
    list->free = NULL;
    return list;
}

// empty the list 
void list_empty(list_t *list) {
    unsigned int len = list->len;
    list_node_t *current, *next;
    current = list->head;
    while (len--) {
        next = current->next;
        if (list->free) {
            list->free(current->value);
        }
        free(current);
        current = next;
    }
    list->head = list->tail = NULL;
    list->len = 0;
}

// add a new node to the head of the list 
bool list_add_node_head(list_t *list, void *value) {
    list_node_t *node;
    if ((node = malloc(sizeof(*node))) == NULL) {
        return false;
    }
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->len++;
    return true;
}

// add a new node to the tail of list 
bool list_add_node_tail(list_t *list, void *value) {
    list_node_t *node;
    if ((node = malloc(sizeof(*node))) == NULL) {
        return false;
    }
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    return true;
}

// delete a node 
void list_del_node(list_t *list, list_node_t *node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }
    if (list->free) {
        list->free(node->value);
    }
    free(node);
    list->len--;
}