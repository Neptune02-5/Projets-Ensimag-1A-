#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ecriture_ppm.h"

void ecrire_image_ppm(uint16_t largeur, uint16_t hauteur, const uint8_t ***pixels_rgb,
                      int taille_bloc, uint8_t nb_composantes, uint16_t nb_MCU,
                      uint8_t h, uint8_t v, uint16_t nbre_bloc_par_MCU) {
     // Je vérifie que le tableau de blocs n'est pas NULL                   
    if (!pixels_rgb) {
        fprintf(stderr, "Erreur : tableau de blocs NULL\n");
        return;
    }

    // J’écris l’en-tête PPM ou PGM selon le nombre de composantes
    if (nb_composantes == 1) {
        fprintf(stdout, "P5\n%u %u\n255\n", largeur, hauteur); // si image pgm
    } else if (nb_composantes == 3) {
        fprintf(stdout, "P6\n%u %u\n255\n", largeur, hauteur); // si image ppm
    } else {
        fprintf(stderr, "Erreur : nombre de composantes non supporté (%d)\n", nb_composantes);
        return;
    }

    // Je calcule le nombre de blocs 8x8 pour couvrir toute l'image
    uint16_t nb_blocs_x = (largeur + 7) / 8;
    uint16_t nb_blocs_y = (hauteur + 7) / 8;

    // Je calcule combien de MCU sont nécessaires selon les facteurs h et v
    uint16_t nb_mcu_x = (largeur + 8 * h - 1) / (8 * h);
    uint16_t nb_mcu_y = (hauteur + 8 * v - 1) / (8 * v);

    // Bloc noir utilisé si un bloc est manquant
    uint8_t bloc_vide[64 * 3] = {0};

    // Je parcours tous les blocs verticalement
    for (uint16_t bloc_y = 0; bloc_y < nb_blocs_y; bloc_y++) {
        for (uint8_t ligne_dans_bloc = 0; ligne_dans_bloc < 8; ligne_dans_bloc++) {

            uint16_t y_image = bloc_y * 8 + ligne_dans_bloc;
            if (y_image >= hauteur) continue;

            // Je parcours les blocs horizontalement
            for (uint16_t bloc_x = 0; bloc_x < nb_blocs_x; bloc_x++) {
                // Je localise le MCU auquel appartient le bloc
                uint16_t mcu_x = bloc_x / h;
                uint16_t mcu_y = bloc_y / v;
                uint16_t mcu_index = mcu_y * nb_mcu_x + mcu_x;
                // Je localise le bloc dans le MCU
                uint16_t bloc_local_x = bloc_x % h;
                uint16_t bloc_local_y = bloc_y % v;
                uint16_t bloc_index_dans_mcu = bloc_local_y * h + bloc_local_x;
                // Je récupère le bloc correspondant
                const uint8_t *bloc = pixels_rgb[mcu_index][bloc_index_dans_mcu];
                if (!bloc) {
                    fprintf(stderr, "Avertissement : bloc %u manquant, rempli de noir\n", bloc_index_dans_mcu);
                    bloc = bloc_vide;
                }

// Je parcours les pixels de la ligne courante du bloc
                for (uint8_t colonne_dans_bloc = 0; colonne_dans_bloc < 8; colonne_dans_bloc++) {
                    uint16_t x_image = bloc_x * 8 + colonne_dans_bloc;
                    if (x_image >= largeur) continue; // hors image

                    uint16_t index = ligne_dans_bloc * 8 + colonne_dans_bloc;
                    // J’écris les pixels dans le flux de sortie selon le type pgm oiu ppm
                    if (nb_composantes == 1) {
                        fputc(bloc[index], stdout);
                    } else {
                        fputc(bloc[3 * index + 0], stdout); // R
                        fputc(bloc[3 * index + 1], stdout); // G
                        fputc(bloc[3 * index + 2], stdout); // B
                    }
                }
            }
        }
    }

    fprintf(stderr, "Image PPM générée avec succès\n");
}