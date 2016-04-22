#include <stdio.h>
#include <sys/resource.h>
#include <string.h>
#include <stdlib.h>
#include <linux/random.h>


int main(int argc,char** argv)
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
    for(unsigned long i=0;i<iter;i++){
        get_random_bytes((void*)data,size); // undefined reference(???)

    }
    struct rusage u;
    int c = getrusage(RUSAGE_SELF,&u);
    if(c == -1){
        perror("getrusage fail");
        return 2;
    }
    unsigned long s = u.ru_utime.tv_sec + u.ru_stime.tv_sec;
    unsigned long us = u.ru_utime.tv_usec + u.ru_stime.tv_usec;
    double t = s+((double)us/(double)1000000);
    printf("%lu * %lu B in %lf s\n",iter,size,t);
    memset(data,0,size);
    data[0] = 'a';// so memset doesn't get optimized away
    return 0;
}

