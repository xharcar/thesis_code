// win_rng_test.c : Simple test program to test BCryptGenRandom's throughput
// and if necessary, return its output in a file

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <bcrypt.h>
#include <cstdio>
#pragma comment (lib,"bcrypt.lib")
#include <cstring>
#include <chrono>
#define KILO 1024
#define MEGA KILO*KILO // 1048576
#define GIGA MEGA*KILO // 1024^3 = 2^30 = (INT_MAX+1)/2
int main(int argc, char** argv)
{
	using namespace std::chrono;
	ULONG rep = strtoul(argv[1], NULL, 10);
	ULONG size = strtoul(argv[2], NULL, 10);
	FILE* out = fopen("win_rng_out.txt", "wb");
	PUCHAR data = (PUCHAR)calloc(size + 1, 1);
	BCRYPT_ALG_HANDLE *alg = (BCRYPT_ALG_HANDLE*)malloc(sizeof(BCRYPT_ALG_HANDLE));
	NTSTATUS c = BCryptOpenAlgorithmProvider(alg, BCRYPT_RNG_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
	double total = 0;
	for (ULONG i = 0; i < rep; i++) {
		memset(data, 0, size+1);
		ULONG g = 0;
		high_resolution_clock::time_point start = high_resolution_clock::now();
		do{
			BCryptGenRandom(*alg, data + g, size - g, 0);
			g = strlen((PCHAR)data);
		} while (g < size);
		high_resolution_clock::time_point end = high_resolution_clock::now();
		duration<double> time_taken = end - start;
		ULONG oc = fwrite(data, 1, size, out);
		if (oc != size) {
			puts("fwrite done goofed\n");
			return 1;
		}
		fflush(out);
		total += time_taken.count();
	}
	free(data);
	fclose(out);
	std::cout << rep << " iterations of " << size << " B of random data in " << total << " s, avg: " << (double)(rep*size) / total << " Bps" << std::endl;
	return 0;
}


