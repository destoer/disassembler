#pragma once
#include <stdint.h>
typedef struct {
	char *filename; // filename of rom
	int flag; // indicates what kind of disassembly we are doing
	unsigned int org; // program origin
	int len; // program len
} Config;

extern void dieifnull(unsigned char *error,void *ptr);
uint32_t hex2int(char *hex);