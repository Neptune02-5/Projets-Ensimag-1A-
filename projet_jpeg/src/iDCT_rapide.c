#include <stdint.h>
#include <math.h>
#include <stdio.h>

#define PI 3.14159265358979323846

static const double sqrt2 = 1.4142135623730951;

// C(x)
double C(int x) {
    return (x == 0) ? 1.0 / sqrt2 : 1.0;
}
void iDCT_1d(double *v) {
    double tmp[8];
    double s0 = v[0], s1 = v[1], s2 = v[2], s3 = v[3];
    double s4 = v[4], s5 = v[5], s6 = v[6], s7 = v[7];
    double a0 = s0 + s4;
    double a1 = s0 - s4;
    double a2 = s2 * C(2) - s6 * C(6);
    double a3 = s2 * C(6) + s6 * C(2);
    double b0 = a0 + a3;
    double b1 = a1 + a2;
    double b2 = a1 - a2;
    double b3 = a0 - a3;
    double c0 = s1 * C(1) - s7 * C(7);
    double c1 = s1 * C(7) + s7 * C(1);
    double c2 = s5 * C(5) - s3 * C(3);
    double c3 = s5 * C(3) + s3 * C(5);
    double d0 = c0 + c2;
    double d1 = c1 + c3;
    double d2 = c1 - c3;
    double d3 = c0 - c2;
    tmp[0] = b0 + d1;
    tmp[1] = b1 + d0;
    tmp[2] = b2 + d3;
    tmp[3] = b3 + d2;
    tmp[4] = b3 - d2;
    tmp[5] = b2 - d3;
    tmp[6] = b1 - d0;
    tmp[7] = b0 - d1;
    for (int i = 0; i < 8; i++) {
        v[i] = tmp[i];
    }
}
void iDCT_rapide(int bloc[64]) {
    double temp[64];
    double row[8], col[8];
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            row[x] = bloc[y * 8 + x] * C(x);
        }
        iDCT_1d(row);
        for (int x = 0; x < 8; x++) {
            temp[y * 8 + x] = row[x];
        }
    }

    // IDCT colonnes 
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            col[y] = temp[y * 8 + x] * C(y);
        }
        iDCT_1d(col);
        for (int y = 0; y < 8; y++) {
            double val = col[y] / 4.0 + 128.0; 
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            bloc[y * 8 + x] = (int)round(val);
        }
    }
}
/*NB:
j'ai au debut utilise l algo de loeffler tel qu'il est avec 11 mult mais il y'avait un tres grand decalage 
Alors: j'ai utilise son principe avec 16 mult et ca a ameliore le resultat*/