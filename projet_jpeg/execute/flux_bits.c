#include <stdio.h>
#include <stdlib.h>

#include "flux_bits.h"


// Je crée et initialise une structure flux_bits à partir d’un fichier JPEG donné
struct flux_bits *creer_flux_bits(const char *nom_fichier) {
    FILE *f = fopen(nom_fichier, "rb");
    if (f == NULL) {
        printf("Erreur : impossible d'ouvrir le fichier JPEG %s\n", nom_fichier);
        return NULL;
    }
// J’alloue une structure flux_bits
    struct flux_bits *flux = malloc(sizeof(struct flux_bits));
    flux->fichier = f;
    flux->bits_restants = 0; // Aucun bit n’est encore en attente de lecture
    return flux;
}

void liberer_flux_bits(struct flux_bits *flux) {
    if (flux != NULL) {
        fclose(flux->fichier);
        free(flux);
    }
}
// Je réinitialise le compteur de bits restants
void sauter_flux_bits(struct flux_bits *flux) {
    flux->bits_restants = 0;
}
// Je lis un octet du fichier
uint8_t lire_octet_flux_bits(struct flux_bits *flux) {
    int caractere = fgetc(flux->fichier);
    if (caractere == EOF) {
        printf("Erreur : fin de fichier\n");
        exit(1);
    }
    return (uint8_t)caractere;
}
// Je lis jusqu’à 16 bits consécutifs au choix à partir du flux 
int lire_bits_flux_bits(struct flux_bits *flux, uint8_t nb_bits, uint32_t *valeur) {
    if (nb_bits > 16) return 0;

    *valeur = 0;
    // Je boucle tant qu’il reste des bits à lire
    while (nb_bits > 0) {
        if (flux->bits_restants == 0) {
            int caractere = fgetc(flux->fichier);
            if (caractere == EOF) return 0;
            flux->octet_courant = (uint8_t)caractere;
            flux->bits_restants = 8;
            //  cas particulier des octets 0xFF
            if (flux->octet_courant == 0xFF) {
                int suivant = fgetc(flux->fichier);
                if (suivant == 0x00) {
                    // octet de bourrage à ignorer
                } else {
                    // marqueur JPEG, je le remets dans le flux
                    fseek(flux->fichier, -1, SEEK_CUR);
                }
            }
        }
        // Je détermine combien de bits je peux lire depuis l’octet courant
        uint8_t bits_a_lire = (nb_bits < flux->bits_restants) ? nb_bits : flux->bits_restants;
        uint8_t decalage = flux->bits_restants - bits_a_lire;
        uint8_t bits = (flux->octet_courant >> decalage) & ((1 << bits_a_lire) - 1);
        // J’extrais les bits à lir et les ajoute à la valeur accumulee
        *valeur = (*valeur << bits_a_lire) | bits;
        flux->bits_restants -= bits_a_lire;
        nb_bits -= bits_a_lire;
    }

    return 1;
}