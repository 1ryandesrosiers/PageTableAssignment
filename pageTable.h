// Braydon Johnston RedID: 131049942
// Ryan Desroisiers RedID: 130096873

#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "Level.h"

// sentinel returned by lookup when no mapping exists for a VA
// chosen to not collide with any realistic PFN
static const unsigned int PAGE_TABLE_MISS = 0xFFFFFFFFu;

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

    // walk the tree and return the PFN, or PAGE_TABLE_MISS if not mapped
    unsigned int lookup(unsigned int virtualAddress);

    // allocate a new frame for this VA and install the mapping
    // creates intermediate nodes as needed, returns the new PFN
    unsigned int insert(unsigned int virtualAddress);

    // pull out the index bits for a specific level from a full VA
    unsigned int extractVPNAtLevel(unsigned int virtualAddress, int level) const;

    // pull out the full VPN (all page index bits) from a VA
    unsigned int extractFullVPN(unsigned int virtualAddress) const;

    // pull out just the offset from a VA
    unsigned int extractOffset(unsigned int virtualAddress) const;

    // total entries across all allocated nodes, for log_summary
    unsigned long int countTotalEntries() const;

private:
    // recursive helper for countTotalEntries
    unsigned long int countEntriesRecursive(Level* node, int level) const;
};

#endif
