#pragma once 
#include "Level.h"
#include <unordered_map>

using namespace std; 

struct PageTable
{
    int numLevels;
    int* shifts;
    int* masks; 

    Level* root; 
    unordered_map<unsigned int, unsigned int>* lookup_vpn2pfn(unsigned int virtualAddress);

    void insert_vpn2pfn(unsigned int virtualAddress, unsigned int frameNumber); 

}; 
