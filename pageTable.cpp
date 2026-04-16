#include "pageTable.h"
#include <iostream>
#include <unordered_map>

unordered_map<unsigned int, unsigned int>* PageTable::lookup_vpn2pfn(unsigned int virtualAddress)
{
    // start @ root of page table 
    Level* current = root; 
    for (int i = 0; i < levelCount; i++)
    {
         // extract VPN and offset from virtual address (use masks and shifts)
        masks[i] = (1 << shifts[i]) -1; // gives mask of 1s for bits in each level 
        int index = (virtualAddress >> shifts[i]) & masks[i]; // gives index for cur level by shifting virtual addr and masking it 
        if(current->isLeaf)// we are @ leaf level, get mapping
        {
            // handle case where entry is not present (return nullptr)
            // if entry is present, return pointer to mapping at leaf level 
            if (current->mapping.find(index) == current->mapping.end()) // .end() means nullptr aka not found 
            {
                return nullptr; 
            }
            else // mapping is there 
            {
                return &(current->mapping); 
            }
        }

        
        else // not at leaf level, keep traversing 
        {// loop through each level (using idx for each level)
            if (current->nextLevel[index] == nullptr)
            {
                return nullptr; // entry at index not there 
            }
            else // entry is there, so traverse 
            {
                current = current->nextLevel[index];
            }
        }

    }
}

void PageTable::insert_vpn2pfn(unsigned int virtualAddress, unsigned int frameNumber)
{
    // what is numEntries? 
    Level* current = root; // start @ root again 
    for (int i = 0; i < levelCount; i++)
    {
        int index = (virtualAddress >> shifts[i]) & masks[i]; 
        if (i == levelCount - 1)// @ leaf level 
        {
            current->mapping[index] = frameNumber; 
            return; 
        }
        else // not a leaf 
        {
            if (current->nextLevel[index] == nullptr) // entry DNE so create it 
            {
                current->nextLevel[index] = new Level(false, numEntries); 
            }
            current = current->nextLevel[index]; 
        }
    }
    return nullptr; 

}