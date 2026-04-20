// Braydon Johnston RedID: 131049942
// Ryan Desrosiers RedID: 130096873
#include "pageTable.h"

static const unsigned int addrBits = 32; 

PageTable::PageTable(int levels, int* bits) 
{ 
    // set levels and allocate all per lvl arrays 
    numLevels = levels;
    bitsPerLevel = new int[numLevels];
    shifts = new unsigned int[numLevels];
    masks = new unsigned int[numLevels];
    entryCounts = new int[numLevels];

    int totalIdxBits = 0; // calc total bits 
    for (int i = 0; i < numLevels; i++) {
        bitsPerLevel[i] = bits[i];
        totalIdxBits += bits[i];
    }
    // offset is total address minus ones used for index
    offsetBits = addrBits - (unsigned int)totalIdxBits;
    offsetMask = (1u << offsetBits) - 1u; 
    pageSize = 1u << offsetBits; // pg size = 2^ offset bits (class slides)

    int bitsProcessed = 0;
    for (int i = 0; i < numLevels; i++) {
        bitsProcessed += bitsPerLevel[i];
        shifts[i] = addrBits - (unsigned int)bitsProcessed; 
        unsigned int lvlMask = (1u << bitsPerLevel[i]) - 1u;
        masks[i] = lvlMask << shifts[i]; 
        entryCounts[i] = 1 << bitsPerLevel[i];
    }

    framesAllocated = 0;

    
    bool rootIsLeaf = (numLevels == 1);
    root = new Level(rootIsLeaf, entryCounts[0]);
}

PageTable::~PageTable() { // make sure destructor cleans up everything (arrays and tree) so no memory leaks 
    delete root;
    delete[] bitsPerLevel;
    delete[] shifts;
    delete[] masks;
    delete[] entryCounts;
}
// split into two functions - one gets certain level idx, second gets whole VPN
unsigned int PageTable::getLvlIndex(unsigned int virtualAddr, int level) const {
    return (virtualAddr & masks[level]) >> shifts[level];
}
unsigned int PageTable::getFullVPN(unsigned int virtualAddr) const {
    return virtualAddr >> offsetBits;
}
unsigned int PageTable::lookup(unsigned int virtualAddr) 
{
    Level* current = root; // start at root of the tree 

    for (int level = 0; level < numLevels; level++) { 
        unsigned int idx = getLvlIndex(virtualAddr, level); // get all indices first 

        if (current->isLeaf) { // if @ leaf, get mapping 
            auto itr = current->mapping.find(idx);
            if (itr == current->mapping.end()) { // end() means not found so = miss
                return pageTableMISS; 
            }
            return itr->second; // gets the PFN 
        }

        if (current->nextLevel[idx] == nullptr) { // cant find child so miss
            return pageTableMISS; 
        }
        current = current->nextLevel[idx]; // if child is there just traverse 
    }
    return pageTableMISS; 
}

unsigned int PageTable::insert(unsigned int virtualAddr) 
{
    Level* current = root; // start @ root again 

    for (int level = 0; level < numLevels; level++) {
        unsigned int idx = getLvlIndex(virtualAddr, level); 

        if (level == numLevels - 1) {  // means we are at leaf level (which is also last lvl)
            unsigned int newFrame = framesAllocated++; 
            current->mapping[idx] = newFrame;
            return newFrame;
        }
       
        if (current->nextLevel[idx] == nullptr) { // if child not there, gotta make it 
            bool childIsLeaf = (level + 1 == numLevels - 1);
            current->nextLevel[idx] = new Level(childIsLeaf, entryCounts[level + 1]);
        }
        current = current->nextLevel[idx];// now that we know its there we can traverse 
    }
    return pageTableMISS; 
}
// two f(x)'s for counting
unsigned long int PageTable::countEntriesRecursive(Level* node, int level) const {
    if (node == nullptr) {
        return 0; 
    }
    unsigned long int total = (unsigned long int)entryCounts[level];
    if (!node->isLeaf) { // make sure were not alr at end 
        for (int i = 0; i < node->numEntries; i++) {
            total += countEntriesRecursive(node->nextLevel[i], level + 1);
        }
    }
    return total;
}

unsigned long int PageTable::countTotalEntries() const {
    return countEntriesRecursive(root, 0); // calls recursive count 
}
