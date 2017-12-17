#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TINYPNG_IMPLEMENTATION
#include "tinypng.h"

#define PIX(x,y,w) ((x)+(w)*(y))

int main() {
	tpImage sample;
	tpImage rawRGB;
	int n, m, np, mp;

	sample = tpLoadPNG("image/sample.png");
	rawRGB.h = sample.h / 2;
	rawRGB.w = sample.w / 2;
	rawRGB.pix = (tpPixel*)malloc(sizeof(tpPixel)*rawRGB.h*rawRGB.w);

	np = 0; mp = 0;
	for (n = 0; n < sample.h; n += 2) {
		for (m = 0; m < sample.w; m += 2) {
			memcpy(rawRGB.pix + PIX(mp, np, rawRGB.w), sample.pix + PIX(m, n, sample.w), sizeof(tpPixel));
			mp++;
		}
		np++;
	}

	tpSavePNG("image/res.png", &rawRGB);
	
	/*
	struct tpPixel
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	struct tpImage
	{
		int w;
		int h;
		tpPixel* pix;
	};
	*/

	free(sample.pix);
	free(rawRGB.pix);
	memset(&sample, 0, sizeof(sample));
	memset(&rawRGB, 0, sizeof(rawRGB));

	return 0;
}
