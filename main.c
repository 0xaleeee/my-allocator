#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "blocklist.h"

typedef struct {
    size_t n;
    bool is_free;
} HEADBLOCK;
void* GlobalFirstBlock;
void* GlobalLastBlock;
BlockList ReservedBlocks;
uint32_t NumOfResBlcs = 0;





void InitialAllocation() {
    GlobalFirstBlock = (void*)malloc(2048);
    if (GlobalFirstBlock == NULL) {
        printf("NULL Malloc");
        return;
    }
    memset(GlobalFirstBlock, 0, 2048);
    GlobalLastBlock = GlobalFirstBlock;
    ((HEADBLOCK*)GlobalFirstBlock)->is_free = true;
    ((HEADBLOCK*)GlobalFirstBlock)->n = 2048 - sizeof(HEADBLOCK);
}


size_t FindTotalFreeSpaceF() {

    HEADBLOCK* FirstBlock = (HEADBLOCK*)GlobalFirstBlock;
    HEADBLOCK* LastBlock = (HEADBLOCK*)GlobalLastBlock;
    size_t TotalFreeSpace = 0;
    while (FirstBlock <= LastBlock) {
        if (FirstBlock->is_free)
            TotalFreeSpace += (FirstBlock->n + sizeof(HEADBLOCK));
        char* TemP = (char*)FirstBlock;
        TemP = TemP + FirstBlock->n + sizeof(HEADBLOCK);
        FirstBlock = (HEADBLOCK*)TemP;
    }
    if (TotalFreeSpace)TotalFreeSpace -= sizeof(HEADBLOCK);

    return TotalFreeSpace;
}


bool LongestContiguousFree(void** OutStart, void** OutEnd, size_t n, size_t* retn) {
    HEADBLOCK* FirstBlock = (HEADBLOCK*)GlobalFirstBlock;
    HEADBLOCK* LastBlock = (HEADBLOCK*)GlobalLastBlock;

    void* StartFree = NULL;
    void* EndFree = NULL;
    size_t LenOfContFree = sizeof(HEADBLOCK);

    void* TempStartFree = NULL;
    void* TempEndFree = NULL;
    uint16_t i = 0;

    while (FirstBlock <= LastBlock) {
        if (FirstBlock->is_free && TempStartFree == NULL) {
            TempStartFree = (void*)FirstBlock;
            i = 0;
        }
        else if (!FirstBlock->is_free && TempStartFree != NULL) {
            if (i > 1) {
                TempEndFree = (void*)FirstBlock;
                if (((uintptr_t)TempEndFree - (uintptr_t)TempStartFree) > LenOfContFree) {
                    LenOfContFree = (uintptr_t)TempEndFree - (uintptr_t)TempStartFree;
                    StartFree = TempStartFree;
                    EndFree = TempEndFree;
                }
            }
            TempStartFree = NULL;
            TempEndFree = NULL;
        }

        char* TemPC = (char*)FirstBlock;
        TemPC = TemPC + FirstBlock->n + sizeof(HEADBLOCK);
        FirstBlock = (HEADBLOCK*)TemPC;
        i++;
    }

    if (TempStartFree != NULL) {
        if (i > 1) {
            TempEndFree = (void*)FirstBlock;
            if (((uintptr_t)TempEndFree - (uintptr_t)TempStartFree) > LenOfContFree) {
                LenOfContFree = (uintptr_t)TempEndFree - (uintptr_t)TempStartFree;
                StartFree = TempStartFree;
                EndFree = NULL;
            }
        }
        TempStartFree = NULL;
        TempEndFree = NULL;
    }

    if (LenOfContFree - sizeof(HEADBLOCK) > n) {
        *OutStart = StartFree;
        *OutEnd = EndFree;
        *retn = LenOfContFree - sizeof(HEADBLOCK);
        return true;
    }

    return false;

}



