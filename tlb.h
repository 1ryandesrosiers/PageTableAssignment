// Braydon Johnston RedID: 131049942
// Ryan Desrosiers RedID: 130096873

#ifndef TLB_H
#define TLB_H

#define tlbMISS 0xFFFFFFFF // for a TLB miss, return this val 

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
TLB *createTLB(int maxSize);
void freeTLB(TLB *tlb); // clean up memory 
unsigned int tlbLookup(TLB *tlb, unsigned int vpn, unsigned int time); // lookup and insert just like page table 
void tlbInsert(TLB *tlb, unsigned int vpn, unsigned int pfn, unsigned int time);

#endif
