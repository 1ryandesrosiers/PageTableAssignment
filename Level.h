// Braydon Johnston RedID: 131049942
// Ryan Desroisiers RedID: 130096873

#ifndef LEVEL_H
#define LEVEL_H

#include <unordered_map>

// one node of the page table tree
// leaf nodes hold the VPN slice -> PFN mapping
// internal nodes hold an array of child pointers
struct Level {
    bool isLeaf; // keep track of whether this level is a leaf or not
    int numEntries; // number of entries in this level
    Level** nextLevel; // array of pointers to the next level (if not a leaf)
    std::unordered_map<unsigned int, unsigned int> mapping; // leaf level mapping

    // param order matches the .cpp implementation
    Level(bool leaf, int entries);
    ~Level();
};

#endif
