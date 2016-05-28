#ifndef _pair_h
#define _pair_h

/* if inode_num and log_block_offset are both -1, means block is garbage */

typedef struct{
    int inode_num;
    int log_block_offset;
} pair;

void initialize_pair(pair *p);

#endif
