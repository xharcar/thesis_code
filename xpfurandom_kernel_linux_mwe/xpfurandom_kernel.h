/*
xpfurandom_kernel.h (Cross-platform (u)random)
Declares kernel-mode-runnable functions (implemented in xpfurandom_kernel.c) 
that can be used on both Windows and Linux
Author: Matej Harcar, 422714@mail.muni.cz
v1: 03/04 Apr 2016
v2: 07 May 2016
v3: 11-13 May 2016
*/

#ifndef _XPFURANDOM_KERNEL_H
#define _XPFURANDOM_KERNEL_H

// Return values
#define OK 0
#define FAIL -1

#ifdef _WIN32
#include <ntddk.h>
#include <wdm.h>
#include <bcrypt.h>
#include <string.h>
#pragma comment (lib, "Cng.lib")
#else // Linux
#include <linux/module.h> // kernel mode
#include <linux/slab.h> // kmalloc
#include <linux/random.h> // get_random_bytes
#include <linux/string.h> // memset
#include <linux/errno.h> // errno
#endif // _WIN32

/*
Acquires resources (memory/handles)
buffer: pointer to memory to prepare
size : desired buffer size
win_alg_handle : RNG algorithm handle to use on Windows; ignored on Linux
return value : 0 on success, -1 on failure
*/
int xpfurandom_kernel_prep(void** buffer, int size, void** win_alg_handle); 

/*
Retrieves random data by calling system-specific CSPRNG
buffer : memory address to write retrieved data to
size : amount of random data to retrieve
win_alg_handle : prepared Windows RNG algorithm handle; ignored on Linux
return value : 0 on success, -1 on failure
*/
int xpfurandom_kernel_get_random_data(void** buffer, int size, void** win_alg_handle);

/*
Cleans up
buffer: memory to clean up(zero)
size: size of buffer
win_alg_handle : Windows algorithm handle to be freed; ignored on Linux
return value: 0 on success(memory zeroed, checked on first byte), -1 on failure
*/
int xpfurandom_kernel_cleanup(void** buffer, int size, void** win_alg_handle);


#endif // !_XPFURANDOM_KERNEL_H
