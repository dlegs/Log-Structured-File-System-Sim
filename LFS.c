#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "segment.h"
#include "inode.h"
#include "imap.h"
#include "defines.h"

void IMPORT(char *command, segment *s, imap *map);
void REMOVE(char *command, segment *s, imap *map);
void CAT(char *command, segment *s, imap *map);
void DISPLAY(char *command, segment *s, imap *map);
/*void OVERWRITE(char *command, segment *s, imap *map);*/
void LIST(imap *map);
void EXIT(segment *s);
void printUsage();

int main(){

    segment s;                  
    imap map;

    cache_imap(&map);               // put imap in cache
    initialize_segment(&s);         // initialize in memory segment

    char command[MAX_COMMAND_SIZE];
    
    printUsage();
    printf("\n");
    while(fgets(command, MAX_COMMAND_SIZE, stdin)){ 
        switch(command[0]){
            case 'I':
                    IMPORT(command, &s, &map);
                    break; 
            case 'R':
                    REMOVE(command, &s, &map);
                    break; 
            case 'C':
                    CAT(command, &s, &map);
                    break; 
            case 'D':
                    DISPLAY(command, &s, &map);
                    break; 
            //case 'O':
            //        OVERWRITE(command, &s, &map);
            //        break; 
            case 'L':
                    LIST(&map);
                    break; 
            case 'E':
                    EXIT(&s);
                    break; 
            default:
                    printUsage();
                    continue;
        }
    }

    return 0;
}

void IMPORT(char *command, segment *s, imap *map){
    char garbage[MAX_SIZE_NAME];
    char filename[MAX_SIZE_NAME];
    char lfs_filename[MAX_SIZE_NAME];
    char c;
    char name[MAX_SIZE_NAME];
    int i, inode_num, num_datablocks, seg_start, block_start, num_read, map_index;

    int fd_mappings, fd_file, fd_checkpoint;
    inode new_inode;
    unsigned int imap_loc;

    /* Parse command */
    sscanf(command, "%s %s %s", garbage, filename, lfs_filename); 

    /* Allocate the next free inode to this file and write name to mappings */
    fd_mappings = open("DRIVE/MAPPINGS", O_RDWR, 0777);
    inode_num = 0;
    while(1){
        i = 0;
        while(read(fd_mappings, &c, 1) != 0){
            if(c != '\n'){
                if(c != ' ')
                    name[i++] = c; 
                else
                    name[i++] = 0;
            } else
                break;
        }
        if(strcmp(name, "F") == 0){
            break;
        }
        inode_num++;
    } 

    lseek(fd_mappings, -SIZE_FILE_LINE, SEEK_CUR);
    write(fd_mappings, lfs_filename, strlen(lfs_filename));
    close(fd_mappings);
 
    /* write this file to in-memory segment */
    fd_file = open(filename, O_RDONLY, 0777);
    num_datablocks = 0;
    seg_start = s->n;       //segment where data blocks start being written
    block_start = s->b;     //block offset where data blocks start being written
    while(1){
        num_read = read(fd_file, (s->buffer + (block_start + num_datablocks)*KB), KB);
        num_datablocks++;
        increment_segment(s);
        if(num_read < KB)
            break;
    }
    close(fd_file);

    /* initialize inode block and then write to in-memory segment */
    initialize_inode(&new_inode, num_datablocks, lfs_filename, (unsigned int)seg_start*KB + block_start, num_read);
    seg_start = s->n;       //segment where inode block is written
    block_start = s->b;     //block offset where inode block is written
    memcpy(s->buffer + block_start*KB, &new_inode, sizeof(inode));  
    increment_segment(s);

    /* update imap and write piece of imap to in-memory segment */
    update_imap(map, inode_num, (unsigned int)seg_start*KB + block_start);
    map_index = inode_num / 40;
    seg_start = s->n;       //segment where imap block is written
    block_start = s->b;     //block offset where imap block is written
    for(i = 0; i < NUM_IMAP_INDEX; i++){
        memcpy(s->buffer + block_start*KB + i*4, map->inds + i, 4);
    }
    increment_segment(s);

    /* update checkpoint region with the new location of the imap block  */ 
    fd_checkpoint = open("DRIVE/CHECKPOINT_REGION", O_RDWR, 0777);
    lseek(fd_checkpoint, map_index*sizeof(unsigned int), SEEK_SET);
    imap_loc = (unsigned int)seg_start*KB + block_start;
    write(fd_checkpoint, &imap_loc, sizeof(unsigned int)); 
    close(fd_checkpoint);

}

