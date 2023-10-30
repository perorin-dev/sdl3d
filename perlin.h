#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>

const int PERLIN_YWRAPB = 4;
const int PERLIN_YWRAP = 1 << PERLIN_YWRAPB;
const int PERLIN_ZWRAPB = 8;
const int PERLIN_ZWRAP = 1 << PERLIN_ZWRAPB;
const int PERLIN_SIZE = 4095;

int perlin_octaves = 4;				// default to medium smooth
double perlin_amp_falloff = 0.5;	// 50% reduction/octave

double scaled_cosine(double i) {
	return 0.5 * (1 - cos(i * M_PI));
}

double perlin[PERLIN_SIZE];
bool virgin = true;

double noise(double x, double y, double z) {
	if (virgin) {
		for (int i = 0; i < PERLIN_SIZE; i++) {
			perlin[i] = rand();
		}
		virgin = false;
	}
	if (x < 0) x = -x;
	if (y < 0) y = -y;
	if (z < 0) z = -z;

	int xi = (int)floor(x);
	int yi = (int)floor(y);
	int zi = (int)floor(z);

	double xf = x - xi;
	double yf = y - yi;
	double zf = z - zi;
	double rxf, ryf;

	double r = 0;
	double ampl = 0.5;

	double n1, n2, n3;

	for (int o = 0; o < perlin_octaves; o++) {
		int of = xi + (yi << PERLIN_YWRAPB) + (zi << PERLIN_ZWRAPB);

		rxf = scaled_cosine(xf);
		ryf = scaled_cosine(yf);

		n1 = perlin[of & PERLIN_SIZE];
		n1 += rxf * (perlin[(of + 1) & PERLIN_SIZE] - n1);
		n2 = perlin[(of + PERLIN_YWRAP) & PERLIN_SIZE];
		n2 += rxf * (perlin[(of + PERLIN_YWRAP + 1) & PERLIN_SIZE] - n2);
		n1 += ryf * (n2 - n1);

		of += PERLIN_ZWRAP;
		n2 = perlin[of & PERLIN_SIZE];
		n2 += rxf * (perlin[(of + 1) & PERLIN_SIZE] - n2);
		n3 = perlin[(of + PERLIN_YWRAP) & PERLIN_SIZE];
		n3 += rxf * (perlin[(of + PERLIN_YWRAP + 1) & PERLIN_SIZE] - n3);
		n2 += ryf * (n3 - n2);

		n1 += scaled_cosine(zf) * (n2 - n1);

		r += n1 * ampl;
		ampl *= perlin_amp_falloff;
		xi <<= 1;
		xf *= 2;
		yi <<= 1;
		yf *= 2;
		zi <<= 1;
		zf *= 2;

		if (xf >= 1) {
			xi += 1;
			xf -= 1;
		}
		if (yf >= 1) {
			yi += 1;
			yf -= 1;
		}
		if (zf >= 1) {
			zi += 1;
			zf -= 1;
		}
	}
	return r;
}
