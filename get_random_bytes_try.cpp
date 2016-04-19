#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <linux/random.h>

int main(int argc, char** argv)
{
    argc=argc;
    unsigned long iter = strtoul(argv[1],NULL,10);
    unsigned long bs = strtoul(argv[2],NULL,10);
    void* buf = calloc(bs+1,1);
    double count = 0;
    for(unsigned long i=0;i<iter;i++){
        auto t1 = std::chrono::high_resolution_clock::now();
        get_random_bytes(buf,bs);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> x = t2-t1;
        count+=x.count();
    }
    printf("%lu iterations of %lu random bytes: %lf ticks",iter,bs,count);
    return 0;
}