void REMOVE(char *command, segment *s, imap *map){
    //parse command
    char garbage[MAX_SIZE_NAME];
    char lfs_filename[MAX_SIZE_NAME];
    sscanf(command,"%s %s",garbage,lfs_filename);

    //look for filename in MAPPINGS & update
    int fd_mappings = open("DRIVE/MAPPINGS", O_RDWR, 0777);
    int fd_checkpoint;
    int i, j;
    char temp[MAX_SIZE_NAME];
    int inode_num;
    char c;
    int map_index, seg_start, block_start;
    unsigned int imap_loc;

    for(i = 0; i < MAX_NUM_FILES; i++){
        j = 0;
        while(read(fd_mappings, &c, 1) != 0){
            if(c != '\n'){
                if(c != ' ')
                    temp[j++] = c;
                else
                    temp[j++] = 0;
            } else
                break;
        }
        if(strcmp(lfs_filename, temp) == 0){
            inode_num = i;
            lseek(fd_mappings, -SIZE_FILE_LINE,SEEK_CUR);
            write(fd_mappings, "F             \n", SIZE_FILE_LINE);
            break;
        }
    }
    close(fd_mappings);

    /* update imap and write piece of imap to in-memory segment */
    update_imap(map, inode_num, -1);
    map_index = inode_num / 40;
    seg_start = s->n;       //segment where imap block is written
    block_start = s->b;     //block offset where imap block is written
    for(i = 0; i < NUM_IMAP_INDEX; i++){
        memcpy(s->buffer + block_start*KB + i*4, map->inds + i, 4);
    }
    increment_segment(s);

    /* update checkpoint region with the new location of the imap block  */ 
    fd_checkpoint = open("DRIVE/CHECKPOINT_REGION", O_RDWR, 0777);
    lseek(fd_checkpoint, map_index*sizeof(unsigned int), SEEK_SET);
    imap_loc = (unsigned int)seg_start*KB + block_start;
    write(fd_checkpoint, &imap_loc, sizeof(unsigned int)); 
    close(fd_checkpoint);
}



void CAT(char *command, segment *s, imap *map){
    int inode_num;
    inode dummy;
    unsigned int inode_block,inode_seg,inode_off;
    unsigned int block_num,block_seg,block_off;
    char lfs_filename[64];
    char garbage[64];
    char buffer[64];
    char segfile[64];
    char block;
    FILE *mappings;
    int tempfile;

    mappings = fopen("DRIVE/MAPPINGS", "r");

    sscanf(command,"%s %s",garbage,lfs_filename);

    //fetch inode number
    int i;
    for(i = 0; i < 10 * KB; i++){
        fscanf(mappings,"%s\n",buffer);
        if(strcmp(buffer,lfs_filename) == 0){
            inode_num = i;
            break;
        }
    }

    //calculate inode location
    inode_block = getInodeLoc(map,inode_num);
    inode_seg = findDiskSeg(inode_block);
    inode_off = findOffset(inode_block);

    //grab inode from mem
    sprintf(segfile,"DRIVE/SEGMENT%d",inode_seg);
    tempfile = open(segfile,O_RDWR);
    lseek(tempfile, inode_off*KB, SEEK_SET);
    read(tempfile, &dummy, sizeof(inode));
    close(tempfile);
    //read/print datablocks inode points to
    int remaining = dummy.file_size;
    for(i = 0; i < (dummy.file_size / KB) + 1; i++){
        block_num = dummy.datablocks[i];
        block_seg = findDiskSeg(block_num);
        block_off = findOffset(block_num);
        sprintf(segfile,"DRIVE/SEGMENT%d",block_seg);
        tempfile = open(segfile, O_RDWR, 0777);
        lseek(tempfile, block_off*KB, SEEK_SET);
        int j;
        if(remaining >= KB){
            for(j = 0; j < KB; j++){
                read(tempfile, &block, 1);
                fprintf(stdout,"%c",block);
            }
        }
        else{
            for(j = 0; j < remaining; j++){
                read(tempfile, &block, 1);
                fprintf(stdout, "%c", block);
            }
        }
        
        remaining -= KB;
        close(tempfile);
    }
    fprintf(stdout,"\n");
    fclose(mappings);
}


