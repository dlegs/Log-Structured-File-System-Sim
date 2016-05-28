#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "imap.h"

void cache_imap(imap *map){

    int seg, offset, fd_checkpoint, fd_seg, i, j;
    unsigned int block_num;
    char name[64];

    fd_checkpoint = open("DRIVE/CHECKPOINT_REGION", O_RDWR);
    for(i = 0; i < NUM_IMAP_BLOCKS; i++){
        read(fd_checkpoint, &block_num, sizeof(unsigned int));
        
        seg = findDiskSeg(block_num);
        offset = findOffset(block_num);

        sprintf(name, "DRIVE/SEGMENT%d", seg);
        fd_seg = open(name, O_RDWR);
        lseek(fd_seg, offset*KB, SEEK_SET);
        for(j = 0; j < NUM_IMAP_INDEX; j++){
            read(fd_seg, map->inds + i*NUM_IMAP_INDEX + j, sizeof(unsigned int));
        }
        close(fd_seg);
    }
    close(fd_checkpoint);
}

void update_imap(imap *map, int inode_num, unsigned int block_num){ 
    *(map->inds + inode_num) = block_num;
}

unsigned int getInodeLoc(imap *map, int inode_num){
    return *(map->inds + inode_num);
}

int findDiskSeg(unsigned int block_num){
    return (block_num / KB);
}

int findOffset(unsigned int block_num){
    return (block_num % KB);
}
