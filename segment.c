#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "segment.h"
#include "pair.h"

void initialize_segment(segment *s){
    int i, j, n;
    char base[MAX_SIZE_NAME];
    char test[MAX_COMMAND_SIZE];
    FILE *tempfile;
    pair new_pair;
    s->n = 0;
    for (i = 0; i < NUM_SEGMENTS; i++){
        sprintf(base, "./DRIVE/SEGMENT%d", i); 
        tempfile = fopen(base, "r"); 
        for(j = 0; j < TEST_SIZE; j++){
            test[j] = fgetc(tempfile);
        }
        if(BlankCheck(test)){
            fclose(tempfile);
            break;
        } else{
            s->n++;
            fclose(tempfile);
        }
    }
   
    /* put in the segment summary blocks ... should be 8 of them */
    initialize_pair(&new_pair);
    n = KB - NUM_SEG_SUM_BLOCKS; 
    for(i = 0; i < n; i++)
        memcpy(s->buffer + i*sizeof(pair), &new_pair, sizeof(pair));

    /* initialize all other bytes to contain zeroes */
    s->b = NUM_SEG_SUM_BLOCKS;
    for(i = s->b*n; i < MB; i++)
        s->buffer[i] = '0';
}

void increment_segment(segment *s){
    s->b++;
    if(s->b == KB){
        write_disk(s);
        reset_segment(s);
    }
}

void reset_segment(segment *s){
    /* just a slightly simpler version of initialize_segment */

    int i, n;
    pair new_pair;
    
    /* put in the segment summary blocks ... should be 8 of them */
    initialize_pair(&new_pair);
    n = KB - NUM_SEG_SUM_BLOCKS; 
    for(i = 0; i < n; i++)
        memcpy(s->buffer + i*sizeof(pair), &new_pair, sizeof(pair));

    /* initialize all other bytes to contain zeroes */
    s->b = NUM_SEG_SUM_BLOCKS;
    for(i = s->b*n; i < MB; i++)
        s->buffer[i] = '0';
}

void write_disk(segment *s){
    int fd;
    char buffer[MAX_SIZE_NAME];
    int i;

    sprintf(buffer, "DRIVE/SEGMENT%d", s->n);
    fd = open(buffer, O_RDWR, 0777);
    for(i = 0; i < MB; i++){
        write(fd, s->buffer + i, 1);
    }
    close(fd);
    s->n++;
}

int BlankCheck(char *test){
    int i;
    for (i = 0; i < TEST_SIZE; i++){
        if(test[i] != '0'){
            return 0;       
        }
    }
    return 1;
}
