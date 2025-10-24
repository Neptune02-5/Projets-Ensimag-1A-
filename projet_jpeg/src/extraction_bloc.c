#include "extraction_bloc.h"
#include <stdio.h>

#define TAILLE_BLOC 8

void extraction_blocs(uint8_t **image,
                      int largeur,
                      int hauteur,
                      uint8_t ***blocks,
                      int nombre_blocs_x,
                      int nombre_blocs_y,
                      int nb_composantes,
                      uint8_t H, uint8_t V,uint16_t nbre_bloc_par_MCU) {

    int blocs_par_MCU = H * V;  // en supposant 1 seul bloc par composante (format RGB déjà combiné)
    int total_MCUs = nombre_blocs_x * nombre_blocs_y;
    int blocs_total = total_MCUs * blocs_par_MCU;

    int MCU_index = 0;

    for (int y_mcu = 0; y_mcu < nombre_blocs_y; y_mcu++) {
        for (int x_mcu = 0; x_mcu < nombre_blocs_x; x_mcu++) {
            for(uint16_t i=0;i<nbre_bloc_par_MCU;i++){

            for (int v = 0; v < V; v++) {
                for (int h = 0; h < H; h++) {

                    int bloc_index = MCU_index * blocs_par_MCU + v * H + h;

                    if (bloc_index >= blocs_total) {
                        fprintf(stderr, "Erreur : bloc_index %d hors limites (max %d)\n", bloc_index, blocs_total - 1);
                        continue;
                    }

                    uint8_t *bloc = blocks[bloc_index][i];
                    if (!bloc) {
                        fprintf(stderr, "Avertissement : bloc NULL à l'indice %d\n", bloc_index);
                        continue;
                    }

                    for (int y = 0; y < TAILLE_BLOC; y++) {
                        for (int x = 0; x < TAILLE_BLOC; x++) {

                            int x_pixel = x_mcu * H * TAILLE_BLOC + h * TAILLE_BLOC + x;
                            int y_pixel = y_mcu * V * TAILLE_BLOC + v * TAILLE_BLOC + y;

                            if (x_pixel < largeur && y_pixel < hauteur) {
                                int bloc_pos = y * TAILLE_BLOC + x;
                                for (int c = 0; c < nb_composantes; c++) {
                                    image[y_pixel][nb_composantes * x_pixel + c] =
                                        bloc[nb_composantes * bloc_pos + c];
                                }
                            }
                        }
                    }
                }
            }
            }

            MCU_index++;
        }
    }
}