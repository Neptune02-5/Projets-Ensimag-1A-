#ifndef FLUX_BITS_H
#define FLUX_BITS_H

#include <stdio.h>
#include <stdint.h>

// definition d'une structure flux bits
struct flux_bits {
    FILE *fichier;
    uint8_t octet_courant;
    uint8_t bits_restants;
};

// creer un flux de bits à partie d'un fichier jpeg
extern struct flux_bits *creer_flux_bits(const char *nom_fichier);

// libérer la mémoire
extern void liberer_flux_bits(struct flux_bits *flux);

// se réaligner sur un ocett
extern void sauter_flux_bits(struct flux_bits *flux);

// lire un octet 
extern uint8_t lire_octet_flux_bits(struct flux_bits *flux);

// lire plusieurs bits et stocke resultat dans valeur
extern int lire_bits_flux_bits(struct flux_bits *flux, uint8_t nb_bits, uint32_t *valeur);

#endif
