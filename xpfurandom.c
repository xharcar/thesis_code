// xpfurandom.c : Cross-platform (Windows/Linux) RNG wrapper;
// implementation of functions defined in xpfurandom.h
// Author : Matej Harcar, 422714@mail.muni.cz
// v0 : 17 Feb 2016 (incomplete version, never versioned online)
// v1 : 18 Feb 2016
// v2 : 19 Feb 2016
// v3 : 03/04 Apr 2016
// v4 : 11 Apr 2016

// Notes: 
// Windows part
// - Must be called on IRQL PASSIVE_LEVEL due to BcryptOpenAlgorithmProvider

#include "xpfurandom.h"

#ifdef _WIN32
#define OK STATUS_SUCCESS
#define FAIL STATUS_UNSUCCESSFUL

int xpfurandom_prep(void** buffer, int size, void** win_alg_handle) {
	int s = size + 1;
	*buffer = ExAllocatePoolWithTag(PagedPool, s, 'BGNR');
	*win_alg_handle = ExAllocatePoolWithTag(PagedPool, sizeof(PVOID), 'AGNR');
	// First tag is reverse for "RNGB", "RNG Buffer", as recommended in MSDN docs;
	// Second is "RNGA", as in "RNG Algorithm"
	if (*win_alg_handle == NULL) return FAIL;
	// failure if memory for algorithm handle isn't allocated
	NTSTATUS st = BCryptOpenAlgorithmProvider(*win_alg_handle, BCRYPT_RNG_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
	// BCRYPT_RNG_ALGORITHM is based on AES_CTR since Vista SP1, and Dual EC was never explicitly used 
	// (both according to Microsoft), sure hope that's true
	if (st != 0) return st;
	// STATUS_SUCCESS is effectively equal to 0
	if (*buffer == NULL) return FAIL;
	memset(*buffer, 0, s);
	return OK;
}

int xpfurandom_get_random_data(void** buffer, int size, void** win_alg_handle) {
	// assuming prep() returned 0 and memset went through
	char* buf_str = (char*)*buffer;
	int n = strlen(buf_str);
	while (n < size) {
		NTSTATUS st = BCryptGenRandom(*win_alg_handle, buf_str + n, size - n, 0);
		if (st != 0) return st;
		n = strlen(buf_str);
	}
	return OK;
}

void xpfurandom_cleanup(void** buffer, int size, void** win_alg_handle) {
	BCryptCloseAlgorithmProvider(*win_alg_handle, 0);
	ExFreePool(*win_alg_handle);
	RtlSecureZeroMemory(*buffer, size);
	// zero memory so our random data does not stay up there
	// one byte may stay non-zeroed; unfortunate, but
	// more idiot-proof (in case someone puts X as size for allocation and X+1 for cleanup)
	ExFreePool(*buffer);
}
#else
int xpfurandom_prep(void** buffer, int size, void** win_alg_handle) {
	*buffer = kmalloc(size, GFP_KERNEL);
	if (*buffer == NULL) return 1;
	memset(*buffer, 0, s);
	return 0;
}

int xpfurandom_get_random_data(void** buffer, int size, void** win_alg_handle) {
	get_random_bytes(*buffer, size); // has void return, hopefully does not fail
	return 0;
}

void xpfurandom_cleanup(void* buffer, int size, void* win_alg_handle) {
	memset(buffer, 0, size);
	char* x = (char*)buffer;
	x[1] = 'X'; // ensure memset does not get optimized away
	kfree(buffer);
}

#endif // _WIN32


// dummy main to trick compiler into doing its job, and for testing(TBD)
int main(void) {
	int s = 0;
	int c = 1024;
	void* buf = NULL;
	void* ah = NULL;
	s = xpfurandom_prep(&buf, c, &ah);
	if (s != 0) return 1;
	s = xpfurandom_get_random_data(&buf, c, &ah);
	if (s != 0) return 2;
	xpfurandom_cleanup(&buf, c, &ah);
	return 0;
}