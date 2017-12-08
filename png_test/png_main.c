#include <stdio.h>
#include <stdlib.h>	// qsort, abs
#include <string.h>

#include <sys/types.h>	// opendir(), closedir(), readdir()
#include <dirent.h>		// opendir(), closedir(), readdir()
#include <sys/stat.h>	// lstat()
#include <unistd.h>		// lstat()

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
}pXY;

char* nstd_iota(int, char*, int);
int fnameCmp(const void*, const void*);
char* find_newName(void);

void dot(tpPixel*, uint8_t, uint8_t, uint8_t, uint8_t);
void drawLine(tpImage* my_surf, pXY pA, pXY pB);
//void append_newChar(char**, char*);

char* nstd_itoa(int val, char * buf, int radix) {
	char* p = buf;

	while (val) {
		if (radix <= 10)
			*p++ = (val % radix) + '0';
		else {
			int t = val % radix;
			if (t <= 9)
				*p++ = t + '0';
			else
				*p++ = t - 10 + 'a';
		}
		val /= radix;
	}

	*p = '\0';
	//reverse(buf);
	return buf;
}
int fnameCmp(const void* f1, const void* f2) {
	int ret = strcmp((const char*)f1, (const char*)f2);
	return ret;
}
char* find_newName(void) {
	// common
	int m, n, ret;
	// file manage
	char** flist;
	int flist_len, flist_cap;
	char* my_fname;
	char char_buf[10];
	unsigned int my_idx;
	DIR* dp = NULL;
	struct dirent* entry = NULL;
	struct stat detail;

	dp = opendir(STORAGE_DIR);
	if (!dp) {
		printf("cannot find the directory : \n\t%s\n", STORAGE_DIR);
		return -1;
	}

	my_idx = 1;
	flist_len = 0;
	flist_cap = 8;
	flist = (char**)malloc(sizeof(char*)*flist_cap);
	my_fname = (char*)malloc(sizeof(char)*DEFAULT_LEN);
	memset(my_fname, 0, DEFAULT_LEN);
	memset(char_buf, 0, 10);
	// new_file.png
	strcat(my_fname, FILE_NAME);
	strcat(my_fname, ".png");

	// get the file name list
	while ((entry = readdir(dp))
		!= NULL) {
		lstat(entry->d_name, &detail);
		if (!S_ISDIR(detail.st_mode)) {
			*(flist + flist_len) = (char*)malloc(sizeof(char)*DEFAULT_LEN);
			strcpy(*(flist + flist_len), entry->d_name));
			flist_len++;
			if (flist_len >= flist_cap) {
				flist_cap *= 2;
				flist = (char**)realloc(flist, sizeof(char*)*flist_cap);
			}
		}
	}

	// sort
	qsort((char*)flist, flist_len, sizeof(char*), fnameCmp);

	// check if we have already same name
	m = 0;
	for (n = 0; n < flist_len; n++) {
		ret = strcmp(my_fname, flist[flist_len]);
		if (!ret) {
			m = 1;
			memset(my_fname, 0, DEFAULT_LEN);
			memset(char_buf, 0, 10);
			// itoa
			nstd_itoa(my_idx++, char_buf, 10);
			// new_file(x).png
			strcat(my_fname, FILE_NAME);
			strcat(my_fname, "(");
			strcat(my_fname, char_buf);
			strcat(my_fname, ")");
			strcat(my_fname, ".png");
		}
		else
			if (m)
				break;
	}

	closedir(dp);
	return my_fname;
}
void dot(tpPixel* here, uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
	here->a = a;
	here->r = r;
	here->g = g;
	here->b = b;
}
// Bresenham Algorithm
// ref: http://forum.falinux.com/zbxe/?mid=graphic&page=3&document_srl=406146&listStyle=&cpage
//
// pN = 2*dy*xN-2*dx*yN+c
// c = 2*dy+dx*(2b-1)
// p0 = 2*dy - dx
// if pN > 0
//	pM = pN + 2*dy-2*dx
// if pN < 0
//	pM = pN + 2*dy
//
// how to draw line?
// 1. find out p0 (the first pN)
// 2. check out whether pN is positive or negative
//	if pN > 0
//	+1 for pY, and pM = pN + 2*(dy-dx)
//	if pN < 0
//	pM = pN + 2*dy
void drawLine(tpImage* my_surf, pXY pA, pXY pB) {
	int n, nBegin, nEnd;
	int dx, dy;
	int pN;
	int aX, aY, bX, bY;
	// cPos = 2*(dy-dx)
	// cNeg = 2*dy
	// cInc refers dy/dx is (+) or (-), y inc or dec corresponding to x
	int cPos, cNeg, cInc;
	// dy/dx is smaller than 1 or not
	int isSteep;

	aX = pA.pX;
	aY = pA.pY;
	bX = pB.pX;
	bY = pB.pY;

	// valid check
	if (aX < 0 || 
		aX >= my_surf->w ||
		bX < 0 || 
		bX >= my_surf->w ||
		aY < 0 ||
		aY >= my_surf->h ||
		bY < 0 ||
		bY >= my_surf-> h) {
		printf("out of Region : A(%d,%d), B(%d,%d)\n",
			aX, aY, bX, bY);
		return;
	}
	
	dx = abs(aX - bX);
	dy = abs(aY - bY);

	// check dy/dx is smaller than 1 or not
	isSteep = (dy <= dx);

	if (isSteep) {
		cPos = 2 * (dy - dx);
		cNeg = 2 * dy;
		// drawing starts from left side, define pA is left dot
		// if pA is in right side, switch pB
		if (bX < aX) {
			SWAP(aX, bX, int);
			SWAP(aY, bY, int);
		}

		if (aY < bY)
			cInc = 1;
		else
			cInc = -1;

		pN = 2 * dy - dx;
		nBegin = aX;
		nEnd = bX;
	}
	else {
		cPos = 2 * (dx - dy);
		cNeg = 2 * dx;
		// drawing starts from down side, define pA is down dot
		// if pA is in down side, switch pB
		if (bY < aY) {
			SWAP(aX, bX, int);
			SWAP(aY, bY, int);
		}

		if (aX < bX)
			cInc = 1;
		else
			cInc = -1;

		pN = 2 * dx - dy;
		nBegin = aY;
		nEnd = bY;
	}

	dot(my_surf->pix + P(aX, aY, my_surf->w), 0x00, 0xff, 0xff, 0xff);
	for (n = nBegin; n < nEnd; n++) {
		if (pN < 0)
			pN += cNeg;
		else {
			pN += cPos;
			if (isSteep)
				aY += cInc;
			else
				aX += cInc;
		}

		if (isSteep)
			dot(my_surf->pix + P(n, aY, my_surf->w), 0x00, 0xff, 0xff, 0xff);
		else
			dot(my_surf->pix + P(aX, n, my_surf->w), 0x00, 0xff, 0xff, 0xff);
	}

	return;
}

int main(int agrc, char** argv) {
	// common
	int m, n, ret;
	// png manage
	char* my_fname;
	tpImage my_surf;
	pXY pA = { 120, 120 };
	pXY pB = { 360, 360 };

	my_surf.h = PNG_DH;
	my_surf.w = PNG_DW;
	my_surf.pix = (tpPixel*)malloc(sizeof(tpPixel)*PNG_DH*PNG_DW);
	memset(my_surf.pix, 0, sizeof(tpPixel)*PNG_DH*PNG_DW);

	drawLine(my_surf, pA, pB);

	my_fname = find_newName();
	ret = tpSavePNG(my_fname, &my_surf);

	free(my_surf.pix);
	free(my_fname);

	if (ret < 0) {
		printf("png creation fail... \n");
		exit(1):
	}

	return 0;
}