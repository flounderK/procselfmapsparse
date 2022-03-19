#ifndef __PROCSELFMAPSPARSE_H__
#define __PROCSELFMAPSPARSE_H__
#include <stdint.h>
#include <unistd.h>

struct MemoryRegion {
    size_t start;
    size_t end;
    int prot;
    int flags;
    off_t offset;
    int dev_maj;
    int dev_min;
    size_t inode;
    char* name;
    struct MemoryRegion* next;
};

struct MemoryRegion* ProcSelfMaps_CreateMemRegionList();
void ProcSelfMaps_DisplayMemRegionList(struct MemoryRegion* mem_region);
void ProcSelfMaps_DestroyMemRegionList(struct MemoryRegion* head);
#endif // __PROCSELFMAPSPARSE_H__
