// xpfurandom_kernel.c : Cross-platform (Windows/Linux) 
// kernel mode RNG wrapper;
// implementation of functions defined in xpfurandom_kernel.h
// Author : Matej Harcar, 422714@mail.muni.cz
// v0 : 17 Feb 2016 (incomplete version, never versioned online)
// v1 : 18 Feb 2016
// v2 : 19 Feb 2016
// v3 : 03/04 Apr 2016
// v4 : 11 Apr 2016
// v5 : 07 May 2016

// Notes: 
// Windows part
// - Must be called on IRQL PASSIVE_LEVEL due to BcryptOpenAlgorithmProvider

#include "xpfurandom_kernel.h"

#ifdef _WIN32
#define OK STATUS_SUCCESS
#define FAIL STATUS_UNSUCCESSFUL

static int xpfurandom_kernel_prep(void** buffer, int size, void** win_alg_handle) {
	*buffer = ExAllocatePoolWithTag(PagedPool, size, 'BGNR');
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

static int xpfurandom_kernel_get_random_data(void** buffer, int size, void** win_alg_handle) {
	// assuming prep() returned 0 and memset went through
	NTSTATUS st = BCryptGenRandom(*win_alg_handle, buf_str, size, 0);
	if (st != 0) return st;
	return OK;
}

static void xpfurandom_kernel_cleanup(void** buffer, int size, void** win_alg_handle) {
	BCryptCloseAlgorithmProvider(*win_alg_handle, 0);
	ExFreePool(*win_alg_handle);
	RtlSecureZeroMemory(*buffer, size);
	// zero memory so our random data does not stay up there
	// one byte may stay non-zeroed; unfortunate, but
	// more idiot-proof (in case someone puts X as size for allocation and X+1 for cleanup)
	ExFreePool(*buffer);
}
#else // Linux

static int xpfurandom_kernel_prep(void** buffer, int size, void** win_alg_handle) {
	pr_alert("XPFUrandom: kernel_prep launched\n");
	*buffer = kmalloc(size, GFP_KERNEL);
	if (*buffer == NULL) return 1;
	printk(KERN_ALERT "XPFUrandom: kernel memory allocated\n");
	return 0;
}
EXPORT_SYMBOL_GPL(xpfurandom_kernel_prep);

static int xpfurandom_kernel_get_random_data(void** buffer, int size, void** win_alg_handle) {
	int i;
	int8_t* x = (int8_t*) *buffer;
	pr_alert("XPFUrandom: kernel_get_random_data running\n");
	get_random_bytes(*buffer, size); 
	pr_alert("XPFUrandom: data follows\n");
	for(i = 0;i < size;i++)	pr_alert("%x ",x[i]);
	// has void return, hopefully does not fail
	pr_alert("XPFUrandom: random data acquired\n");
	return 0;
}
EXPORT_SYMBOL_GPL(xpfurandom_kernel_get_random_data);

static int xpfurandom_kernel_cleanup(void** buffer, int size, void** win_alg_handle) {
	char* x = (char*) *buffer;
	pr_alert("XPFUrandom: kernel_cleanup\n");
	memset(x,0,size);
	if(x[1] != 0) {
		return -1;
	}
	kfree(*buffer);
	pr_alert("XPFUrandom: cleanup done\n");
	return 0;
}
EXPORT_SYMBOL_GPL(xpfurandom_kernel_cleanup);

MODULE_AUTHOR("Matej Harcar, 422714@mail.muni.cz");
MODULE_DESCRIPTION("XPFUrandom Kernel test module");
MODULE_LICENSE("GPL");

#endif // _WIN32

