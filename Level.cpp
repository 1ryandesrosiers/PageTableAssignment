// Braydon Johnston RedID: 131049942
// Ryan Desrosiers RedID: 130096873

#include "Level.h"

Level::Level(bool leaf, int entries) {
    isLeaf = leaf;
    numEntries = entries;
    nextLevel = nullptr;

    if (isLeaf) {
        // leaf, mapping is default constructed (empty), nothing to init
    }
    else { // if not a leaf, init the nextLevel array
        nextLevel = new Level*[numEntries];
        for (int i = 0; i < numEntries; i++) {
            nextLevel[i] = nullptr; // init all children -> null
        }
    }
}

Level::~Level() { // destructor 
    if (!isLeaf && nextLevel != nullptr) {
        for (int i = 0; i < numEntries; i++) {
            delete nextLevel[i]; // recursively delete child subtrees
        }
        delete[] nextLevel;
    }
    // leaf mapping cleans itself up
}
