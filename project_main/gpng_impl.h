#ifndef _GPNG_IMPLE_H
#define _GPNG_IMPLE_H

#include <stdio.h>
#include <stdlib.h>	// qsort, abs
#include <string.h>

#include <sys/types.h>	// opendir(), closedir(), readdir()
#include <dirent.h>		// opendir(), closedir(), readdir()
#include <sys/stat.h>	// lstat()
#include <unistd.h>		// lstat(), getpwd()

//#define TINYPNG_IMPLEMENTATION
#include "tinypng.h"

#define STORAGE_DIR "./storage"
#define FILE_NAME "new_file"
#define DEFAULT_LEN 40
#define PNG_DH 640
#define PNG_DW 640

#define P(x,y,w) ((x)+(y)*(w))
#define SWAP(a, b, type) do {  \
		type temp;	\
		temp = a;	\
		a = b;		\
		b = temp;	\
	} while(0)

typedef struct {
	unsigned int pX;
	unsigned int pY;
}tXY;

//char* nstd_iota(int, char*, int);
//int fnameCmp(const void*, const void*);
char* find_newName(void);

void dot(tpPixel*, uint8_t, uint8_t, uint8_t, uint8_t);
void drawLine(tpImage*, tXY, tXY);
//void append_newChar(char**, char*);

#endif



