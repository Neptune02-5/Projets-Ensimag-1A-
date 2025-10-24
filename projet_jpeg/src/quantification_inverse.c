#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

void quantification_inverse (int bloc[], int table_quant_bitstream[]){
    for (uint32_t i=0 ; i<64;i++){
            bloc[i]= bloc[i] * table_quant_bitstream[i];
        }
    }

