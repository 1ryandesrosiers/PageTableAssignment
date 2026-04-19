// Braydon Johnston RedID: 131049942
// Ryan Desroisiers RedID: 130096873

#ifndef TLB_H
#define TLB_H

#define TLB_MISS 0xFFFFFFFF // sentinel for a TLB miss

typedef struct {
    unsigned int vpn;
    unsigned int pfn;
    unsigned int lastUsed; // timestamp for LRU approx
    int valid;
} TLBEntry;

typedef struct {
    TLBEntry *entries;
    int capacity;
} TLB;

TLB *createTLB(int capacity);
void freeTLB(TLB *tlb);
unsigned int tlbLookup(TLB *tlb, unsigned int vpn, unsigned int time);
void tlbInsert(TLB *tlb, unsigned int vpn, unsigned int pfn, unsigned int time);

#endif
