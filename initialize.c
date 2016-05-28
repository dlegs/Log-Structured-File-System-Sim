#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "defines.h"

int main(){
    int i, j, fd;
    unsigned int x;
    FILE *tempfile;
    char base[MAX_SIZE_NAME]; 
    //char buffer[15] = {'F', 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 10};
    //char c = '0';

    mkdir("./DRIVE", 0777);

    for(i = 0; i < NUM_SEGMENTS; i++){
        sprintf(base, "./DRIVE/SEGMENT%d", i); 
        //tempfile = fopen(base, O_RDWR | O_CREAT | O_TRUNC, 0777);
        tempfile = fopen(base, "w");
        for(j = 0; j < MB; j++){
            fprintf(tempfile, "%c", '0');
           // write(tempfile, &c, 1);
        } 
        fclose(tempfile);
    }


    sprintf(base, "./DRIVE/CHECKPOINT_REGION");
    fd = open(base, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    x = 0;
    /* write the initial states of all imap locations ... address of 0 */
    for(i = 0; i < NUM_IMAP_BLOCKS; i++){
        write(fd, &x, 4);
    } 
    close(fd);

    
    sprintf(base, "./DRIVE/MAPPINGS");
    fd = open(base, O_RDWR | O_CREAT | O_TRUNC, 0777);
    
    /* have a name that signals free inode ... when we give an inode out, we change the name to the name of the file ... need to leave a buffer space for long file names */    
    
    for(i = 0; i < MAX_NUM_FILES; i++){
        write(fd, "F             \n", 15);
    }
    close(fd);

    return 0;

}
