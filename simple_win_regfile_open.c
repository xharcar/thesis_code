#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

char* fn = "C:\\Windows\\System32\\config\\SYSTEM";
char* fn2 = "C:/Windows/System32/config/SYSTEM";

int main()
{
    int desc;
    FILE* fd;
    desc = open(fn,O_RDONLY);
    if(desc == -1){
        printf("open : %d-%s \n",errno,strerror(errno));
    }
    fd = fopen(fn2,"rb");
    if(fd == NULL){
        printf("fopen : %d-%s \n",errno,strerror(errno));
    }
    if(desc== -1 || fd == NULL){
        puts("Error opening file as specified above. Exiting.\n");
        return 1;
    }
    fseek(fd,0,SEEK_END);
    int fsize = ftell(fd);
    char* data = calloc(fsize+1,sizeof(char));
    char* data2 = calloc(fsize+1,sizeof(char));
    puts("Allocation OK \n");
    rewind(fd);
    fread(data,sizeof(char),fsize,fd);
    if(ferror(fd)!=0){
        printf("fread failed");
        free(data);
        free(data2);
        return 2;
    }
    if((read(desc,data2,fsize))==-1){
        printf("%s \n",strerror(errno));
        free(data);
        free(data2);
        return 2;
    }
    printf("Data 1 : %s \n",data);
    printf("Data 2 : %s \n",data2);

    free(data);
    free(data2);
    sleep(5);
    return 0;
}
