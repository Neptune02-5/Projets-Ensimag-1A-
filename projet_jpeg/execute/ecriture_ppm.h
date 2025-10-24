#ifndef ECRITURE_PPM_H
#define ECRITURE_PPM_H

#include <stdint.h>

void ecrire_image_ppm(uint16_t largeur, uint16_t hauteur, const uint8_t ***pixels_rgb,
                      int taille_bloc, uint8_t nb_composantes, uint16_t nb_MCU,
                      uint8_t h, uint8_t v, uint16_t nbre_bloc_par_MCU);

#endif