// Braydon Johnston RedID: 131049942
// Ryan Desrosiers RedID: 130096873

#ifndef TLB_H
#define TLB_H

#define TLB_MISS 0xFFFFFFFF // for a TLB miss, return this val 

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
// functions 
TLB *createTLB(int capacity);
void freeTLB(TLB *tlb);
unsigned int tlbLookup(TLB *tlb, unsigned int vpn, unsigned int time);
void tlbInsert(TLB *tlb, unsigned int vpn, unsigned int pfn, unsigned int time);

#endif
