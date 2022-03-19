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

static struct MemoryRegion* ProcSelfMaps_CreateMemRegion(char* lineptr, size_t linelen){

    struct MemoryRegion* mem_region = NULL;
    char* tokptr = NULL;
    char* nextstrptr = NULL;
    size_t memory_region_designation_len = 0;
    if (lineptr == NULL){
        perror("empty lineptr");
        goto exit;
    }

    mem_region = (struct MemoryRegion*)malloc(sizeof(struct MemoryRegion));
    if (mem_region == NULL){
        perror("malloc MemRegion failed");
        goto exit;
    }
    memset(mem_region, 0, sizeof(struct MemoryRegion));
    //find string before - delimeter
    tokptr = strtok_r(lineptr, "-", &nextstrptr);
    mem_region->start = strtoul(tokptr, NULL, 16);
    // find string before ' ' delimeter
    tokptr = strtok_r(nextstrptr, " ", &nextstrptr);
    mem_region->end = strtoul(tokptr, NULL, 16);
    // find string before ' ' delimeter for page permissions
    tokptr = strtok_r(nextstrptr, " ", &nextstrptr);
    if (tokptr[0] == 'r'){
        mem_region->prot += PROT_READ;
    }
    if (tokptr[1] == 'w'){
        mem_region->prot += PROT_WRITE;
    }
    if (tokptr[2] == 'x'){
        mem_region->prot += PROT_EXEC;
    }

    if (tokptr[3] == 's'){
        mem_region->flags = MAP_SHARED;
    } else if (tokptr[3] == 'p'){
        mem_region->flags = MAP_PRIVATE;
    }

    tokptr = strtok_r(nextstrptr, " ", &nextstrptr);
    mem_region->offset = strtoul(tokptr, NULL, 16);

    tokptr = strtok_r(nextstrptr, ":", &nextstrptr);
    mem_region->dev_maj = strtoul(tokptr, NULL, 10);

    tokptr = strtok_r(nextstrptr, " ", &nextstrptr);
    mem_region->dev_min = strtoul(tokptr, NULL, 10);

    tokptr = strtok_r(nextstrptr, " ", &nextstrptr);
    mem_region->inode = strtoul(tokptr, NULL, 10);

    // don't try to copy the string if there is no string to copy
    if (linelen <= ((size_t)nextstrptr - (size_t)lineptr)){
        goto exit;
    }

    // clip off the extra spaces at the front of the string
    while (*(char*)nextstrptr == (char)' '){
        nextstrptr++;
    }

    memory_region_designation_len = strlen(nextstrptr);
    // replace newline with null byte if necessary
    if (nextstrptr[memory_region_designation_len-1] == (char)'\n'){
        nextstrptr[memory_region_designation_len-1] = '\0';
    } else {
        // strncpy does not account for null terminator if null byte is not contained
        // within length, adjust so that it encompasses the null byte
        memory_region_designation_len += 1;
    }

    mem_region->name = (char*)malloc(memory_region_designation_len);
    if (mem_region->name == NULL){
        perror("CreateMemRegion malloc mem_region name failure");
        goto exit;
    }

    mem_region->name = strncpy(mem_region->name, nextstrptr, memory_region_designation_len);
exit:
    return mem_region;
}


struct MemoryRegion* ProcSelfMaps_CreateMemRegionList(){

    char* lineptr = NULL;
    size_t linelen = 0;
    FILE* fil = fopen("/proc/self/maps", "r");

    struct MemoryRegion* head = NULL;
    struct MemoryRegion* tail = NULL;
    struct MemoryRegion* mem_region = NULL;

    while (getline(&lineptr, &linelen, fil) != -1){
        mem_region = ProcSelfMaps_CreateMemRegion(lineptr, linelen);
        if (tail == NULL){
            // initialize list
            head = mem_region;
            tail = mem_region;
        } else {
            // link it to the list
            tail->next = mem_region;
            tail = mem_region;
        }
        // setup for the next call to getline
        free(lineptr);
        lineptr = NULL;
        linelen = 0;
    }
    return head;
}

void ProcSelfMaps_DisplayMemRegionList(struct MemoryRegion* mem_region){
    if (mem_region == NULL){
        goto exit;
    }
    while (mem_region != NULL){
        printf("start=%p end=%p prot=%x flags=%x %08lx %d:%d %lu ",
               (void*)mem_region->start,
               (void*)mem_region->end,
               mem_region->prot,
               mem_region->flags,
               mem_region->offset,
               mem_region->dev_maj,
               mem_region->dev_min,
               mem_region->inode
               );
        if (mem_region->name != NULL){
            printf("%s", mem_region->name);
        }
        printf("\n");
        mem_region = mem_region->next;
    }

exit:
    return;
}

void ProcSelfMaps_DestroyMemRegionList(struct MemoryRegion* head){
    int strsize = 0;
    struct MemoryRegion* next = NULL;
    while (head != NULL){
        if (head->name != NULL){
            strsize = strlen(head->name);
            memset(head->name, 0, strsize);
            free(head->name);
        }
        next = head->next;
        memset(head, 0, sizeof(struct MemoryRegion));
        free(head);
        head = next;
    }

    return;
}
