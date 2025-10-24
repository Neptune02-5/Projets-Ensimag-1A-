#ifndef HUFFMANN_H
#define HUFFMANN_H

#include <stdint.h>
#include "flux_bits.h"

// Structure de dictionnaire Huffman
struct key_item {
    char* key;
    uint32_t valeur;
};

// Ajout d'un code dans le dictionnaire
void ajouter_element_dictionnaire(int32_t vecteur[], char code[], uint16_t indice_dict, struct key_item dictionnaire_codes[]);

// Construction de l'arbre de Huffman et dictionnaire
void arbre_huffman_DC(int vecteur[], struct key_item* dictionnaire_codes_DC, uint16_t* indice_dict_DC);
void arbre_huffman_AC(int vecteur[], struct key_item* dictionnaire_codes_AC, uint16_t* indice_dict_AC);

// Décodage Huffman depuis le flux binaire
uint32_t decode_huffman_DC(struct flux_bits* flux, uint16_t* longueur_mot, struct key_item* dictionnaire_codes_DC, uint16_t indice_dict_DC);
uint32_t decode_huffman_AC(struct flux_bits* flux, uint16_t* longueur_mot, struct key_item* dictionnaire_codes_AC, uint16_t indice_dict_AC);

// Conversion Huffman vers coefficients JPEG
void conversion_DC(int32_t* predicateur, struct flux_bits* flux, int bloc[64], struct key_item* dictionnaire_codes_DC, uint16_t indice_dict_DC);
void conversion_AC(struct flux_bits* flux, int bloc[64], struct key_item* dictionnaire_codes_AC, uint16_t indice_dict_AC);

#endif // HUFFMANN_H