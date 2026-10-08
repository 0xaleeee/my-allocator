# my-allocator

My own `malloc` and `free` in C, written to learn how memory allocators work.

The program takes one 2048 byte pool from the real `malloc` and hands out pieces of it. Every piece starts with a small header that stores its size and whether it is free.

## How it works

- **Allocation:** `my_malloc` walks through the blocks and uses the first free one that is big enough. If the block is much bigger than needed, it is split and the rest stays free.
- **Merging:** if no single block fits but the total free space is enough, neighbouring free blocks are merged into one.
- **Compaction:** if that is still not enough, all used blocks are moved to the start of the pool, so the free space becomes one big block at the end.
- **Free:** `my_free` marks the block as free and removes it from the list.

## Handles

Blocks can move during compaction, so `my_malloc` returns a `void**` (a handle) instead of a normal pointer. The real address is `*handle`. When a block moves, the address inside its handle is updated automatically.

Handles are stored in a doubly linked list (`blocklist.c`). I used a linked list instead of a vector because a vector can reallocate and move its elements, which would break the handles. List nodes never move.

## Files

- `main.c`: the allocator
- `blocklist.h`, `blocklist.c`: linked list that tracks allocated blocks

## Build

    gcc main.c blocklist.c -o allocator

## Example

    void** p = my_malloc(100);
    strcpy((char*)*p, "hello");
    my_free(p);

Always read `*p` when you need the address. Don't save it in another variable, because the block may move later.

## Limitations

- Pool size is fixed (2048 bytes)
- Not thread safe
