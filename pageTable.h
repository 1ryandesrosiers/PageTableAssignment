// Braydon Johnston RedID: 131049942
// Ryan Desrosiers RedID: 130096873

#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "Level.h"

// if no val actually exists, return this 
static const unsigned int pageTableMISS = 0xFFFFFFFFu;

// multi level page table for a 32 bit virtual address space
// tree of Level nodes, leaf stores the final VPN -> PFN mapping
struct PageTable {
    int numLevels;
    int* bitsPerLevel; // bits used at each level
    unsigned int* shifts; // right shift amount per level
    unsigned int* masks; // full position mask per level
    int* entryCounts; // 2^bitsPerLevel[i] per level
    unsigned int offsetBits; // bits used for the page offset
    unsigned int offsetMask; // mask for the offset bits
    unsigned int pageSize; // 1 << offsetBits

    Level* root;
    unsigned int framesAllocated; // running PFN counter, starts at 0

    PageTable(int levels, int* bits);
    ~PageTable();

    // walk the tree and return the PFN, or its a miss
    unsigned int lookup(unsigned int virtualAddress);

    // allocate a new frame & install mapping
    unsigned int insert(unsigned int virtualAddress);

    unsigned int getVPNatLevel(unsigned int virtualAddress, int level) const;
    unsigned int getFullVPN(unsigned int virtualAddress) const;
    unsigned int getOffset(unsigned int virtualAddress) const;

    // total entries across all allocated nodes, for log_summary
    unsigned long int countTotalEntries() const;

private:
    // recursive helper for countTotalEntries
    unsigned long int countEntriesRecursive(Level* node, int level) const;
};

#endif
