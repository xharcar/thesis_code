#include "xpfurandom_kernel.h"
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD Unload;

NTSTATUS DriverEntry(_In_ struct _DRIVER_OBJECT *DriverObject,_In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status;
	int size = 256;
	void* data;
	void* alg_handle;
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "XPFUrandom: DriverEntry\n"));
	status = xpfurandom_kernel_prep(&data, size, &alg_handle);
	if (status != 0) return STATUS_UNSUCCESSFUL;
	status = xpfurandom_kernel_get_random_data(&data, size, &alg_handle);
	if (status != 0) return STATUS_UNSUCCESSFUL;
	status = xpfurandom_kernel_cleanup(&data, size, &alg_handle);
	if (status != 0) return STATUS_UNSUCCESSFUL;
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "XPFUrandom: DriverEntry OK\n"));
	return STATUS_SUCCESS;
}

VOID Unload(_In_ struct _DRIVER_OBJECT *DriverObject){
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "XPFUrandom: Unload\n"));
}
