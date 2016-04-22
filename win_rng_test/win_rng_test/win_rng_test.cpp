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
#define STATUS_NOT_FOUND 0xC0000225
int main(int argc, char** argv)
{
	using namespace std::chrono;
	ULONG rep = strtoul(argv[1], NULL, 10);
	ULONG size = strtoul(argv[2], NULL, 10);
	/*FILE* out = fopen("win_rng_out_4", "wb");
	if (out == NULL) {
		std::cerr << "fopen fail" << std::endl;
		return 1;
	}*/
	PUCHAR data = (PUCHAR)calloc(size + 1, 1);
	if (data == NULL) {
		std::cerr << "alloc fail" << std::endl;
		return 2;
	}
	BCRYPT_ALG_HANDLE alg = NULL;
	NTSTATUS c = BCryptOpenAlgorithmProvider(&alg, BCRYPT_RNG_ALGORITHM, NULL, 0);
	if (c != 0) {
		std::cerr << "BCryptOpenAlgorithmProvider fail: ";
		switch (c) {
		case STATUS_NOT_FOUND: std::cerr << "provider not found";
			break;
		case STATUS_INVALID_PARAMETER: std::cerr << "invalid parameter";
			break;
		case STATUS_NO_MEMORY: std::cerr << "memory allocation failure";
		}
		std::cerr << std::endl;
		return 3;
	}
	double total = 0;
	for (ULONG i = 0; i < rep; i++) {
		memset(data, 0, size+1);
		high_resolution_clock::time_point start = high_resolution_clock::now();
		c = BCryptGenRandom(alg, data, size, 0);
		high_resolution_clock::time_point end = high_resolution_clock::now();
		if (c != 0) {
			std::cerr << "BCryptGenRandom fail: ";
			switch (c) {
			case STATUS_INVALID_HANDLE: std::cerr << "invalid RNG algorithm handle";
				break;
			case STATUS_INVALID_PARAMETER: std::cerr << "invalid parameter";
			}
			std::cerr << std::endl;
			return 4;
		}
		duration<double> time_taken = end - start;
		total += time_taken.count();
		/*ULONG oc = fwrite(data, 1, size, out);
		if (oc != size) {
			puts("fwrite fail");
			return 1;
		}
		fflush(out);*/
	}
	free(data);
	//fclose(out);
	std::cout << rep << " iterations of " << size << " B of random data in " << total << " s, avg: " << (double)(rep*size) / total << " Bps" << std::endl;
	return 0;
}


