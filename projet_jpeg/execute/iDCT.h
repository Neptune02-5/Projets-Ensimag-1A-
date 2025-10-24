#ifndef IDCT_H
#define IDCT_H

#include <stdint.h>

// Applique la transformée en cosinus inverse (iDCT) sur un bloc 8x8
void iDCT(int bloc[64]);

#endif // IDCT_H
