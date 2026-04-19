// Braydon Johnston RedID: 131049942
// Ryan Desroisiers RedID: 130096873

#include "pageTable.h"

static const unsigned int ADDRESS_BITS = 32; // 32 bit virtual address space

PageTable::PageTable(int levels, int* bits) {
    numLevels = levels;
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
    offsetBits = ADDRESS_BITS - (unsigned int)totalIndexBits;
    offsetMask = (1u << offsetBits) - 1u;
    pageSize = 1u << offsetBits;

    // build per level shifts, masks, entry counts
    // level 0 is the highest bits so it has the biggest shift
    int consumedBits = 0;
    for (int i = 0; i < numLevels; i++) {
        consumedBits += bitsPerLevel[i];
        shifts[i] = ADDRESS_BITS - (unsigned int)consumedBits;
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
    delete root; // recursive delete handles the whole tree
    delete[] bitsPerLevel;
    delete[] shifts;
    delete[] masks;
    delete[] entryCounts;
}

unsigned int PageTable::extractVPNAtLevel(unsigned int vaddr, int level) const {
    return (vaddr & masks[level]) >> shifts[level];
}

unsigned int PageTable::extractFullVPN(unsigned int vaddr) const {
    return vaddr >> offsetBits;
}

unsigned int PageTable::extractOffset(unsigned int vaddr) const {
    return vaddr & offsetMask;
}

unsigned int PageTable::lookup(unsigned int vaddr) {
    Level* current = root; // start at the root

    for (int level = 0; level < numLevels; level++) {
        unsigned int index = extractVPNAtLevel(vaddr, level);

        if (current->isLeaf) { // at the leaf, grab the mapping
            auto it = current->mapping.find(index);
            if (it == current->mapping.end()) {
                return PAGE_TABLE_MISS; // not mapped yet
            }
            return it->second;
        }

        // internal node, go down to the next level
        if (current->nextLevel[index] == nullptr) {
            return PAGE_TABLE_MISS; // subtree doesnt exist
        }
        current = current->nextLevel[index];
    }

    return PAGE_TABLE_MISS; // shouldnt get here unless numLevels <= 0
}

unsigned int PageTable::insert(unsigned int vaddr) {
    Level* current = root;

    for (int level = 0; level < numLevels; level++) {
        unsigned int index = extractVPNAtLevel(vaddr, level);

        if (level == numLevels - 1) { // deepest level, put the mapping here
            unsigned int newFrame = framesAllocated++; // grab next sequential frame
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

    return PAGE_TABLE_MISS; // shouldnt hit this
}

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
    return countEntriesRecursive(root, 0);
}
