#ifndef CONVERSION_RGB_H
#define CONVERSION_RGB_H

#include <stdint.h>

// Convertit un tableau YCbCr vers une composante RGB (R, G ou B selon le type)
// - Y, Cb, Cr : tableaux de 64 coefficients
// - bloc     : tableau de sortie (64 valeurs RGB)
// - type     : 'R', 'G' ou 'B'
void conversion(int Y[], int Cb[], int Cr[], uint8_t bloc_RGB[64 * 3]);


#endif // CONVERSION_RGB_H
