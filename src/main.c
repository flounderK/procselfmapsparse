#define _GNU_SOURCE
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include "procselfmapsparse.h"


int main(int argc, char* argv[]){
    int ret = -1;
    struct MemoryRegion* head = ProcSelfMaps_CreateMemRegionList();
    ProcSelfMaps_DisplayMemRegionList(head);
    ProcSelfMaps_DestroyMemRegionList(head);

    ret = 0;
exit:
    return ret;
}
