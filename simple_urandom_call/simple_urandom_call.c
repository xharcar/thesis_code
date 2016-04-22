#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

char* helpstring = "Valid arguments: \n \
                    n -- decimal <= 2147483647 = # of bytes to be read from /dev/urandom \n\
                    -h -- prints this";

int main(int argc, char *argv[])
{
    int data_amount;
    int data_read = 0;
    if(argv[1]==NULL){
        printf("No arguments found, run with -h for help");
        return -1;
    }
    if(strcmp(argv[1],"-h")==0){
        puts(helpstring);
        return 0;
    }
    else{
        data_amount = strtol(argv[1],NULL,10);
        if(data_amount == 0){
            puts("Invalid argument, reading default amount of data (1KB)");
            data_amount = 1024;
        }
    }

    int desc = open("/dev/urandom", O_RDONLY);
    // not every *nix/Linux-like OS has urandom
    if(desc == -1) {
        puts("Opening /dev/urandom failed, trying /dev/random");
        desc = open("/dev/random",O_RDONLY);
    }
    if(desc == -1){
        switch(errno){
            case EACCES:
                puts("Access denied, exiting.");
                break;
            case ENODEV:
            case ENXIO:
                puts("Device does not exist, exiting.");
                break;
            default:
                printf("Error: %s \n",strerror(errno));
        }
        return 1;
    }
    char* data = calloc(data_amount,1);
    // sizeof(char)==1
    if(data == NULL){
        puts("Memory allocation failed, exiting");
        return 2;
    }
    while(data_read<data_amount){
        int recv = read(desc,data+data_read,data_amount-data_read);
        if(recv == -1) {
            switch(errno){
                case EINTR:
                    recv = 0;
                    break;
                default:
                    printf("Error: %s \n",strerror(errno));
                    return 3;
            }
        }
        data_read = strlen(data);
    }//while-loop in case of non-fatal error(EINTR)
    printf("Data read from /dev/urandom: %d bytes",
         data_amount);
    for(int i=0;i<data_amount;i++){
        printf("%d  ",data[i]);
    }
    return 0;
}
