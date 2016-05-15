/* 
xpfurandom_kernel.c : Cross-platform (Windows/Linux) 
kernel mode RNG wrapper;
implementation of functions defined in xpfurandom_kernel.h
Author : Matej Harcar, 422714@mail.muni.cz
v0 : 17 Feb 2016 (incomplete version, never versioned online)
v1 : 18 Feb 2016
v2 : 19 Feb 2016
v3 : 03/04 Apr 2016
v4 : 11 Apr 2016
v5 : 07 May 2016
v6 : 11-13 May 2016

Notes: 
Windows part
- Must be called on IRQL PASSIVE_LEVEL due to BcryptOpenAlgorithmProvider
*/
#include "xpfurandom_kernel.h"

#ifdef _WIN32
int xpfurandom_kernel_prep(void** buffer, int size, void** win_alg_handle) {
	*buffer = ExAllocatePoolWithTag(PagedPool, size, 'BGNR');
	*win_alg_handle = ExAllocatePoolWithTag(PagedPool, sizeof(PVOID), 'AGNR');
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "XPFUrandom: Kernel prep\n"));
	if (*win_alg_handle == NULL) {
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "XPFUrandom: Kernel prep failed: \
			algorithm handle space allocation failed\n"));
		return FAIL;
	}
	NTSTATUS st = BCryptOpenAlgorithmProvider(*win_alg_handle, BCRYPT_RNG_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
	if (st != 0) {
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "XPFUrandom: Kernel prep failed: \
			BCryptOpenAlgorithm failed:"));
		switch (st) {
		case STATUS_NOT_FOUND: KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "No suitable provider\
			found(STATUS_NOT_FOUND)\n"));
			break;
		case STATUS_INVALID_PARAMETER: KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Invalid \
			parameter(STATUS_INVALID_PARAMETER)\n"));
			break;
		case STATUS_NO_MEMORY: KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Memory allocation \
			failure(STATUS_NO_MEMORY)\n"));
			break;
		}
		return FAIL;
	}
	if (*buffer == NULL) {
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "XPFUrandom: Kernel prep failed: \
			random data space allocation failed\n"));
		return FAIL;
	}
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "XPFUrandom: Kernel prep OK\n"));
	return OK;
}

int xpfurandom_kernel_get_random_data(void** buffer, int size, void** win_alg_handle) {
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "XPFUrandom: Getting random data in kernel\n"));
	NTSTATUS st = BCryptGenRandom(*win_alg_handle, *buffer, size, 0);
	switch (st) {
	case STATUS_SUCCESS: KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "XPFUrandom: Random data successfully acquired\n"));
		break;
	case STATUS_INVALID_HANDLE: {KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "XPFUrandom: BCryptGenRandom failed: \
		invalid algorithm handle(STATUS_INVALID_HANDLE)\n"));
		return FAIL;
		}break;
	case STATUS_INVALID_PARAMETER: {KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "XPFUrandom: BCryptGenRandom failed: \
		invalid parameter(STATUS_INVALID_PARAMETER)\n"));
		return FAIL;
		}break;
	}
	return OK;
}

int xpfurandom_kernel_cleanup(void** buffer, int size, void** win_alg_handle) {
	char* x = (char*) *buffer;
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "XPFUrandom: cleaning up\n"));
	BCryptCloseAlgorithmProvider(*win_alg_handle, 0);
	RtlSecureZeroMemory(*buffer, size);
	if (x[0] != 0) {
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "XPFUrandom: cleanup failed\n"));
		return FAIL;
	}
	ExFreePool(*buffer);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "XPFUrandom: cleanup OK\n"));
	return OK;
}
#else // Linux

int xpfurandom_kernel_prep(void** buffer, int size, void** win_alg_handle) {
	pr_notice("XPFUrandom: kernel_prep launched\n");
	*buffer = kmalloc(size, GFP_KERNEL);
	if (*buffer == NULL) {
		pr_err("XPFUrandom: kernel memory allocation failure\n");
		return FAIL;
	}
	pr_notice("XPFUrandom: kernel memory allocated\n");
	return OK;
}
EXPORT_SYMBOL_GPL(xpfurandom_kernel_prep);

int xpfurandom_kernel_get_random_data(void** buffer, int size, void** win_alg_handle) {
	pr_notice("XPFUrandom: kernel_get_random_data running\n");
	get_random_bytes(*buffer, size); 
	// has void return, hopefully does not fail
	pr_notice("XPFUrandom: random data acquired\n");
	return OK;
}
EXPORT_SYMBOL_GPL(xpfurandom_kernel_get_random_data);

int xpfurandom_kernel_cleanup(void** buffer, int size, void** win_alg_handle) {
	char* x = (char*) *buffer;
	pr_notice("XPFUrandom: kernel_cleanup\n");
	memset(x,0,size);
	if (x[0]) {
		pr_err("XPFUrandom: kernel cleanup(memory zeroing) failed\n");
		return FAIL;
	}
	kfree(*buffer);
	pr_notice("XPFUrandom: cleanup done\n");
	return OK;
}
EXPORT_SYMBOL_GPL(xpfurandom_kernel_cleanup);

//MODULE_AUTHOR("Matej Harcar, 422714@mail.muni.cz");
//MODULE_DESCRIPTION("XPFUrandom Kernel test module");
//MODULE_LICENSE("GPL");

#endif // _WIN32

