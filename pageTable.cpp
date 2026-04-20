// Braydon Johnston RedID: 131049942
// Ryan Desrosiers RedID: 130096873

#include "pageTable.h"


static const unsigned int addrBits = 32; // says 32 bit virtual address space

PageTable::PageTable(int levels, int* bits) { // constructor, sets up everything needed for later 
    numLevels = levels;
    // allocate all arrays we'll need 
    bitsPerLevel = new int[numLevels];
    shifts = new unsigned int[numLevels];
    masks = new unsigned int[numLevels];
    entryCounts = new int[numLevels];

    // copy bit widths and figure out offset bits from whats left
    int totalIndexBits = 0;
    for (int i = 0; i < numLevels; i++) {
        bitsPerLevel[i] = bits[i];
        totalIndexBits += bits[i];
    }
    offsetBits = addrBits - (unsigned int)totalIndexBits;
    offsetMask = (1u << offsetBits) - 1u; 
    pageSize = 1u << offsetBits;

    // build per level shifts, masks, entry counts
    // level 0 is the highest bits so it has the biggest shift
    int bitsProcessed = 0;
    for (int i = 0; i < numLevels; i++) {
        bitsProcessed += bitsPerLevel[i];
        shifts[i] = addrBits - (unsigned int)bitsProcessed;
        unsigned int lowMask = (1u << bitsPerLevel[i]) - 1u;
        masks[i] = lowMask << shifts[i]; // full position mask for this level
        entryCounts[i] = 1 << bitsPerLevel[i];
    }

    framesAllocated = 0;

    // root is a leaf only when theres just one level
    bool rootIsLeaf = (numLevels == 1);
    root = new Level(rootIsLeaf, entryCounts[0]);
}

PageTable::~PageTable() {
    delete root; // recursive cleans whole tree
    delete[] bitsPerLevel;
    delete[] shifts;
    delete[] masks;
    delete[] entryCounts;
}

unsigned int PageTable::getVPNatLevel(unsigned int virtualAddr, int level) const {
    return (virtualAddr & masks[level]) >> shifts[level];
}
unsigned int PageTable::getFullVPN(unsigned int virtualAddr) const {
    return virtualAddr >> offsetBits;
}
unsigned int PageTable::getOffset(unsigned int virtualAddr) const {
    return virtualAddr & offsetMask;
}

unsigned int PageTable::lookup(unsigned int virtualAddr) {
    Level* current = root; // start at the root

    for (int level = 0; level < numLevels; level++) { // for every level, get index
        unsigned int index = getVPNatLevel(virtualAddr, level);

        if (current->isLeaf) { // at the leaf, grab the mapping
            auto it = current->mapping.find(index);
            if (it == current->mapping.end()) {
                return pageTableMISS; // not mapped yet
            }
            return it->second;
        }

        // child not there = miss
        if (current->nextLevel[index] == nullptr) {
            return pageTableMISS; // subtree doesnt exist
        }
        current = current->nextLevel[index];
    }

    return pageTableMISS; // shouldnt get here unless numLevels <= 0
}

unsigned int PageTable::insert(unsigned int virtualAddr) {
    Level* current = root;

    for (int level = 0; level < numLevels; level++) {
        unsigned int index = getVPNatLevel(virtualAddr, level);

        if (level == numLevels - 1) { // deepest level, put the mapping here
            unsigned int newFrame = framesAllocated++; // grab next frame
            current->mapping[index] = newFrame;
            return newFrame;
        }

        // intermediate level, make the child subtree if missing
        if (current->nextLevel[index] == nullptr) {
            bool childIsLeaf = (level + 1 == numLevels - 1);
            current->nextLevel[index] = new Level(childIsLeaf, entryCounts[level + 1]);
        }
        current = current->nextLevel[index];
    }

    return pageTableMISS; // shouldnt hit this
}
// two f(x)'s for counting 
unsigned long int PageTable::countEntriesRecursive(Level* node, int level) const {
    if (node == nullptr) {
        return 0;
    }
    // every allocated node contributes entryCounts[level] entries
    unsigned long int total = (unsigned long int)entryCounts[level];

    if (!node->isLeaf) {
        for (int i = 0; i < node->numEntries; i++) {
            total += countEntriesRecursive(node->nextLevel[i], level + 1);
        }
    }
    return total;
}

unsigned long int PageTable::countTotalEntries() const {
    return countEntriesRecursive(root, 0); // calls recursive count 
}
