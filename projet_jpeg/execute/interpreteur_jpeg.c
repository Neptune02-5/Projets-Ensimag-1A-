#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "interpreteur_jpeg.h"
#include "flux_bits.h"

struct ImageInfos *lire_jpeg(const char *nom_fichier) {
    struct ImageInfos *infos = malloc(sizeof(struct ImageInfos));
    infos->flux = creer_flux_bits(nom_fichier);
    if (infos->flux == NULL) {
        free(infos);
        return NULL;
    }
    // vérifier le marquer SOI 
    uint8_t octet_1 = lire_octet_flux_bits(infos->flux);
    uint8_t octet_2 = lire_octet_flux_bits(infos->flux);
    if (octet_1 != 0xff || octet_2 != 0xd8 ) {
        fprintf(stderr,"Erreur : fichier JPEG non valide (pas de marqueur de debut)\n");
        exit(1);
    }
    int fin = 1;
    while (fin) {
        uint8_t marqueur_1 = lire_octet_flux_bits(infos->flux);
        if (marqueur_1 != 0xff) 
            continue;
        uint8_t marqueur_2 = lire_octet_flux_bits(infos->flux);
        // etude de cas 
        switch (marqueur_2) {
            case 0xdb: 
                {
                    uint8_t t1 = lire_octet_flux_bits(infos->flux);
                    uint8_t t2 = lire_octet_flux_bits(infos->flux);
                    uint16_t taille_dqt = (t1 << 8) | t2;
                    uint16_t octets_lus = 2; // t1 et t2
                    while (octets_lus < taille_dqt) {
                        uint8_t precision_index = lire_octet_flux_bits(infos->flux);
                        octets_lus++;
                        uint8_t precision = precision_index >> 4; // 4 bits de poids fort du troisieme octet
                        uint8_t indice = precision_index & 0x0f; // 4 bits de poids faible du troisime octet
                        // stocker dans la table de quantif les 64 coeff selon la precision (8 ou 16 bits)
                        for (int i = 0; i < 64; i++) {
                            // etude de cas selon la precision
                            uint16_t val = (precision == 0) ? lire_octet_flux_bits(infos->flux) : (lire_octet_flux_bits(infos->flux) << 8 | lire_octet_flux_bits(infos->flux));
                            infos->tables_quantif[indice][i] = (int)val;
                            octets_lus += (precision == 0) ? 1 : 2;
                        }
                    }
                }
                break;

            case 0xc0: 
                lire_octet_flux_bits(infos->flux);// octet 1 taille segment
                lire_octet_flux_bits(infos->flux);// octet 2 taille segment
                lire_octet_flux_bits(infos->flux);// octet de la précision(1 seul en baseline)
                // extraction de l'hauteur
                uint8_t h1 = lire_octet_flux_bits(infos->flux);
                uint8_t h2 = lire_octet_flux_bits(infos->flux);
                infos->hauteur = (h1 << 8) | h2;
                // extraction de la largeur
                uint8_t l1 = lire_octet_flux_bits(infos->flux);
                uint8_t l2 = lire_octet_flux_bits(infos->flux);
                infos->largeur = (l1 << 8) | l2;
                // extraction du nb de composantes YCbCr
                infos->nb_composantes = lire_octet_flux_bits(infos->flux);

                for (int i = 0; i < infos->nb_composantes; i++) {
                    infos->composantes[i].id = lire_octet_flux_bits(infos->flux); // Y, Cb ou Cr
                    uint8_t facteur = lire_octet_flux_bits(infos->flux);
                    // facteurs d echantillonage
                    infos->composantes[i].facteur_h = facteur >> 4;// 4 premier bits
                    infos->composantes[i].facteur_v = facteur & 0x0f;// 4 derniers
                    infos->composantes[i].index_table_quantif = lire_octet_flux_bits(infos->flux);
                    

                }
                break;

            case 0xc4: {
                // lecture de la taille du segment
                uint8_t t1 = lire_octet_flux_bits(infos->flux);
                uint8_t t2 = lire_octet_flux_bits(infos->flux);
                uint16_t taille_dht = (t1 << 8) | t2;
                uint16_t octets_lus = 2;
                // lecture des tables de Huffman
                while (octets_lus < taille_dht) {
                    uint8_t classe_id = lire_octet_flux_bits(infos->flux);
                    octets_lus++;

                    uint8_t classe = classe_id >> 4;  // 0 = DC, 1 = AC
                    uint8_t id = classe_id & 0x0F; //l'identifiant de la table
                    // allocation d’une table temporaire pour stocker les infos
                    int *table = malloc(256 * sizeof(int));
                    if (!table) {
                        fprintf(stderr, "Erreur allocation table Huffman\n");
                        exit(1);
                    }

                    int nb_symboles = 0;
                    // lecture des 16 octets indiquant le nombre de symboles pour chaque longueur
                    for (int i = 0; i < 16; i++) {
                        table[i] = lire_octet_flux_bits(infos->flux);
                        nb_symboles += table[i];
                        octets_lus++;
                    }
                    // lecture des symboles associés aux longueurs précédentes
                    for (int i = 0; i < nb_symboles; i++) {
                        table[16 + i] = lire_octet_flux_bits(infos->flux);
                        octets_lus++;
                    }

                    if (classe == 0) {
                        infos->tables_huffman_dc[id] = table;
                    } else {
                        infos->tables_huffman_ac[id] = table;
                    }
    }
    break;
}




            
            case 0xda:
                // le premier octet désigne le nombre de composantes (YcbCr)
                uint8_t t1 = lire_octet_flux_bits(infos->flux);
                uint8_t t2 = lire_octet_flux_bits(infos->flux);
                uint16_t taille_sos = (t1 << 8) | t2;
                uint8_t nb_comp_scan = lire_octet_flux_bits(infos->flux);
                for (int i = 0; i < nb_comp_scan; i++) {
                    // pour chaque composante on extrait ID et le selecteur de tables
                    uint8_t id = lire_octet_flux_bits(infos->flux);
                    int tables = lire_octet_flux_bits(infos->flux);
                    for (int j = 0; j < infos->nb_composantes; j++) {//nb_composantes=nb_comp_scan en baseline
                        if (infos->composantes[j].id == id) {
                            infos->composantes[j].index_table_dc = tables >> 4; // 4 bits de poids fort
                            infos->composantes[j].index_table_ac = tables & 0x0f;// 4 bits de pods faible
                        }     
                    }
                }
                //lire et ignorer dans le mode baseline jpeg
                lire_octet_flux_bits(infos->flux); //index du premier coeff ac (0)
                lire_octet_flux_bits(infos->flux); //index du dernier coeff ac (63)
                lire_octet_flux_bits(infos->flux); //approximation (0)
                // sos et le dernier marqueur avant eoi donc fin de boucle while
                fin = 0;
                break;
                
            // le case pour ffe0 (APP0) peut etre ignoré car non utile au decodage des images
            
            case 0xd9:
                fprintf(stderr,"Erreur : fin d'image avant SOS\n");
                exit(1);

            // default lit les octets des marqueurs non reconnus
            default:
            {
                // determiner la taille du marqueur
                uint8_t t1 = lire_octet_flux_bits(infos->flux);
                uint8_t t2 = lire_octet_flux_bits(infos->flux);
                uint16_t taille_marqueur = (t1 << 8) | t2;
                // lire tous les octets du marqueur
                for (int i = 0; i < taille_marqueur - 2; i++) {
                    lire_octet_flux_bits(infos->flux);
                }
            }
            break;
        }
    }
    return infos; 
} 
// liberation des malloic
void detruire_image(struct ImageInfos *infos) {
    for (int i = 0; i < 2; i++) {
        free(infos->tables_huffman_dc[i]);
        free(infos->tables_huffman_ac[i]);
    }
    liberer_flux_bits(infos->flux);
    free(infos);
}
// retourne la largeur de l’image
uint16_t obtenir_largeur_image(const struct ImageInfos *infos) {
    return infos->largeur;
}
// retourne la hauteur de l’image
uint16_t obtenir_hauteur_image(const struct ImageInfos *infos) {
    return infos->hauteur;
}
// retourne le nombre de composantes (1 = gris, 3 = couleur)
uint16_t obtenir_nb_composantes(const struct ImageInfos *infos) {
    return infos->nb_composantes;
}