void ShiftFreeBlocksToEnd() {
    HEADBLOCK* FirstBlock = (HEADBLOCK*)GlobalFirstBlock;
    HEADBLOCK* LastBlock = (HEADBLOCK*)GlobalLastBlock;
    void* StartFree = NULL;

    while (FirstBlock <= LastBlock) {
        if (FirstBlock->is_free && StartFree == NULL)
            StartFree = (void*)FirstBlock;

        else if (!FirstBlock->is_free && StartFree != NULL) {
            size_t Space = (uintptr_t)FirstBlock - (uintptr_t)StartFree;
            size_t BytesToShift = (uintptr_t)LastBlock - (uintptr_t)FirstBlock;
            BytesToShift = BytesToShift + LastBlock->n + sizeof(HEADBLOCK);
            memmove(StartFree, (void*)FirstBlock, BytesToShift);

            void* UpdateF = (void*)(FirstBlock + 1);
            void* UpdateL = (void*)(LastBlock + 1);
            for (BlockNode* node = ReservedBlocks.head; node != NULL; node = node->next) {
                if (node->ptr >= UpdateF && node->ptr <= UpdateL)
                    node->ptr = (void*)((char*)node->ptr - Space);
            }


            FirstBlock = ((HEADBLOCK*)((char*)FirstBlock - Space));
            LastBlock = ((HEADBLOCK*)((char*)LastBlock - Space));
            GlobalLastBlock = (void*)LastBlock;

            if (LastBlock->is_free)
                LastBlock->n += Space;
            else {
                HEADBLOCK* TempLH = ((HEADBLOCK*)((char*)LastBlock + LastBlock->n + sizeof(HEADBLOCK)));
                TempLH->is_free = true;
                TempLH->n = Space - sizeof(HEADBLOCK);
                GlobalLastBlock = (void*)TempLH;
                LastBlock = TempLH;
            }
            StartFree = NULL;


        }

        char* TemPC = (char*)FirstBlock;
        TemPC = TemPC + FirstBlock->n + sizeof(HEADBLOCK);
        FirstBlock = (HEADBLOCK*)TemPC;
    }
    if (StartFree != NULL && LastBlock->is_free) {
        size_t Space = (uintptr_t)LastBlock - (uintptr_t)StartFree;
        HEADBLOCK* TempLH = ((HEADBLOCK*)((char*)LastBlock - Space));
        TempLH->is_free = true;
        TempLH->n = LastBlock->n + Space;
        GlobalLastBlock = (void*)TempLH;
    }

}


void** my_malloc(size_t len) {
    HEADBLOCK* FirstBlock = (HEADBLOCK*)GlobalFirstBlock;
    HEADBLOCK* LastBlock = (HEADBLOCK*)GlobalLastBlock;

    while (FirstBlock <= LastBlock) {
        if (FirstBlock->is_free && FirstBlock->n >= len)
            goto allocate;
        char* TemPC = (char*)FirstBlock;
        TemPC = TemPC + FirstBlock->n + sizeof(HEADBLOCK);
        FirstBlock = (HEADBLOCK*)TemPC;
    }

    if (FindTotalFreeSpaceF() > len) {
        void* OutStart;
        void* OutEnd;
        size_t retn;
        if (LongestContiguousFree(&OutStart, &OutEnd, len, &retn)) {
            FirstBlock = (HEADBLOCK*)OutStart;
            HEADBLOCK* MergeEnd = (HEADBLOCK*)OutEnd;
            if (MergeEnd == NULL) {
                LastBlock = (HEADBLOCK*)OutStart;
                GlobalLastBlock = (void*)LastBlock;
            }
            FirstBlock->is_free = true;
            FirstBlock->n = retn;
            goto allocate;
        }
        else {
            ShiftFreeBlocksToEnd();
            FirstBlock = (void*)GlobalLastBlock;
            LastBlock = (void*)GlobalLastBlock;
            goto allocate;
        }
    }

    printf("There is no enough space");
    return NULL;

allocate:
    FirstBlock->is_free = false;
    if (FirstBlock->n - len > sizeof(HEADBLOCK)) {
        char* TemP = (char*)FirstBlock;
        TemP += (len + sizeof(HEADBLOCK));
        ((HEADBLOCK*)TemP)->is_free = true;
        ((HEADBLOCK*)TemP)->n = FirstBlock->n - len - sizeof(HEADBLOCK);
        if (FirstBlock == LastBlock)
            GlobalLastBlock = (HEADBLOCK*)TemP;
        FirstBlock->n = len;
    }
    void** handle = blocklist_add(&ReservedBlocks, (void*)(FirstBlock + 1));
    if (handle == NULL) {
        FirstBlock->is_free = true;
        return NULL;
    }

    return handle;
}

void* my_free(void** handle) {
    HEADBLOCK* TempH = (HEADBLOCK*)handle[0];
    TempH -= 1;
    TempH->is_free = true;

    blocklist_remove(&ReservedBlocks, handle);
    return NULL;
}

int main() {
    InitialAllocation();
    blocklist_init(&ReservedBlocks);



    blocklist_clear(&ReservedBlocks);
    free(GlobalFirstBlock);
    return 0;
}
