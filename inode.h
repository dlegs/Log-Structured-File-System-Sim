#ifndef _inode_h
#define _inode_h

#include "defines.h"

typedef struct{
    int file_size;
    char file_name[MAX_SIZE_NAME];
    unsigned int datablocks[MAX_DATABLOCKS_FILE];    
} inode;

void initialize_inode(inode *node, int num_datablocks, char *filename, unsigned int block_start, int last_offset);
void print_inode(inode *node);



#endif
