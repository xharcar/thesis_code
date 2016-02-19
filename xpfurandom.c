// xpfurandom.c : Cross-platform (Win/Unix/GNU/Mac OS) wrapper for /dev/urandom and Windows RNG source
// Author : Matej Harcar, 422714@mail.muni.cz
// v1 : 17 Feb 2016
// v2 : 18 Feb 2016

// Visual Studio harps up about fopen(), this suppresses it
// fopen() is required for compatibility
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

// Standard C includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

// Unix/GNU/Mac OS includes
#ifndef _WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
// /dev/urandom descriptor set by prep() function
int urandom_desc;
#else
// Windows includes
#include <Windows.h>
#define STATUS_SUCCESS 0x00000000
// Cannot include winnt.h, macro redefinitions
#include <bcrypt.h>
#pragma comment (lib, "bcrypt.lib")
// RNG algorithm handle obtained from BCryptOpenAlgorithmProvider() called by prep()
BCRYPT_ALG_HANDLE *rng_alg_handle;
#endif // !_WIN32

char* filename = NULL;
FILE* open_file;
int data_amount = 128;
void* data;

// Help string
char* helptext = "XPFUrandom : Cross-Platform urandom and Windows RNG Wrapper \n\
				Valid arguments :\n\
				-h : prints this text(must be first and only argument, else ignored)\n\
				-f (filename) : writes acquired random data to specified file; if unspecified, writes to console; does NOT accept \"-f\" as a valid filename\n\
				-n (number) : acquires given amount of data (in bytes,<2'147'483'647); if unspecified, retrieves 128 B (base RSA key size,1 kb)\n\
				-d : default setting; retrieves 128 B and writes to console, must be first and only argument, else ignored\n\
				Unknown arguments are not handled";

// Prepares descriptor for reading data on Unix-like
// Acquires an RNG algorithm handle on Windows
// Allocates memory for random data
// Opens a file descriptor if necessary
int prep(void) {
	int rv = 0;
#ifndef _WIN32
	urandom_desc = open("/dev/urandom", O_RDONLY);
	rv = urandom_desc;
#else
	NTSTATUS status = BCryptOpenAlgorithmProvider(rng_alg_handle, BCRYPT_RNG_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
	// this wouldn't be needed on Win7+; however Vista does not support 
	// BCryptGenRandom(NULL,buffer,number,x) where x & 0x00000002 == 0x2;
	// the parameter combination means "Use system-preferred RNG";
	// this ensures backwards compatibility with it
	if (status != STATUS_SUCCESS) rv = -1;
#endif // !_WIN32
	data = malloc(data_amount + 1);
	if (data == NULL) rv -= 2;
	else memset(data, 0, data_amount + 1);	
	if (filename != NULL) {
		open_file = fopen(filename, "w");
		if (open_file == NULL) rv -= 4;
	}
	return rv;
}

// Retrieves random data by calling system-specific CSPRNG
// pos : memory address to write retrieved data to
// size : amount of random data to retrieve
int get_random_data(void* pos, int size) {
	int r = 0;
	while (r < size) {

#ifndef _WIN32
		int r2 = read(urandom_desc, pos+r, size-r);
		if (r2 < size && r2 != -1) {
			r += r2;
		}
		else if (r2 == -1 && errno != EINTR) {
			return -1;
		}
		//on read() return -1 with EINTR, continues (r2 > size will not occur)
#else
		NTSTATUS status = BCryptGenRandom(rng_alg_handle, (PUCHAR)pos+r, (ULONG)size-r, 0);
		// 0x00000001 flag would have just been ignored on Win8+, so the "extra entropy count" most likely doesn't matter (a lot);
		// 0x00000002 flag see comments in prep()
		if (status != STATUS_SUCCESS) {
			return (int)status;
			// NTSTATUS defined as LONG, which is typedef'ed as long; however NTSTATUS codes are all 8 hex digits,
			// indicating a 32-bit domain, which on x86 is the same as int; therefore this is a safe cast
		}
		r = strlen((char*)pos);
		// can use strlen() since memory is set to zeroes with length size+1 for null termination; this is specifically done
		// in case BCryptGenRandom does not retrieve enough data in one go
#endif // !_WIN32
	}
	return 0;
}

// Cleans up:
// Frees memory allocated for random data, closes open file descriptor
// On Windows, closes RNG algorithm provider,
// On Unix-like, closes descriptor of /dev/urandom
void cleanup() {
	free(data);
	fclose(open_file);
#ifndef _WIN32
	close(urandom_desc);
#else
	BCryptCloseAlgorithmProvider(rng_alg_handle, 0);
#endif // !_WIN32
}

// Parses command-line arguments given to program;
// Would have preferred getopt(), but Windows being Windows, that's a no-go.
// Arguments are argc and argv, respectively. Return values: 
// -1 on successful exit with at least one of (filename,data_amount) correctly set
// 0 if -h argument was given (as first and only argument)
// 1 if no valid arguments were detected
// 2 if exactly one argument was given, but it was neither -h nor -d
// 3 if -f was given with no filename
// 4 if -n was given with no data amount
// 5 if -n was given with invalid data amount (more than 2147483647 or less than 0)
int parse_args(int c, char* v[]) {
	if (c < 2) {
		fprintf(stderr, "No valid arguments detected. Please run with -h.\n");
		return 1;
	}
	if (c == 2) {
		if (strcmp(v[1], "-h") == 0) {
			puts(helptext);
			return 0;
		}
		else if (strcmp(v[1], "-d") != 0) {
			fprintf(stderr, "Invalid arguments detected. Run with -h and/or check given arguments.\n");
			return 2;
		}
	}

	for (int i = 1; i < c; i++) {
		if (strcmp(v[i], "-f") != 0 && strcmp(v[i], "-n") != 0) continue;
		if (strcmp(v[i], "-f") == 0) {
			if (   i + 1 == c
				|| strcmp(v[i + 1], "-d") == 0
				|| strcmp(v[i + 1], "-h") == 0
				|| strcmp(v[i + 1], "-n") == 0) {
				fprintf(stderr, "No filename given.\n");
				return 3;
			}
			else if (strcmp(v[i + 1], "-f") == 0) continue;
			else filename = v[i + 1];
		}
		if (strcmp(v[i], "-n") == 0) {
			if (i + 1 == c) {
				fprintf(stderr, "No data amount given.\n");
				return 4;
			}
			long x = strtol(v[i+1],NULL,10);
			if (x <= 0 || x == LONG_MAX) {
				fprintf(stderr, "Invalid data amount input.\n");
				return 5;
			}
		}

	}
	return -1;
}

//Main 
int main(int argc, char* argv[]) {
	int r;
	if ((r = parse_args(argc, argv)) >= 0) return r;

	if (prep() < 0) {
		// Can check for negative, since NTSTATUS error(and even warning) codes all start with 0xC, 0x8 respectively;
		// therefore when cast to int(as prep() does), whatever decimal number they are is negative
		// TODO/optional : return code switch
		fprintf(stderr, "Resource preparation failed. Exiting.\n");

		return 6;
	}

	if (get_random_data(data,data_amount) < 0) {
		// TODO/optional : error code switch
		fprintf(stderr, "Random data retrieval failed. Exiting.\n");
		cleanup();
		return 7;
	}

	if (filename == NULL) {
		printf("%s\n", (char*)data);
	}
	else {
		fprintf(open_file, "%s", (char*)data);
	}

	cleanup();
	return 0;
}