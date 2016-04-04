// xpfurandom.c : Cross-platform (Win/Unix/GNU/Mac OS) wrapper for /dev/urandom and Windows RNG source
// Author : Matej Harcar, 422714@mail.muni.cz
// v0 : 17 Feb 2016 (incomplete version, never versioned online)
// v1 : 18 Feb 2016
// v2 : 19 Feb 2016

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
int random_desc;
#else
// Windows includes
#include <Windows.h>
#define STATUS_SUCCESS 0x0
#define STATUS_NOT_FOUND 0xC0000225 // as per https://msdn.microsoft.com/en-us/library/cc704588.aspx
// Cannot include winnt.h, macro redefinitions
#include <bcrypt.h>
#pragma comment (lib, "bcrypt.lib")
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
#ifndef _WIN32
	random_desc = open("/dev/urandom", O_RDONLY);
	if (random_desc == -1) {
		fprintf(stderr, "Opening /dev/urandom failed, trying /dev/random\n");
		random_desc = open("/dev/random", O_RDONLY);
		if (random_desc == -1) {
			return -1;
		}
	}
#endif // !_WIN32
	data = malloc(data_amount + 1);
	if (data == NULL) return -2;
	else memset(data, 0, data_amount + 1);	
	if (filename != NULL) {
		open_file = fopen(filename, "w");
		if (open_file == NULL) return -3;
	}
	return 0;
}

// Retrieves random data by calling system-specific CSPRNG
// pos : memory address to write retrieved data to
// size : amount of random data to retrieve
int get_random_data(void* pos, int size) {
	int r = 0;
	while (r < size) {
#ifndef _WIN32
		int r2 = read(random_desc, pos + r, size - r);
        if (r2 != -1) r += r2;
		else if (errno != EINTR) return -1;
		//on read() return -1 with EINTR, continues
#else
		NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR)pos+r, size-r, 0x2);
		// 0x1 flag would have just been ignored on Win8+, so the "extra entropy count" most likely doesn't matter (a lot);
		// 0x2 uses system-preferred RNG; doesn't work on Vista, but nobody sane uses that
		if (status != STATUS_SUCCESS) return (int)status;
			// NTSTATUS defined as LONG, which is typedef'ed as long; however NTSTATUS codes are all 8 hex digits,
			// indicating a 32-bit domain, which is the same as int; therefore this is a safe cast
		r = strlen(pos);
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
	if (open_file != NULL) {
		fclose(open_file);
	}
#ifndef _WIN32
	close(random_desc);
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
			data_amount = x;
		}

	}
	return -1;
}

//Main 
int main(int argc, char* argv[]) {
	int r;
	if ((r = parse_args(argc, argv)) >= 0) return r;

	if ((r = prep())< 0) {
		// Can check for negative, since NTSTATUS error(and even warning) codes all start with 0xC, 0x8 respectively;
		// therefore when cast to int(as prep() does), whatever decimal number they are is negative
		switch (r) {
			case -1: fprintf(stderr, "Opening random device failed.");
				break;
			case -2: fprintf(stderr, "Memory allocation for random data failed.");
				break;
			case -3: fprintf(stderr, "Opening file to write failed.");
				break;
			default: fprintf(stderr, "Unspecified error while preparing resources occurred.");
		}
		fprintf(stderr, "Exiting.\n");
		return 6;
	}

	if ((r = get_random_data(data,data_amount)) < 0) {
		switch (r) {
			case -1: fprintf(stderr, "Reading random device failed.");
				break;
#ifdef _WIN32
			case STATUS_INVALID_PARAMETER: fprintf(stderr, "Invalid parameter for BCryptGenRandom.");
				break;
#endif // _WIN32
			default: fprintf(stderr, "Unspecified error while retrieving random data occurred.");
		}
		fprintf(stderr, "Exiting.\n");
		cleanup();
		return 7;
	}

	if (filename == NULL) {
		printf("%s\n", (char*)data);
	}else {
		fprintf(open_file, "%s", (char*)data);
	}

	cleanup();
	return 0;
}