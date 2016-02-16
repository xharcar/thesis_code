// basic_win_rng.c : Defines the entry point for the console application.
//

//Standard C includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _WIN32
//Windows-specific includes
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib,"bcrypt.lib")
#else
#include <unistd.h>
#endif // _WIN32

// Waits for sec seconds
void wait_s(int sec) {
#ifdef _WIN32
	Sleep(sec * 1000);
#else
	sleep(sec);
#endif // _WIN32
}


int main(int argc, char* argv[])
{
	int data_amount = 0;
	int data_read = 0;
	if (argv[1] == NULL) {
		puts("No argument detected, exiting. Run with -h argument for help.");
		return -1;
	}
	if (strcmp("-h", argv[1]) == 0) {
		puts("Valid arguments:\n-h: Prints this\nX: Number of output bytes desired, 0<=X<2147483647");
		return 0;
	}
	else {
		data_amount = strtol(argv[1], NULL, 10);
		if (data_amount == 0) {
			puts("Invalid argument, reading default amount of data (1KB)");
			data_amount = 1024;
		}
	}

	PUCHAR data = (PUCHAR)calloc(data_amount+1,sizeof(char));
	if (data == NULL) {
		puts("Allocation failed, exiting.");
	}
	while (data_read < data_amount) {
		NTSTATUS status = BCryptGenRandom(NULL, data+data_read, data_amount-data_read, 0x00000003);
		// argument 1 : algorithm to use, iff NULL AND (arg4 & 0x2) uses system-preferred CSPRNG (not supported on Vista)
		// arg 2 : pointer to data storage
		// arg 3 : number of bytes of data to be read
		// arg 4 : flags; 0x2 see above, 0x1 uses entropy in arg3 as extra(ignored on Win8+), 
		//			else uses just a random number for entropy
		switch (status) {
			case 0: //just continue, all good
				break;
			case STATUS_INVALID_HANDLE:
				puts("RNG algorithm handle invalid, exiting.");
				return 2;
				break;
			case STATUS_INVALID_PARAMETER:
				puts("Most likely invalid flags, exiting");
				return 2;
		}
		data_read = strlen(data);
	}
	//while-loop is necessary, test runs sometimes came up with less data than desired
	//will be fixed in urandom call
	char* data_str = (char*)data;
	printf("Data returned: %d bytes\n", strlen(data_str));
	puts(data_str); // demonstrates functionality
	//wait_s(5); // was used for debugging
    return 0;
}