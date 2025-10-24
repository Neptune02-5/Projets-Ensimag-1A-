#ifndef UPSAMPLING_H
#define UPSAMPLING_H

#include <stdint.h>

void upsampling_horizontal(int *bloc, uint16_t nbre_bloc_Y_par_MCU, int **blocs_upsamplee);
void upsampling_vertical(int *bloc, uint16_t nbre_bloc_Y_par_MCU, int **blocs_upsamplee);
void upsampling_horizontal_vertical(int *bloc, uint16_t nbre_bloc_Y_par_MCU, int **blocs_upsamplee);

#endif // UPSAMPLING_H
