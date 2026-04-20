// Braydon Johnston RedID: 131049942
// Ryan Desrosiers RedID: 130096873

#ifndef LEVEL_H
#define LEVEL_H

#include <unordered_map>

// level struct represents EACH NODE in page table tree 
struct Level {
    bool isLeaf; // are we at leaf yet ? 
    int numEntries; // number of entries in this level
    Level** nextLevel; // array of pointers to the next level (if not a leaf) 
    std::unordered_map<unsigned int, unsigned int> mapping; // leaf level mapping

    // make sure param order matches the .cpp implementation (const & dest)
    Level(bool leaf, int entries);
    ~Level();
};

#endif
