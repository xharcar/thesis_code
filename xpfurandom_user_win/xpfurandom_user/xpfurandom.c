/*
xpfurandom.c - Implementations of functions defined in xpfurandom.h
Author: Matej Harcar, 422714@mail.muni.cz
v1: 14-15 May 2016
*/

#include "xpfurandom.h"

#ifdef _WIN32

int xpfurandom_parse_args(int c, char** v,xpfurandom_settings* s) {
	s->filename = NULL;
	s->file_out = NULL;
	s->size = 256;
	if (c < 2) {
		fprintf(stderr, "XPFUrandom: no arguments given. Run with -h for help.\n");
		return FAIL;
	}
	for (int i = 0; i < c; i++) {
		if (!strcmp(v[i], ARG_HELP)) {
			printf("%s", HELP_TEXT);
			return HELPED;
		}
		if (!strcmp(v[i], ARG_SIZE)) {
			if (v[i + 1] == NULL) {
				fprintf(stderr, "XPFUrandom: Buffer size not given.\n");
				return FAIL;
			}
			s->size = strtoul(v[i + 1], NULL, 10);
			if (errno == ERANGE || errno == EINVAL || s->size > INT32_MAX) {
				fprintf(stderr, "XPFUrandom: Invalid buffer size given.\n");
				return FAIL;
			}
		}
		if (!strcmp(v[i], ARG_FILE)) {
			if (v[i + 1] == NULL || v[i + 1][0] == '-') {
				fprintf(stderr, "XPFUrandom: Invalid or no filename given.\n");
				return FAIL;
			}
			for (unsigned j = 0; j < strlen(v[i]); j++) {
				if (!isgraph(v[i+1][j])) {
					fprintf(stderr, "XPFUrandom: Invalid character in filename.\n");
					return FAIL;
				}
			}
			s->filename = v[i+1];
		}
	}
	return OK;
}

