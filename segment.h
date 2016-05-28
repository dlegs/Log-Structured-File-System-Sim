#ifndef _segment_h
#define _segment_h

#include "defines.h"

typedef struct{
    int n; 
    int b;
    char buffer[MB]; 
} segment;

void initialize_segment(segment *s);
void increment_segment(segment *s);
void reset_segment(segment *s);
void write_disk(segment *s);
int BlankCheck(char *test);

#endif
