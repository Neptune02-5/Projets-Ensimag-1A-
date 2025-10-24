#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "flux_bits.h"
#include "interpreteur_jpeg.h"
#include "huffmann.h"
#include "extraction_bloc.h"
#include "quantification_inverse.h"
#include "zig_zag.h"
#include "iDCT.h"
//#include "iDCT_rapide.h"
#include "upsampling.h"
#include "YCbCr_to_RGB.h"
#include "ecriture_ppm.h"


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage : %s fichier.jpeg\n", argv[0]);
        return 1;
    }

    // Stockage des données de l'entete 

    struct ImageInfos* infos = lire_jpeg(argv[1]);
    uint16_t largeur = obtenir_largeur_image(infos);
    uint16_t hauteur = obtenir_hauteur_image(infos);
    uint8_t nb_composantes = obtenir_nb_composantes(infos);

    uint8_t hY=infos->composantes[0].facteur_h;
    uint8_t vY=infos->composantes[0].facteur_v;
    
    uint8_t hCb=infos->composantes[1].facteur_h;
    uint8_t vCb=infos->composantes[1].facteur_v;


    uint8_t hCr=infos->composantes[2].facteur_h;
    uint8_t vCr=infos->composantes[2].facteur_v;

    // Calcul du nombre de MCU , nombre de bloc (Y,Cb,Cr) par MCU
    uint16_t nb_MCU_x = (largeur + 8*hY-1) / (8*hY);
    uint16_t nb_MCU_y = (hauteur + 8*vY-1) / (8*vY);
    uint16_t nb_blocs_x=(largeur+7)/8;
    uint16_t nb_blocs_y=(hauteur+7)/8;
    uint16_t nbre_MCU = nb_MCU_x * nb_MCU_y;
    uint16_t nbre_bloc_Y_par_MCU = hY*vY; 
    uint16_t nbre_bloc_Cb_par_MCU = hCb*vCb;
    uint16_t nbre_bloc_Cr_par_MCU = hCr*vCr;


    // tab_MCU est une table stockant les MCU 
    int ***tab_MCU = malloc(nbre_MCU * sizeof(int **));
    for (int mcu = 0; mcu < nbre_MCU; mcu++) {
        tab_MCU[mcu] = malloc((3*nbre_bloc_Y_par_MCU) * sizeof(int *));
        for (int i = 0; i < 3*nbre_bloc_Y_par_MCU; i++) {
            tab_MCU[mcu][i] = NULL;  
        }
    }

    // tab_RGB est une table stockant les MCU après conversion RGB 
    uint8_t ***tab_RGB = malloc(nbre_MCU * sizeof(uint8_t **));
    for (int mcu = 0; mcu < nbre_MCU; mcu++) {
        tab_RGB[mcu] = malloc(nbre_bloc_Y_par_MCU * sizeof(uint8_t *));
        for (int i = 0; i < nbre_bloc_Y_par_MCU; i++) {
            tab_RGB[mcu][i] = NULL;  
        }
    }

    // initialisation des pédicateurs 
    int32_t dc_Y = 0, dc_Cb = 0, dc_Cr = 0;

    // création de la table de huffmann de Y 
    struct key_item *table_DC_Y = malloc(256 * sizeof(struct key_item));
    struct key_item *table_AC_Y = malloc(256 * sizeof(struct key_item));
    uint16_t indice_dict_DC_Y = 0, indice_dict_AC_Y = 0;

    arbre_huffman_DC(infos->tables_huffman_dc[0], table_DC_Y, &indice_dict_DC_Y);
    arbre_huffman_AC(infos->tables_huffman_ac[0], table_AC_Y, &indice_dict_AC_Y);

    struct key_item *table_DC_C = NULL, *table_AC_C = NULL;
    uint16_t indice_dict_DC_C = 0, indice_dict_AC_C = 0;
    struct flux_bits* flux=infos->flux;

    // création des tables de huffmann communes entre Cb Cr
    if (nb_composantes == 3) { 
        table_DC_C = malloc(256 * sizeof(struct key_item));
        table_AC_C = malloc(256 * sizeof(struct key_item));
        arbre_huffman_DC(infos->tables_huffman_dc[1], table_DC_C, &indice_dict_DC_C);
        arbre_huffman_AC(infos->tables_huffman_ac[1], table_AC_C, &indice_dict_AC_C);
    }

    // parcours mcu par mcu d'après le bitstream 
   for (uint16_t mcu_y = 0; mcu_y < nb_MCU_y; mcu_y++) {
    for (uint16_t mcu_x = 0; mcu_x < nb_MCU_x; mcu_x++) {
        int MCU = mcu_y * nb_MCU_x + mcu_x;// indice de notre mcu actuelle 

        for (uint16_t i=0;i<nbre_bloc_Y_par_MCU;i++){ // itération sur les blocs Y 
            int bloc_Y[64] = {0};
            conversion_DC(&dc_Y, infos->flux, bloc_Y, table_DC_Y, indice_dict_DC_Y);
            conversion_AC(infos->flux, bloc_Y, table_AC_Y, indice_dict_AC_Y);
            quantification_inverse(bloc_Y, infos->tables_quantif[0]);
            zig_zag_inverse(bloc_Y);
            iDCT(bloc_Y);

            if (nb_composantes == 1) {
                uint8_t *pixels = malloc(64);
                for (int i = 0; i < 64; i++) {
                    int val = bloc_Y[i];
                    if (val < 0) val = 0;
                    if (val > 255) val = 255;
                    pixels[i] = (uint8_t)val;
                }
                tab_RGB[MCU][i] = pixels;
            } 
            else if ( nb_composantes == 3){
                // écrasement de pointeur pour chaque itération ce qui nous fait perdre notre pointeur donc il faut faire une copie 
                int *copie_bloc_Y = malloc(64 * sizeof(int));
                memcpy(copie_bloc_Y, bloc_Y, 64 * sizeof(int));
                tab_MCU[MCU][i] = copie_bloc_Y;
            }
        }
        if (nb_composantes==3){
            uint16_t n= nbre_bloc_Y_par_MCU/ nbre_bloc_Cb_par_MCU ; // le nombre de blocs qu'on va générer à partir de notre bloc_(Cb,Cr) après sur-échantillonage 
            for (uint16_t i=0;i<nbre_bloc_Cb_par_MCU;i++){ 
                
                int bloc_Cb[64]={0};
                conversion_DC(&dc_Cb, infos->flux, bloc_Cb, table_DC_C, indice_dict_DC_C);
                conversion_AC(infos->flux, bloc_Cb, table_AC_C, indice_dict_AC_C);
                quantification_inverse(bloc_Cb, infos->tables_quantif[1]);
                zig_zag_inverse(bloc_Cb);
                iDCT(bloc_Cb);

                int *copie_bloc_Cb = malloc(64 * sizeof(int));
                memcpy(copie_bloc_Cb, bloc_Cb, 64 * sizeof(int));

                if (hY*vY!=1){ // cas où on a sur-échantillonage 

                int **blocs_upsamp_Cb = malloc(n * sizeof(int *));
                for (int i = 0; i < n; i++) {
                    blocs_upsamp_Cb[i] = malloc(64 * sizeof(int));  
                }
                if (hY==2 & vY==2){// cas échantillonage horizontal vertical 
                upsampling_horizontal_vertical(bloc_Cb,  nbre_bloc_Y_par_MCU ,blocs_upsamp_Cb);
                }
                else if (hY==1 & vY==2){// cas échantillonage vertical 
                upsampling_vertical(bloc_Cb,  nbre_bloc_Y_par_MCU ,blocs_upsamp_Cb);
                }
                else if (hY==2 & vY==1){// cas échantillonage horizontal 
                upsampling_horizontal(bloc_Cb,  nbre_bloc_Y_par_MCU ,blocs_upsamp_Cb);
                }

                for (int j = 0; j < n; j++) {
                tab_MCU[MCU][nbre_bloc_Y_par_MCU+i*2 + j] = blocs_upsamp_Cb[j];
                }

                }
                else{ // cas pas de sur-échantillonage 
                for (int j = 0; j < n; j++) {
                tab_MCU[MCU][nbre_bloc_Y_par_MCU+i*2+j] = copie_bloc_Cb;
                    }
             }
        }    
            for (uint16_t i=0;i<nbre_bloc_Cr_par_MCU;i++){
                int bloc_Cr[64]={0};

                conversion_DC(&dc_Cr, infos->flux, bloc_Cr, table_DC_C, indice_dict_DC_C);
                conversion_AC(infos->flux, bloc_Cr, table_AC_C, indice_dict_AC_C);
                quantification_inverse(bloc_Cr, infos->tables_quantif[1]);
                zig_zag_inverse(bloc_Cr);
                iDCT(bloc_Cr);

                int *copie_bloc_Cr = malloc(64 * sizeof(int));
                memcpy(copie_bloc_Cr, bloc_Cr, 64 * sizeof(int));

                if (hY*vY!=1){
                int **blocs_upsamp_Cr = malloc(n * sizeof(int *));
                for (int i = 0; i < n; i++) {
                    blocs_upsamp_Cr[i] = malloc(64 * sizeof(int));  
                }
                if (hY==2 & vY==2){ // cas échantillonage horizontal vertical 
                upsampling_horizontal_vertical(bloc_Cr,  nbre_bloc_Y_par_MCU ,blocs_upsamp_Cr);
                }
                else if (hY==1 & vY==2){ // cas échantillonage vertical 
                upsampling_vertical(bloc_Cr,  nbre_bloc_Y_par_MCU ,blocs_upsamp_Cr);
                }
                else if (hY==2 & vY==1){ // cas échantillonage horizontal 
                upsampling_horizontal(bloc_Cr,  nbre_bloc_Y_par_MCU ,blocs_upsamp_Cr);
                }

                for (int j = 0; j < n; j++) {
                tab_MCU[MCU][2*nbre_bloc_Y_par_MCU+2*i + j] = blocs_upsamp_Cr[j];
                
                }
                }
                else{ // cas pas de sur-échantillonage 
                for (int j = 0; j < n; j++) {
                tab_MCU[MCU][2*nbre_bloc_Y_par_MCU+2*i+j] = copie_bloc_Cr;
                    }
             }
                
        }
    
        // conversion RGB pour notre MCU actuelle 
        for (int i=0;i<nbre_bloc_Y_par_MCU;i++){
        uint8_t *pixels_RGB = malloc(64 * 3);
        conversion(tab_MCU[MCU][i], tab_MCU[MCU][nbre_bloc_Y_par_MCU+i], tab_MCU[MCU][2*nbre_bloc_Y_par_MCU+i], pixels_RGB); 
        tab_RGB[MCU][i] = pixels_RGB;
            }

        }
    }
}
    // ecriture des données dans le fichier ppm 
    ecrire_image_ppm(largeur, hauteur, (const uint8_t ***)tab_RGB, 64, nb_composantes,nbre_MCU,hY,vY,nbre_bloc_Y_par_MCU);
    return 0;
}