int xpfurandom_prep(void** data, uint32_t size, void** win_alg_handle, xpfurandom_settings* s) {
	NTSTATUS st;
	*data = malloc(size);
	if (*data == NULL) {
		fprintf(stderr, "XPFUrandom: Random data buffer allocation failed.");
		return FAIL;
	}
	*win_alg_handle = malloc(sizeof(PVOID));
	if (*win_alg_handle == NULL) {
		fprintf(stderr, "XPFUrandom: RNG Algorithm handle space allocation failed.");
		return FAIL;
	}
	st = BCryptOpenAlgorithmProvider(win_alg_handle, BCRYPT_RNG_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
	switch (st) {
		case STATUS_SUCCESS:break;
		case STATUS_NOT_FOUND: {
			fprintf(stderr, "XPFUrandom: BCryptOpenAlgorithm failed: no RNG provider found(STATUS_NOT_FOUND)(this should never happen)\n");
			return FAIL;
		}break;
		case STATUS_INVALID_PARAMETER: {
			fprintf(stderr, "XPFUrandom: BCryptOpenAlgorithm failed: invalid parameter(STATUS_INVALID_PARAMETER)\n");
			return FAIL;
		}break;
		case STATUS_NO_MEMORY: {
			fprintf(stderr, "XPFUrandom: BCryptOpenAlgorithm failed: memory allocation failure(STATUS_NO_MEMORY)\n");
			return FAIL;
		}break;
			// if NOT_FOUND happens and this is run on Win7+, something is SERIOUSLY WRONG;
			// INVALID_PARAMETER shouldn't happen either, but maybe if wrong pointer is given it could;
	}
	if (s->filename != NULL) {
		s->file_out = fopen(s->filename, "wb");
		if (s->file_out == NULL) {
			fprintf(stderr, "XPFUrandom: fopen failed: %s\n", strerror(errno));
			return FAIL;
		}
	}
	return OK;
}

int xpfurandom_get_random_data(void** data, uint32_t size, void* win_alg_handle, xpfurandom_settings* s){
	NTSTATUS st;
	st = BCryptGenRandom(win_alg_handle, *data, size, 0);
	switch (st){
		case STATUS_SUCCESS:break;
		case STATUS_INVALID_HANDLE: {
			fprintf(stderr, "XPFUrandom: BCryptGenRandom failed: invalid algorithm handle(STATUS_INVALID_HANDLE)\n");
			return FAIL;
		}break;
		case STATUS_INVALID_PARAMETER: {
			fprintf(stderr, "XPFUrandom: BCryptGenRandom failed: invalid parameter(STATUS_INVALID_PARAMETER)\n");
			return FAIL;
		}break;
	}
	if (s->file_out) {
		st = fwrite(*data, 1, size, s->file_out);
		if (st != size) {
			fprintf(stderr, "XPFUrandom: fwrite failed\n");
			return FAIL;
		}
		st = fflush(s->file_out);
		if (st) {
			fprintf(stderr, "XPFUrandom: fflush failed: %s\n",strerror(errno));
			return FAIL;
		}
	}
	return OK;
}

int xpfurandom_cleanup(void** data, uint32_t size, void** win_alg_handle, xpfurandom_settings* s) {
	NTSTATUS st;
	char* x = *data;
	st = BCryptCloseAlgorithmProvider(*win_alg_handle, 0);
	switch (st){
		case STATUS_SUCCESS: break;
		case STATUS_INVALID_HANDLE: {
			fprintf(stderr, "XPFUrandom: BCryptCloseAlgorithmProvider failed: invalid algorithm handle(STATUS_INVALID_HANDLE)\n");
			return FAIL;
		}break;
	}
	SecureZeroMemory(*data, size);
	if (x[0]) {
		fprintf(stderr, "XPFUrandom: SecureZeroMemory failed, memory not zeroed, will not be freed\n");
		return FAIL;
	}
	if (s->file_out) {
		st = fclose(s->file_out);
		if (st) {
			fprintf(stderr, "XPFUrandom: fclose failed: %s\n", strerror(errno));
			return FAIL;
		}
	}
	free(*data);
	*data = NULL;
	return OK;
}

#else // Linux

int xpfurandom_parse_args(int c, char** v, xpfurandom_settings* s) {
	s->mode = GRND_NONBLOCK;
	s->filename = NULL;
	s->file_out = NULL;
	s->size = 256;

	for (int i = 0; i < c; i++) {
		if (!strcmp(v[i], ARG_HELP)) {
			printf("%s", HELP_TEXT);
			return HELPED;
		}
		if (!strcmp(v[i], ARG_BLOCK)) {
			s->mode ^= GRND_NONBLOCK;
		}
		if (!strcmp(v[i], ARG_RAND)) {
			s->mode ^= GRND_RANDOM;
		}
		if (!strcmp(v[i], ARG_SIZE)) {
			s->size = strtoul(v[i + 1], NULL, 10);
			if(errno == ERANGE || errno == EINVAL || s->size > INT32_MAX){
				fprintf(stderr, "XPFUrandom: Invalid size given.\n");
				return FAIL;
			}
		}
		if (!strcmp(v[i], ARG_FILE)) {
			if (v[i + 1] == NULL || v[i + 1][0] == '-') {
				fprintf(stderr, "XPFUrandom: Invalid or no filename given.\n");
				return FAIL;
			}
			for (int j = 0; j < strlen(v[i]); j++) {
				if (!isgraph(v[i+1][j])) {
					fprintf(stderr, "XPFUrandom: Invalid character in filename.\n");
					return FAIL;
				}
			}
			s->filename = v[i + 1];
		}
	}
	return OK;
}

int xpfurandom_prep(void** data, uint32_t size, void** win_alg_handle, xpfurandom_settings* s) {
	*data = malloc(size);
	if (data == NULL) {
		fprintf(stderr, "XPFUrandom: memory allocation failed\n");
		return FAIL;
	}
	if (s->filename != NULL) {
		s->file_out = fopen(s->filename, "wb");
		if (s->file_out == NULL) {
			fprintf(stderr, "XPFUrandom: fopen failed: %s\n", strerror(errno));
			return FAIL;
		}
	}
	return OK;
}

int xpfurandom_get_random_data(void** data, uint32_t size, void* win_alg_handle, xpfurandom_settings* s) {
	int32_t st;
	errno = 0;
	do {
		st = syscall(SYS_getrandom, *data, size, s->mode);
	} while (errno == EINTR);
	if (st != size) {
		if (st == -1) {
			switch (errno) {
				case EINVAL: {
					fprintf(stderr, "XPFUrandom: getrandom: %s (EINVAL)\n",strerror(errno));
				}break;
				case EFAULT: {
					fprintf(stderr, "XPFUrandom: getrandom: %s (EFAULT)\n",strerror(errno));
				}break;
				case EAGAIN: {
					fprintf(stderr, "XPFUrandom: getrandom: %s (EAGAIN)\n", strerror(errno));
				}
			}
			return FAIL;
		}
		return st;
	}
	if (s->file_out) {
		st = fwrite(data, 1, size, s->file_out);
		if (st != size) {
			fprintf(stderr, "XPFUrandom: fwrite failed\n");
			return FAIL;
		}
		st = fflush(s->file_out);
		if (st) {
			fprintf(stderr, "XPFUrandom: fflush failed: %s\n", strerror(errno));
			return FAIL;
		}
	}
	return OK;
}

int xpfurandom_cleanup(void** data, uint32_t size, void** win_alg_handle, xpfurandom_settings* s) {
	int st;
	char* x = *data;
	memset(data, 0, size);
	if (x[0]) {
		fprintf(stderr, "XPFUrandom: memory zero failed, memory not freed\n");
		return FAIL;
	}
	if (s->file_out) {
		st = fclose(s->file_out);
		if (st) {
			fprintf(stderr, "XPFUrandom: fclose failed: %s\n", strerror(errno));
			return FAIL;
		}
	}
	return OK;
}

#endif // _WIN32
