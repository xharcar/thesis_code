#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>

int main(int argc, char** argv)
{
    if(argc < 3){
        perror("not enough arguments\n");
        return -1;
    }
    unsigned long iter = strtoul(argv[1],NULL,10);
    unsigned long size = strtoul(argv[2],NULL,10);
    char* data = malloc(size);
    if(data == NULL){
        perror("malloc fail\n");
        return 1;
    }
    int urandom_desc = open("/dev/urandom",O_RDONLY);
    if(urandom_desc == -1){
        perror("open fail");
        return 2;
    }
    for (unsigned long i=0;i<iter;i++){
        read(urandom_desc,data,size);
    }
    struct rusage u;
    int c = getrusage(RUSAGE_SELF,&u);
    if(c == -1){
        perror("getrusage fail\n");
        return 3;
    }
    unsigned long s = u.ru_utime.tv_sec + u.ru_stime.tv_sec;
    unsigned long us = u.ru_utime.tv_usec + u.ru_stime.tv_usec;
    double t = s+((double)us/(double)1000000);
    printf("Urandom:%lu * %lu B in %lf s\n",iter,size,t);
    memset(data,0,size);
    data[0] = 'a';
    return 0;
}

