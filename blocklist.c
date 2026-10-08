#include <stdlib.h>
#include "blocklist.h"

void blocklist_init(BlockList* list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void** blocklist_add(BlockList* list, void* ptr) {
    BlockNode* node = (BlockNode*)malloc(sizeof(BlockNode));
    if (node == NULL)
        return NULL;
    node->ptr = ptr;
    node->next = NULL;
    node->prev = list->tail;
    if (list->tail)
        list->tail->next = node;
    else
        list->head = node;
    list->tail = node;
    list->size++;
    return &node->ptr;
}

void blocklist_remove(BlockList* list, void** handle) {
    BlockNode* node = (BlockNode*)handle;
    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;
    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;
    list->size--;
    free(node);
}

void blocklist_clear(BlockList* list) {
    BlockNode* node = list->head;
    while (node) {
        BlockNode* next = node->next;
        free(node);
        node = next;
    }
    list->head = list->tail = NULL;
    list->size = 0;
}
