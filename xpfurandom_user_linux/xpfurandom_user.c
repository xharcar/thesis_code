/*
xpfurandom_user.c- Simple console application demonstrating 
XPFUrandom userspace funcionality
Author: Matej Harcar, 422714@mail.muni.cz
v1: 14-15 May 2016
*/

#include "xpfurandom.h"

int main(int argc, char** argv) {	
	xpfurandom_settings s;
	void* data;
	void* alg;
	int st = 0;
	st = xpfurandom_parse_args(argc, argv, &s);
	if (st == HELPED) return 0;
	else if (st) return 1;
	st = xpfurandom_prep(&data, s.size, &alg, &s);
	if (st) return 1;
	st = xpfurandom_get_random_data(&data, s.size, alg, &s);
	if (st) return 1;
	st = xpfurandom_cleanup(&data, s.size, &alg, &s);
	return st;
}

