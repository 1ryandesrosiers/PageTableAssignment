// Braydon Johnston RedID: 131049942
// Ryan Desrosiers RedID: 130096873

#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "Level.h"

// if no val actually exists, return this 
static const unsigned int pageTableMISS = 0xFFFFFFFFu;


struct PageTable {
    int numLevels;
    int* bitsPerLevel; // used to make shifts and masks arrays 
    unsigned int* shifts; 
    unsigned int* masks; 
    int* entryCounts; 
    unsigned int offsetBits; // offset number of bits and mask 
    unsigned int offsetMask; 
    unsigned int pageSize; 

    Level* root;
    unsigned int framesAllocated; 

    PageTable(int levels, int* bits); // constructor and destructor 
    ~PageTable();

    // all f(x)s 
    unsigned int lookup(unsigned int virtualAddress); 
    
    unsigned int insert(unsigned int virtualAddress); 

    unsigned int getLvlIndex(unsigned int virtualAddress, int level) const; 
    unsigned int getFullVPN(unsigned int virtualAddress) const;

    
    unsigned long int countTotalEntries() const;

private:
    // recursive helper for countTotalEntries
    unsigned long int countEntriesRecursive(Level* node, int level) const;
};

#endif
