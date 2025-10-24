#ifndef INTERPRETEUR_JPEG_H
#define INTERPRETEUR_JPEG_H

#include <stdint.h>
#include "flux_bits.h"
struct Composante {
    uint8_t id;
    uint8_t facteur_h;
    uint8_t facteur_v;
    uint8_t index_table_quantif;
    uint8_t index_table_dc;
    uint8_t index_table_ac;
};


// structure contenant les infos d'une image JPEG
struct ImageInfos {
    struct flux_bits *flux;
    uint16_t largeur;
    uint16_t hauteur;
    uint8_t nb_composantes; 
    struct Composante composantes[3];
    int tables_quantif[2][64]; // baseline jpeg seulement precision 8 bits dans les coefficient de la table
    int *tables_huffman_dc[2];
    int *tables_huffman_ac[2];

};

// fonction principlae qui lit un fichier JPEG et extrait les infos utiles
struct ImageInfos *lire_jpeg(const char *nom_fichier);

// libère la mémoire et ferme le fichier
void detruire_image(struct ImageInfos *infos);

// Accèder aux informations extraites 
uint16_t obtenir_largeur_image(const struct ImageInfos *infos);
uint16_t obtenir_hauteur_image(const struct ImageInfos *infos);
uint16_t obtenir_nb_composantes(const struct ImageInfos *infos);


#endif
