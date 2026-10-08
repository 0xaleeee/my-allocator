#ifndef BLOCKLIST_H
#define BLOCKLIST_H

#include <stddef.h>

typedef struct BlockNode {
    void* ptr;
    struct BlockNode* prev;
    struct BlockNode* next;
} BlockNode;

typedef struct {
    BlockNode* head;
    BlockNode* tail;
    size_t size;
} BlockList;

void   blocklist_init(BlockList* list);
void** blocklist_add(BlockList* list, void* ptr);  
void   blocklist_remove(BlockList* list, void** handle);
void   blocklist_clear(BlockList* list);

#endif
