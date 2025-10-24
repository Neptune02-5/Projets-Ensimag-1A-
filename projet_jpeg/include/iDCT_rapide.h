#ifndef IDCT_RAPIDE_H
#define IDCT_RAPIDE_H

#include <stdint.h>
#include <math.h>


extern const double sqrt2;

double C(int xi);

static void rot(double *x0, double *x1, double c, double s);

void iDCT_1d(double *block);

void iDCT_rapide(int bloc[64]);

#endif