void DISPLAY(char *command, segment *s, imap *map){

    char garbage[MAX_SIZE_NAME];
    char lfs_filename[MAX_SIZE_NAME];
    int howmany, start;
    int fd_mappings, fd_segment;
    char c;
    char temp[64];
    unsigned int block_num;
    int i, j, disk_seg, offset;
    int starting_block, byte_offset, end;
    int inMemFlag;
    char *tracker;
    inode dummy;
    sscanf(command, "%s %s %d %d", garbage, lfs_filename, &howmany, &start);
    
    /* find the corresponding inode number */
    fd_mappings = open("DRIVE/MAPPINGS", O_RDWR, 0777);
    
    for(i = 0; i < 10*KB; i++){
        j = 0;
        while(read(fd_mappings, &c, 1) != 0){
            if(c != '\n'){
                if(c != ' ')
                    temp[j++] = c;
                else
                    temp[j++] = 0;
            } else
                break;
        }
        if(strcmp(lfs_filename, temp) == 0){
            block_num = *(map->inds + i);
            disk_seg = findDiskSeg(block_num); 
            offset = findOffset(block_num);

            /* check to see if it is in current in-memory segment */
            if(s->n == disk_seg){ //in current segment
                memcpy(&dummy, s->buffer + offset*KB, sizeof(inode));
            } else { //on disk
                sprintf(temp, "DRIVE/SEGMENT%d", disk_seg);
                fd_segment = open(temp, O_RDWR, 0777);
                lseek(fd_segment, offset*KB, SEEK_SET);
                read(fd_segment, &dummy, sizeof(inode));
                close(fd_segment);     
            }
            break;
        }
    }
    close(fd_mappings);
    
    /* the correct inode is now stored in the variable dummy ... the locations of the datablocks are within the datablocks array */
    starting_block = start / KB;
    byte_offset = start % KB;
    end = byte_offset + howmany;

    inMemFlag = 0;
    block_num = dummy.datablocks[starting_block];
    disk_seg = findDiskSeg(block_num);
    offset = findOffset(block_num);

    if(s->n == disk_seg){ //datablock in current in-memory segment
        tracker = s->buffer + offset*KB + byte_offset;
        inMemFlag = 1;
    } else { //datablock on disk
        sprintf(temp, "DRIVE/SEGMENT%d", disk_seg);  
        fd_segment = open(temp, O_RDWR, 0777);
        lseek(fd_segment, offset*KB + byte_offset, SEEK_SET);
    }

    for(i = byte_offset; i < end; i++){
        if((i % KB == 0) && (i != byte_offset)){ //data continues in next data block
            if(!inMemFlag)
                close(fd_segment);

            block_num = dummy.datablocks[++starting_block];
            disk_seg = findDiskSeg(block_num);
            offset = findOffset(block_num);

            if(s->n == disk_seg){
                tracker = s->buffer + offset*KB;
                inMemFlag = 1;
            } else {
                sprintf(temp, "DRIVE/SEGMENT%d", disk_seg);
                fd_segment = open(temp, O_RDWR, 0777);
                lseek(fd_segment, offset*KB, SEEK_SET);
                inMemFlag = 0;
            }
        }

        if(inMemFlag)
            memcpy(&c, tracker++, 1); 
        else
            read(fd_segment, &c, 1);
    
        write(STDOUT_FILENO, &c, 1);
    }

    c = '\n';
    write(STDOUT_FILENO, &c, 1);

    if(!inMemFlag)
        close(fd_segment);
}

void LIST(imap *map){

    int fd_mappings = open("DRIVE/MAPPINGS", O_RDWR, 0777);
    int i, j, disk_seg, offset;
    char temp[128];
    char c;
    char buffer[128];
    inode dummy;
    int tempfile;
    unsigned int block_num;
    for(i = 0; i < 10*KB; i++){
        j = 0;
        while(read(fd_mappings, &c, 1) != 0){
            if(c != '\n'){
                if(c != ' ')
                    temp[j++] = c;
                else
                    temp[j++] = 0;
            } else
                break;
        }
        if(strcmp("F", temp) != 0){
            block_num = *(map->inds + i);
            disk_seg = findDiskSeg(block_num); 
            offset = findOffset(block_num);
            sprintf(buffer, "DRIVE/SEGMENT%d", disk_seg);
            tempfile = open(buffer, O_RDWR);
            lseek(tempfile, offset*KB, SEEK_SET);
            read(tempfile, &dummy, sizeof(inode));

            fprintf(stdout, "filename: %s , filesize: %d \n", dummy.file_name, dummy.file_size);
            close(tempfile);
        }
    }
    close(fd_mappings);
}

void EXIT(segment *s){
    /* if nothing was written to in-memory segment ... don't need to write to disk! */
    if(s->b > NUM_SEG_SUM_BLOCKS)
        write_disk(s);
    exit(0);
}

void printUsage(){
    fprintf(stderr,"Usage: input [<options>] [<arguments> ...]\n");
    fprintf(stderr,"Options:\n");
    fprintf(stderr,"IMPORT <filename> <lfs_filename>\n");
    fprintf(stderr,"REMOVE <lfs_filename>\n");
    fprintf(stderr,"CAT <lfs_filename>\n");
    fprintf(stderr,"DISPLAY <lfs_filename> <howmany> <start>\n");
    fprintf(stderr,"OVERWRITE <lfs_filename> <howmany> <start> <c>\n");
    fprintf(stderr,"LIST\n");
    fprintf(stderr,"EXIT\n");
}


