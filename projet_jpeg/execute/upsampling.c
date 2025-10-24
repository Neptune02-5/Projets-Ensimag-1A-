#include "upsampling.h"
#include <stdint.h>

void upsampling_horizontal(int *bloc, uint16_t nbre_bloc_Y_par_MCU, int **blocs_upsamplee) {

    //4 premières lignes
    for (int y = 0; y < 32; y++) {
            int pixel = bloc[y];
            blocs_upsamplee[0][2*y] = pixel;
            blocs_upsamplee[0][2*y+1] = pixel;
    }
    //4 autres lignes 
    for (int y = 32; y < 64; y++) {
            int pixel = bloc[y];
            int pos = y - 32;
            blocs_upsamplee[1][2*pos] = pixel;
            blocs_upsamplee[1][2*pos+1] = pixel;
    }
   }

void upsampling_vertical(int *bloc, uint16_t nbre_bloc_Y_par_MCU, int **blocs_upsamplee) {

    // 4 premières lignes 
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 8; x++) {
            int pixel = bloc[y * 8 + x];
            blocs_upsamplee[0][2 * y * 8 + x] = pixel;
            blocs_upsamplee[0][(2 * y + 1) * 8 + x] = pixel;
        }
    }
    // 4 autres lignes 
    for (int y = 4; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int pixel = bloc[y * 8 + x];
            blocs_upsamplee[1][2 * (y - 4) * 8 + x] = pixel;
            blocs_upsamplee[1][(2 * (y - 4) + 1) * 8 + x] = pixel;
        }
    }
}

void upsampling_horizontal_vertical(int *bloc, uint16_t nbre_bloc_Y_par_MCU, int **blocs_upsamplee) {

    // la je m'intéresse au premier sous-bloc, 4 première ligne, 4 première colonne 
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int pixel = bloc[y * 8 + x];
            blocs_upsamplee[0][(2 * y)     * 8 + 2 * x]     = pixel;
            blocs_upsamplee[0][(2 * y)     * 8 + 2 * x + 1] = pixel;
            blocs_upsamplee[0][(2 * y + 1) * 8 + 2 * x]     = pixel;
            blocs_upsamplee[0][(2 * y + 1) * 8 + 2 * x + 1] = pixel;
        }
    }

    // je m'intéresse au sous-bloc juste à coté en haut à droite 
    for (int y = 0; y < 4; y++) {
        for (int x = 4; x < 8; x++) {
            int pixel = bloc[y * 8 + x];
            int xx = x - 4;
            blocs_upsamplee[1][(2 * y)     * 8 + 2 * xx]     = pixel;
            blocs_upsamplee[1][(2 * y)     * 8 + 2 * xx + 1] = pixel;
            blocs_upsamplee[1][(2 * y + 1) * 8 + 2 * xx]     = pixel;
            blocs_upsamplee[1][(2 * y + 1) * 8 + 2 * xx + 1] = pixel;
        }
    }


    // sous-bloc en bas à gauche 
    for (int y = 4; y < 8; y++) {
        for (int x = 0; x < 4; x++) {
            int pixel = bloc[y * 8 + x];
            int yy = y - 4;
            blocs_upsamplee[2][(2 * yy)     * 8 + 2 * x]     = pixel;
            blocs_upsamplee[2][(2 * yy)     * 8 + 2 * x + 1] = pixel;
            blocs_upsamplee[2][(2 * yy + 1) * 8 + 2 * x]     = pixel;
            blocs_upsamplee[2][(2 * yy + 1) * 8 + 2 * x + 1] = pixel;
        }
    }


    // sous-bloc en bas à droite 
    for (int y = 4; y < 8; y++) {
        for (int x = 4; x < 8; x++) {
            int pixel = bloc[y * 8 + x];
            int yy = y - 4;
            int xx = x - 4;
            blocs_upsamplee[3][(2 * yy)     * 8 + 2 * xx]     = pixel;
            blocs_upsamplee[3][(2 * yy)     * 8 + 2 * xx + 1] = pixel;
            blocs_upsamplee[3][(2 * yy + 1) * 8 + 2 * xx]     = pixel;
            blocs_upsamplee[3][(2 * yy + 1) * 8 + 2 * xx + 1] = pixel;
        }
    }
}
