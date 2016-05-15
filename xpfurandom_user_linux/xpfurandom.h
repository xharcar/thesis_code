/*
xpfurandom.h- Defines usermode XPFUrandom functions
Author: Matej Harcar, 422714@mail.muni.cz
v0 : 17 Feb 2016 (incomplete version, never versioned online)
v1 : 18 Feb 2016
v2 : 19 Feb 2016
v3 : 14-15 May 2016
*/

#ifndef _XPFURANDOM_H
#define _XPFURANDOM_H

#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
// Visual Studio harps up about fopen(), with which there is no problem

// Return codes and constants
#define OK 0
#define FAIL -1
#define HELPED -2 // for when help is printed
#define ARG_FILE "-f"
#define ARG_HELP "-h"
#define ARG_SIZE "-s"
#define HELP_TEXT "XPFUrandom user mode \n \
Accepted command-line arguments: \n\
-b: on Linux, blocks (by default, doesn't(GRND_NONBLOCK specified by default))\n\
	ignored on Windows\n\
-f <filename>: sets filename to output data to(you still have to do file ops\
	, this is just for convenience)\n\
-h: prints help \n\
-r: on Linux, reads from /dev/random pool(reads from /dev/urandom pool by\n\
	default); ignored on Windows\n\
-s <size>: sets random data buffer size in bytes; default is 256\n"


// Common includes
#include <stdio.h>
#include <string.h> // strcmp
#include <stdlib.h> // strtoul
#include <errno.h> 
#include <stdint.h> // uint32_t, INT32_MAX
#include <ctype.h> // isgraph

#ifdef _WIN32
#include <Windows.h>
#include <bcrypt.h> // BCryptXxx
#pragma comment (lib,"bcrypt.lib")
#define STATUS_SUCCESS 0x0
#define STATUS_NOT_FOUND 0xC0000225
#else //Linux
#define _GNU_SOURCE
#define ARG_BLOCK "-b"
#define ARG_RAND "-r"
#include <linux/random.h> // getrandom
#include <unistd.h> // syscall w/ sys/syscall.h
#include <sys/syscall.h> //syscall w/ unistd.h
#endif // _WIN32

typedef struct{
	unsigned int mode;
	char* filename;
	FILE* file_out;
	uint32_t size;
}xpfurandom_settings;

/*
Parses command-line arguments
c: argument count
v: string arguments
s: struct containing saved settings
Intended use: pass argc and argv from main
Accepted arguments:
-b: On Linux, blocks when entropy estimate is too low(does not block by default);
	ignored on Windows
-f <filename>: filename to save random data to; neither specified by default nor required;
	MUST NOT start with a "-"(minus); purely for convenience's sake
-h: ignores all other arguments, prints help and exits
-s <size>: sets size as desired amount of random data; 256 B by default
	accepted range: 0 < size <= 2147483647(INT32_MAX) - assuming 32 bit integer
Special return value: -2 when "-h" is specified
*/
int xpfurandom_parse_args(int c, char** v,xpfurandom_settings* s);

/*
Prepares necessary things
data: pointer to random data buffer
size: desired amount of random data
win_alg_handle: pointer to a BCRYPT_ALG_HANDLE(glorified PVOID aka void*)
 - ignored on Linux
Intended use: declare necessary variables, define size, call this
*/
int xpfurandom_prep(void** data, uint32_t size, void** win_alg_handle,xpfurandom_settings* s);

/*
Acquires random data from system CSPRNG
data: pointer to random data buffer
size: desired amount of random data
win_alg_handle: pointer to a BCRYPT_ALG_HANDLE(glorified PVOID aka void*)
- ignored on Linux
Intended use: right after prep
Special return values: 
-on Linux, if reading from /dev/random pool without blocking
and less random bytes than requested were acquired, number of bytes of random
data acquired
*/
int xpfurandom_get_random_data(void** data, uint32_t size, void* win_alg_handle,xpfurandom_settings* s);

/*
Cleans up random data, frees memory
data: pointer to random data buffer
size: desired amount of random data
win_alg_handle: pointer to a BCRYPT_ALG_HANDLE(glorified PVOID aka void*)
- ignored on Linux
Intended use: before exiting application
*/
int xpfurandom_cleanup(void** data, uint32_t size, void** win_alg_handle,xpfurandom_settings* s);

#endif // !_XPFURANDOM_H
