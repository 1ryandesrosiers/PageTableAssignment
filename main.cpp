// Braydon Johnston RedID: 131049942
// Ryan Desroisiers RedID: 130096873

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

// tracereader.h and log.h are C headers with no extern "C" guard
// so wrap them here so the linker finds the right symbols
extern "C" {
#include "tracereader.h"
#include "log.h"
}

#include "pageTable.h"
#include "tlb.h"

// max total bits across all page table levels, per the spec
static const int MAX_PAGE_INDEX_BITS = 28;

// output mode tags, used for strcmp with the -o arg
static const char* MODE_BITMASKS     = "bitmasks";
static const char* MODE_VA2PA        = "va2pa";
static const char* MODE_VA2PA_ATC_PT = "va2pa_atc_ptwalk";
static const char* MODE_VPN2PFN      = "vpn2pfn";
static const char* MODE_OFFSET       = "offset";
static const char* MODE_SUMMARY      = "summary";

// holds everything parsed off the command line
struct Options {
    const char* tracePath;
    int*        bitsPerLevel;
    int         numLevels;
    int         maxAddresses; // -1 means no limit
    int         tlbCapacity; // 0 means no TLB
    const char* outputMode;
};

// parse argv into an Options struct, returns true on success
// on recoverable error prints the required message and returns false
static bool parseArguments(int argc, char* argv[], Options& opts) {
    opts.tracePath    = nullptr;
    opts.bitsPerLevel = nullptr;
    opts.numLevels    = 0;
    opts.maxAddresses = -1;
    opts.tlbCapacity  = 0;
    opts.outputMode   = MODE_SUMMARY;

    int option;
    while ((option = getopt(argc, argv, "n:c:o:")) != -1)
    {
        switch (option)
        {
            case 'n':
                opts.maxAddresses = atoi(optarg);
                if (opts.maxAddresses <= 0) {
                    fprintf(stderr, "Number of memory accesses must be a number, greater than 0\n");
                    return false;
                }
                break;
            case 'c':
                opts.tlbCapacity = atoi(optarg);
                if (opts.tlbCapacity < 0) {
                    fprintf(stderr, "Cache capacity must be a number, greater than or equal to 0\n");
                    return false;
                }
                break;
            case 'o':
                opts.outputMode = optarg;
                break;
            default:
                return false;
        }
    }

    // first mandatory arg is the trace file, rest are bits per level
    if (optind >= argc) {
        return false; // no trace file given
    }
    opts.tracePath = argv[optind++];

    opts.numLevels = argc - optind;
    if (opts.numLevels < 1) {
        return false; // no level bits given
    }

    opts.bitsPerLevel = new int[opts.numLevels];
    int totalBits = 0;
    for (int i = 0; i < opts.numLevels; i++) {
        opts.bitsPerLevel[i] = atoi(argv[optind + i]);
        if (opts.bitsPerLevel[i] < 1) {
            fprintf(stderr, "Level %d page table must be at least 1 bit\n", i);
            return false;
        }
        totalBits += opts.bitsPerLevel[i];
    }
    if (totalBits > MAX_PAGE_INDEX_BITS) {
        fprintf(stderr, "Too many bits used in page tables\n");
        return false;
    }

    return true;
}

