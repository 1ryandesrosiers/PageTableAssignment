// Braydon Johnston RedID: 131049942
// Ryan Desrosiers RedID: 130096873

#include <cstdlib>
#include "tlb.h"

TLB *createTLB(int capacity) {
    TLB *tlb = (TLB *)malloc(sizeof(TLB)); // allocate size 
    if (tlb == NULL) {
        return NULL;
    }

    tlb->capacity = capacity;
    tlb->entries = NULL;

    if (capacity > 0) { 
        tlb->entries = (TLBEntry *)malloc(sizeof(TLBEntry) * capacity);
        if (tlb->entries == NULL) {
            free(tlb);
            return NULL;
        }

        for (int i = 0; i < capacity; i++) { // init all entries to empty
            tlb->entries[i].vpn = 0;
            tlb->entries[i].pfn = 0;
            tlb->entries[i].lastUsed = 0;
            tlb->entries[i].valid = 0;
        }
    }

    return tlb;
}

void freeTLB(TLB *tlb) {
    if (tlb == NULL) {
        return;
    }
    free(tlb->entries);
    free(tlb);
}

unsigned int tlbLookup(TLB *tlb, unsigned int vpn, unsigned int time) {
    if (tlb == NULL || tlb->capacity == 0) {
        return TLB_MISS;
    }

    for (int i = 0; i < tlb->capacity; i++) {
        if (tlb->entries[i].valid && tlb->entries[i].vpn == vpn) {
            tlb->entries[i].lastUsed = time; // refresh on hit for LRU
            return tlb->entries[i].pfn;
        }
    }
    // if didnt find it... 
    return TLB_MISS;
}

void tlbInsert(TLB *tlb, unsigned int vpn, unsigned int pfn, unsigned int time) {
    if (tlb == NULL || tlb->capacity == 0) {
        return;
    }

    // if vpn is already in cache just update it
    for (int i = 0; i < tlb->capacity; i++) {
        if (tlb->entries[i].valid && tlb->entries[i].vpn == vpn) {
            tlb->entries[i].pfn = pfn;
            tlb->entries[i].lastUsed = time;
            return;
        }
    }

    // fill the first empty slot
    for (int i = 0; i < tlb->capacity; i++) {
        if (!tlb->entries[i].valid) {
            tlb->entries[i].vpn = vpn;
            tlb->entries[i].pfn = pfn;
            tlb->entries[i].lastUsed = time;
            tlb->entries[i].valid = 1;
            return;
        }
    }

    // all full, evict the LRU one (which is smallest lastUsed)
    int lruIndex = 0;
    unsigned int minTime = tlb->entries[0].lastUsed;

    for (int i = 1; i < tlb->capacity; i++) {
        if (tlb->entries[i].lastUsed < minTime) {
            minTime = tlb->entries[i].lastUsed;
            lruIndex = i;
        }
    }
    // replace evicted info with new's info
    tlb->entries[lruIndex].vpn = vpn;
    tlb->entries[lruIndex].pfn = pfn;
    tlb->entries[lruIndex].lastUsed = time;
    tlb->entries[lruIndex].valid = 1;
}
