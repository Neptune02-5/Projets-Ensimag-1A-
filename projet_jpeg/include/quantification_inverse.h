#ifndef QUANTIFICATION_INVERSE_H
#define QUANTIFICATION_INVERSE_H

#include <stdint.h>

// Applique la quantification inverse à un bloc de 64 coefficients
void quantification_inverse(int bloc[], int table_quant_bitstream[]);

#endif // QUANTIFICATION_INVERSE_H
