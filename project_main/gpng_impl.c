#include "gpng_impl.h"
#define TINYPNG_IMPLEMENTATION
#include "tinypng.h"


//char* nstd_itoa(int val, char * buf, int radix) {
//	char* p = buf;
//
//	while (val) {
//		if (radix <= 10)
//			*p++ = (val % radix) + '0';
//		else {
//			int t = val % radix;
//			if (t <= 9)
//				*p++ = t + '0';
//			else
//				*p++ = t - 10 + 'a';
//		}
//		val /= radix;
//	}
//
//	*p = '\0';
//	//reverse(buf);
//	return buf;
//}
//int fnameCmp(const void* f1, const void* f2) {
//	return strcmp((char*)f1, (char*)f2);
//}
char* find_newName(void) {
	// common
	int m, n, ret;
	// file manage
	char** flist;
	int flist_len, flist_cap;
	char* my_fname;
	unsigned int my_idx;
	DIR* dp = NULL;
	struct dirent* entry = NULL;
	struct stat detail;

	dp = opendir(STORAGE_DIR);
	if (!dp) {
		printf("cannot find the directory : \n\t%s\n", STORAGE_DIR);
		return NULL;
	}

	flist_len = 0;
	flist_cap = 8;
	flist = (char**)malloc(sizeof(char*)*flist_cap);
	my_fname = (char*)malloc(sizeof(char)*DEFAULT_LEN);
	memset(my_fname, 0, DEFAULT_LEN);
	// new_file.png
	snprintf(my_fname, DEFAULT_LEN, "%s(0).png", FILE_NAME);
	
	// get the file name list
	printf("[ENTRY] \n");
	while ((entry = readdir(dp))
		!= NULL) {
		char target[512] = {0};
		snprintf(target, 512, "%s/%s", STORAGE_DIR, entry->d_name);
		lstat(target, &detail);
		printf("\t%20s", entry->d_name);

		switch(detail.st_mode & S_IFMT){
		case S_IFDIR:
			printf("\t->DIR\n");
			break;
		case S_IFREG:
			printf("\t->FILE\n");
			*(flist + flist_len) = (char*)malloc(sizeof(char)*DEFAULT_LEN);
			memset(*(flist + flist_len), 0, sizeof(char)*DEFAULT_LEN);
			snprintf(*(flist + flist_len), DEFAULT_LEN, "%s", entry->d_name);
			flist_len++;
			if (flist_len >= flist_cap) {
				flist_cap *= 2;
				flist = (char**)realloc(flist, sizeof(char*)*flist_cap);
			}
			break;
		default:
			printf("\t->others\n");
			break;
		}
	}

	if(flist_len == 0)
		goto first_file;

	// sort
	// qsort(flist, flist_len, sizeof(flist[0]), fnameCmp);

	// Bubble Sort 
	for (n = 0; n < flist_len; n++) {
		for (m = 0; m < flist_len-(n+1); m++) {
			if(strcmp(flist[m], flist[m+1]) > 0){
				char char_tmp[DEFAULT_LEN] = {0};
				strcpy(char_tmp, flist[m+1]);
				memset(flist[m+1], 0, DEFAULT_LEN);
				strcpy(flist[m+1], flist[m]);
				memset(flist[m], 0, DEFAULT_LEN);	
				strcpy(flist[m], char_tmp);
			}
		}
	}

	printf("[FILE LIST] \n");
	for (n = 0; n < flist_len; n++)
		printf("\t%s\n", flist[n]);
	printf("\t\tFile Entry/Capacity : %4d/%4d\n", flist_len, flist_cap);
	
	// check if we have already same name
	m = 0; my_idx = 1;
	for (n = 0; n < flist_len; n++) {
		ret = strcmp(my_fname, flist[n]);
		if (!ret) {
			memset(my_fname, 0, DEFAULT_LEN);
			snprintf(my_fname, DEFAULT_LEN, "%s(%d).png", FILE_NAME, my_idx++);
			m = 1;
		}
		else
			if (m)
				break;
	}
first_file:
	for(n = 0; n < flist_len; n++)
		free(flist[n]);
	free(flist);
	closedir(dp);

	printf("allocated new name : %s \n", my_fname);
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
void drawLine(tpImage* my_surf, tXY pA, tXY pB) {
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

	dot(my_surf->pix + P(aX, aY, my_surf->w), 0xff, 0x00, 0x00, 0x00);
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
			dot(my_surf->pix + P(n, aY, my_surf->w), 0xff, 0x00, 0x00, 0x00);
		else
			dot(my_surf->pix + P(aX, n, my_surf->w), 0xff, 0x00, 0x00, 0x00);
	}

	return;
}


