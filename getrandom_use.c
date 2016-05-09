// getrandom_use : simple getrandom usage example/throughput test/output to
// file (for dieharder/other purposes); can be compiled:
// using -std=c99, -std=c11, or without any switches
// Matej Harcar, 422714@mail.muni.cz, 2016-05-02
#include <linux/random.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

typedef unsigned long ulong;

int main(int argc, char** argv){
    ulong iter = strtoul(argv[1],NULL,10);
    ulong size = strtoul(argv[2],NULL,10);
    char* data = malloc(size);
    int c;
    if(data == NULL){
    	fprintf(stderr,"malloc fail\n");
    	return 1;
    }
    memset(data,0,size);
	FILE* out = fopen("getrandom_out_16gb.txt","wb");
	if(out == NULL){
		fprintf(stderr,"fopen:%s\n",strerror(errno));
		return 2;
	}
    for(int i = 0;i<iter;i++){
    c = syscall(SYS_getrandom,data,size,0);
    if(c!=0){
    	fprintf(stderr,"getrandom: %s\n",strerror(errno));
    	return 3;
    }
    c = fwrite(data,1,size,out);
    if(c!=size){
    	fprintf(stderr,"fwrite failed\n");
    	return 4;
    }
    fflush(out);
    }
    struct rusage u;
    c = getrusage(RUSAGE_SELF,&u);
    if(c!=0){
    	fprintf(stderr,"getrusage:%s\n",strerror(errno));
    	return 5;
    }
    double total_time = 0;
    total_time += u.ru_utime.tv_sec;
    total_time += ((double)u.ru_utime.tv_usec/(double)1000000);
    total_time += u.ru_stime.tv_sec;
    total_time += ((double)u.ru_stime.tv_usec/(double)1000000);
    printf("%lu x %lu B in %lf sec\n",iter,size,total_time);
    memset(data,0,size);
    data[0] = 'a';
    free(data);
    fclose(out);
    return 0;
}

