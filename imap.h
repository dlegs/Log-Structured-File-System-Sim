#ifndef _imap_h
#define _imap_h

#include "defines.h"

typedef struct{ 
    unsigned int inds[MAX_NUM_INODES];
} imap;

void cache_imap(imap *map);
void update_imap(imap *map, int inode_num, unsigned int block_num);
unsigned int getInodeLoc(imap *map, int inode_num);
int findDiskSeg(unsigned int block_num);
int findOffset(unsigned int block_num);


#endif
