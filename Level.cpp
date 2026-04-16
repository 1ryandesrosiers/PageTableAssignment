#include "Level.h"

Level::Level(bool leaf, int entries)
{
    isLeaf = leaf; 
    numEntries = entries; 
    nextLevel = nullptr; 
    if (isLeaf)
    {
        mapping = unordered_map<unsigned int, unsigned int>(); 
        
    }
    else // if not a leaf, initialize the nextLevel array
    {
        nextLevel = new Level*[numEntries]; 
        for (int i = 0; i < numEntries; i++)
        {
            nextLevel[i] = nullptr; 
        }
    }

}

Level::~Level()
{
    if (!isLeaf)
    {
        for (int i = 0; i < numEntries; i++)
        {
            delete nextLevel[i]; 
        }
        delete[] nextLevel; 
    }
    else
    {
        mapping.clear();
    }
}