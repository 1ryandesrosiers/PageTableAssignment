// Braydon Johnston RedID: 131049942
// Ryan Desrosiers RedID: 130096873

#include <cstdlib>
#include "tlb.h"

TLB *createTLB(int maxSize) {
    TLB *tlb = (TLB *)malloc(sizeof(TLB)); // allocate size 
    if (tlb == NULL) {
        return NULL;
    }

    tlb->capacity = maxSize;
    tlb->entries = NULL;

    if (maxSize > 0) { 
        tlb->entries = (TLBEntry *)malloc(sizeof(TLBEntry) * maxSize);
        if (tlb->entries == NULL) {
            free(tlb);
            return NULL;
        }

        for (int i = 0; i < maxSize; i++) { // init all entries to empty
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

unsigned int tlbLookup(TLB *tlb, unsigned int vpn, unsigned int time) 
{
    if (tlb == NULL || tlb->capacity == 0) {
        return tlbMISS;
    }

    for (int i = 0; i < tlb->capacity; i++) {
        if (tlb->entries[i].valid && tlb->entries[i].vpn == vpn) {
            tlb->entries[i].lastUsed = time; // refresh on hit for LRU
            return tlb->entries[i].pfn;
        }
    }
    // if didnt find it... 
    return tlbMISS;
}

void tlbInsert(TLB *tlb, unsigned int vpn, unsigned int pfn, unsigned int time) 
{
    if (tlb == NULL || tlb->capacity == 0) {
        return; // if TLB doesnt exist or capacity is 0, cant do anything 
    }

    // if vpn is already in cache just update it
    for (int i = 0; i < tlb->capacity; i++) {
        if (tlb->entries[i].valid && tlb->entries[i].vpn == vpn) { // make sure its valid too
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

    // LRU approx per a4-specs: evict oldest among the 10 most-recently-used
    int lruIndex = -1;
    unsigned int minTime = 0;
    for (int i = 0; i < tlb->capacity; i++) {
        int moreRecent = 0;
        for (int j = 0; j < tlb->capacity; j++)
            if (tlb->entries[j].lastUsed > tlb->entries[i].lastUsed) moreRecent++;
        if (moreRecent < 10 && (lruIndex == -1 || tlb->entries[i].lastUsed < minTime)) {
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
