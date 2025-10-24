#ifndef EXTRACTION_BLOCS_H
#define EXTRACTION_BLOCS_H

#include <stdint.h>

#define TAILLE_BLOC 8
void extraction_blocs(uint8_t **image,
                      int largeur,
                      int hauteur,
                      uint8_t ***blocks,
                      int nombre_blocs_x,
                      int nombre_blocs_y,
                      int nb_composantes,
                      uint8_t H, uint8_t V,uint16_t nbre_bloc_par_MCU);
#endif