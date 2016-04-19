// xpfurandom.h (Cross-platform (u)random)
// Declares kernel-mode-runnable functions (implemented in xpfurandom.c) 
// that can be used on both Windows and Linux
// Author: Matej Harcar, 422714@mail.muni.cz
// v1: 03/04 Apr 2016

#ifndef _XPFURANDOM_H
#define _XPFURANDOM_H

#ifdef _WIN32
#include <wdm.h>
// Add path to wdm.h in project settings in VS
#include <bcrypt.h>
#include <string.h>
#pragma comment (lib, "bcrypt.lib")
#pragma comment (lib, "ntoskrnl.lib")
// Add path to ntoskrnl.lib in project settings in VS
#else // Linux
#include <linux/slab.h> // kmalloc
#include <linux/random.h> // get_random_bytes
#include <linux/string.h> // memset
#include <linux/errno.h> // errno

#endif // _WIN32

// Acquires resources (memory/handles)
// buffer: pointer to memory to prepare
// size : desired memory size 
// (please don't try allocating more than 4 kB)
// win_alg_handle : RNG algorithm handle to use on Windows; reset to NULL on Linux
// RNG_ALGORITHM_HANDLE is typedef'ed as PVOID aka void*;
// return value : 0 on success, 0xC0000001 on failure
int xpfurandom_prep(void* buffer, int size, void* win_alg_handle); 

// Retrieves random data by calling system-specific CSPRNG
// buffer : memory address to write retrieved data to
// size : amount of random data to retrieve
// return value: on success, 0;
// on BCryptGenRandom failure, the corresponding error code, see MSDN docs
// on all-zero/all-one data, 0xC0000001
int xpfurandom_get_random_data(void* buffer, int size, void* win_alg_handle);

// Cleans up, can be used in module exit functions
void xpfurandom_cleanup(void* buffer, int size, void* win_alg_handle);

#endif // !_XPFURANDOM_H