// xpfurandom.c : Cross-platform (Windows/Linux) RNG wrapper;
// implementation of functions defined in xpfurandom.h
// Author : Matej Harcar, 422714@mail.muni.cz
// v0 : 17 Feb 2016 (incomplete version, never versioned online)
// v1 : 18 Feb 2016
// v2 : 19 Feb 2016
// v3 : 03 Apr 2016

// Notes: 
// Windows part
// - Must be called on IRQL PASSIVE_LEVEL due to BcryptOpenAlgorithmProvider

#include "xpfurandom.h"

int xpfurandom_prep(void* buffer, int size, void* win_alg_handle) {
	int s = size;
	if (s < 1) return 0xC0000001;
#ifdef _WIN32
	s = size + 1;// one extra byte on Windows for data amount checking via strlen()
	buffer = ExAllocatePoolWithTag(PagedPool, s, "BGNR");
	win_alg_handle = ExAllocatePoolWithTag(PagedPool, sizeof(void*), "AGNR");
	// First tag is reverse for "RNGB", "RNG Buffer", as recommended in MSDN docs;
	// Second is "RNGA", as in "RNG Algorithm"
	if (win_alg_handle = NULL) return 0xC0000001;
	// failure if memory for algorithm handle isn't allocated
	NTSTATUS st = BCryptOpenAlgorithmProvider(win_alg_handle, BCRYPT_RNG_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
	// BCRYPT_RNG_ALGORITHM is based on AES_CTR since Vista SP1, and Dual EC was never explicitly used 
	// (both according to Microsoft)
	if (st != 0) return 0xC0000001;
	// STATUS_SUCCESS is effectively equal to 0
#else // Linux
	win_alg_handle = NULL;
	buffer = kmalloc(size, GFP_KERNEL);
#endif // _WIN32
	// Conveniently both return NULL on failure
	if (buffer == NULL) return 0xC0000001;
	// STATUS_UNSUCCESSFUL as per MSDN- seems good enough as a failure indicator
	// is nonzero- good enough for classic Unix/Linux zero-returned error checks
	memset(buffer, 0, s);
	// memset is supposed to be kernel-OK on both Windows and Linux
	return 0;
}

int xpfurandom_get_random_data(void* buffer, int size, void* win_alg_handle) {
#ifdef _WIN32
	// strlen is dupposed to be kernel-OK
	while (strlen(buffer) < size) {
		NTSTATUS st = BCryptGenRandom(win_alg_handle, buffer + strlen(buffer), size - strlen(buffer), 1);
		if (st != 0) return st;
	}
#else // Linux
	get_random_bytes(buffer, size);
#endif // _WIN32
	char* x = (char*)buffer;
	int chk = 0xC0000001;
	for (int i = 0; i < size; i++) {
		if (x[i] != 0 && x[i] != 0xFF) chk = 0;
	} // simple check; if data is all 0/all 1, return failure
	return chk;
}

void xpfurandom_cleanup(void* buffer, int size, void* win_alg_handle) {
#ifdef _WIN32
	BCryptCloseAlgorithmProvider(win_alg_handle, 0);
	ExFreePool(win_alg_handle);
	RtlSecureZeroMemory(buffer, size);
	// zero memory so our random data does not stay up there
	// one byte may stay non-zeroed; unfortunate, but
	// more idiot-proof (in case someone puts X as size for allocation and X+1 for cleanup)
	ExFreePool(buffer);
#else // Linux
	memset(buffer, 0, size);
	char* x = (char*)buffer;
	x[0] = 'X'; // ensure memset does not get optimized away
	kfree(buffer);
#endif // _WIN32
}


//dummy Windows routines (please compile already- nope, VS just refuses)
#ifdef _WIN32
DRIVER_INITIALIZE DriverEntry;
DRIVER_ADD_DEVICE AddDevice;
DRIVER_UNLOAD Unload;

NTSTATUS DriverEntry(
	_In_ struct _DRIVER_OBJECT *DriverObject,
	_In_ PUNICODE_STRING       RegistryPath
)
{
	return 0;
}

NTSTATUS AddDevice(
	_In_ struct _DRIVER_OBJECT *DriverObject,
	_In_ struct _DEVICE_OBJECT *PhysicalDeviceObject
)
{
	return 0;
}

VOID Unload(
	_In_ struct _DRIVER_OBJECT *DriverObject
)
{
	return;
}

#endif // _WIN32