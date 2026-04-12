#pragma once 
#include <unordered_map>
using namespace std; 

struct Level 
{
    bool isLeaf; // keep track of whether this level is a leaf or not
    int numEntries; // number of entries in this level
    Level** nextLevel; // array of pointers to the next level (if not a leaf
    unordered_map<unsigned int, unsigned int> mapping; // leaf level mapping ? 
    Level(int entries, bool leaf);
    ~Level(); 

}; 