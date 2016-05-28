#include <string.h>
#include "inode.h"


void initialize_inode(inode *node, int num_datablocks, char *filename, unsigned int block_start, int last_offset){
    int i;

    node->file_size = (num_datablocks-1)*KB + last_offset;
    strcpy(node->file_name, filename); 

    for(i = 0; i < MAX_DATABLOCKS_FILE; i++)
        node->datablocks[i] = (unsigned int)-1;

    for(i = 0; i < num_datablocks; i++)
         node->datablocks[i] = (i+block_start);
}