// main simulation loop, address by address
// returns the number of addresses actually processed
static unsigned int runSimulation(FILE* traceFile,
                                  PageTable& pageTable,
                                  TLB* tlb,
                                  const Options& opts,
                                  unsigned int& cacheHits,
                                  unsigned int& pageTableHits,
                                  unsigned int& pageTableMisses) {
    cacheHits       = 0;
    pageTableHits   = 0;
    pageTableMisses = 0;
    unsigned int processed = 0;

    // reused buffer for the vpn2pfn output mode
    unsigned int* perLevelVPN = new unsigned int[opts.numLevels];

    p2AddrTr trace;
    while (NextAddress(traceFile, &trace)) {
        if (opts.maxAddresses != -1 &&
            processed >= (unsigned int)opts.maxAddresses) {
            break; // hit the -n limit
        }

        unsigned int vaddr  = trace.addr;
        unsigned int offset = pageTable.extractOffset(vaddr);
        unsigned int vpn    = pageTable.extractFullVPN(vaddr);

        // offset mode just prints the offset, no translation needed
        if (strcmp(opts.outputMode, MODE_OFFSET) == 0) {
            hexnum(offset);
            processed++;
            continue;
        }

        // MMU flow: try TLB first, then page table, then demand page
        bool tlbHit = false;
        bool pageTableHit = false;
        unsigned int pfn = PAGE_TABLE_MISS;

        if (tlb != nullptr && opts.tlbCapacity > 0) {
            unsigned int tlbResult = tlbLookup(tlb, vpn, processed);
            if (tlbResult != TLB_MISS) {
                pfn = tlbResult;
                tlbHit = true;
            }
        }

        if (!tlbHit) {
            unsigned int ptResult = pageTable.lookup(vaddr);
            if (ptResult != PAGE_TABLE_MISS) {
                pfn = ptResult;
                pageTableHit = true;
            }
            else {
                // page fault, allocate a new frame via demand paging
                pfn = pageTable.insert(vaddr);
            }

            // either way the mapping goes into the TLB for next time
            if (tlb != nullptr && opts.tlbCapacity > 0) {
                tlbInsert(tlb, vpn, pfn, processed);
            }
        }

        // tally the counters under the spec's three way split
        if (tlbHit) {
            cacheHits++;
        }
        else if (pageTableHit) {
            pageTableHits++;
        }
        else {
            pageTableMisses++;
        }

        // build physical address, PFN goes in high bits, offset in low bits
        unsigned int paddr = (pfn << pageTable.offsetBits) | offset;

        // per address output for whichever mode is active
        if (strcmp(opts.outputMode, MODE_VA2PA) == 0) {
            log_virtualAddr2physicalAddr(vaddr, paddr);
        }
        else if (strcmp(opts.outputMode, MODE_VA2PA_ATC_PT) == 0) {
            log_va2pa_ATC_PTwalk(vaddr, paddr, tlbHit, pageTableHit);
        }
        else if (strcmp(opts.outputMode, MODE_VPN2PFN) == 0) {
            for (int i = 0; i < opts.numLevels; i++) {
                perLevelVPN[i] = pageTable.extractVPNAtLevel(vaddr, i);
            }
            log_pagemapping(opts.numLevels, perLevelVPN, pfn);
        }
        // summary mode prints at the end, nothing here per address

        processed++;
    }

    delete[] perLevelVPN;
    return processed;
}

int main(int argc, char* argv[]) {
    Options opts;
    if (!parseArguments(argc, argv, opts)) {
        delete[] opts.bitsPerLevel;
        return 1;
    }

    // bitmasks mode, build the table just to compute masks then exit
    if (strcmp(opts.outputMode, MODE_BITMASKS) == 0) {
        PageTable pt(opts.numLevels, opts.bitsPerLevel);
        log_bitmasks(opts.numLevels, pt.masks);
        delete[] opts.bitsPerLevel;
        return 0;
    }

    // every other mode needs the trace file open
    FILE* traceFile = fopen(opts.tracePath, "rb");
    if (traceFile == nullptr) {
        fprintf(stderr, "Unable to open <<%s>>\n", opts.tracePath);
        delete[] opts.bitsPerLevel;
        return 1;
    }

    PageTable pageTable(opts.numLevels, opts.bitsPerLevel);

    // only make a real TLB if -c N was given with N > 0
    TLB* tlb = nullptr;
    if (opts.tlbCapacity > 0) {
        tlb = createTLB(opts.tlbCapacity);
    }

    unsigned int cacheHits       = 0;
    unsigned int pageTableHits   = 0;
    unsigned int pageTableMisses = 0;
    unsigned int addressesProcessed =
        runSimulation(traceFile, pageTable, tlb, opts,
                      cacheHits, pageTableHits, pageTableMisses);

    // summary is the default mode if -o wasnt given
    if (strcmp(opts.outputMode, MODE_SUMMARY) == 0) {
        log_summary(pageTable.pageSize,
                    cacheHits,
                    pageTableHits,
                    addressesProcessed,
                    pageTable.framesAllocated,
                    pageTable.countTotalEntries());
    }

    // cleanup
    fclose(traceFile);
    if (tlb != nullptr) {
        freeTLB(tlb);
    }
    delete[] opts.bitsPerLevel;
    return 0;
}
