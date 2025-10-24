#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "flux_bits.h"
#include <math.h>

#define PI 3.14159265358979323846

struct flux_bits {

    FILE *fichier;
    uint8_t octet_courant;
    uint8_t bits_restants;
    
};

struct flux_bits *creer_flux_bits(const char *nom_fichier) {
    FILE *f = fopen(nom_fichier, "rb");
    if (!f) {
        printf("Erreur : impossible d'ouvrir le fichier JPEG %s\n", nom_fichier);
        return NULL;
    }

    struct flux_bits *flux = malloc(sizeof(struct flux_bits));
    flux->fichier = f;
    flux->bits_restants = 0;
    return flux;
}

void liberer_flux_bits(struct flux_bits *flux) {
    if (flux != NULL) {
        fclose(flux->fichier);
        free(flux);
    }
}

void sauter_flux_bits(struct flux_bits *flux) {
    flux->bits_restants = 0;
}

uint8_t lire_octet_flux_bits(struct flux_bits *flux) {
    int caractere = fgetc(flux->fichier);
    if (caractere == EOF) {
        printf("Erreur : fin de fichier\n");
        exit(1);
    }
    return (uint8_t)caractere;
}

int lire_bits_flux_bits(struct flux_bits *flux, uint8_t nb_bits, uint32_t *valeur) {
    if (nb_bits > 16) return 0;  

    *valeur = 0;  

    while (nb_bits > 0) {
        
        if (flux->bits_restants == 0) {
            int caractere = fgetc(flux->fichier); // la fct fgetc ne prend qu'un octet entier
            if (caractere == EOF) return 0;
            flux->octet_courant = (uint8_t)caractere;
            flux->bits_restants = 8;

            if (flux->octet_courant == 0xff) {
                int suivant = fgetc(flux->fichier);
                if (suivant != 0x00) {
                    fseek(flux->fichier, -1, SEEK_CUR);
                }
            }
        }

        uint8_t bits_a_lire = (nb_bits < flux->bits_restants) ? nb_bits : flux->bits_restants; // le plus petit des deux
        uint8_t decalage = flux->bits_restants - bits_a_lire; 
        uint8_t bits = (flux->octet_courant >> decalage) & ((1 << bits_a_lire) - 1);
        
        *valeur = (*valeur << bits_a_lire) | bits;
        // on met à jour les compteurs
        flux->bits_restants -= bits_a_lire;
        nb_bits -= bits_a_lire;
    }

    return 1;  
}


struct key_item{
    char* key;
    uint32_t valeur;
};


struct key_item dictionnaire_codes_DC[16];
struct key_item dictionnaire_codes_AC[162];
uint16_t indice_dict_DC=0;
uint16_t indice_dict_AC=0;

void ajouter_element_dictionnaire(int32_t vecteur[], char code[], uint16_t indice_dict,struct key_item dictionnaire_codes[]){
    dictionnaire_codes[indice_dict].key = malloc(strlen(code)+1);
    strcpy(dictionnaire_codes[indice_dict].key,code);
    dictionnaire_codes[indice_dict].valeur=(uint32_t)vecteur[16+indice_dict];
}

struct noeud {
    char* val;
    struct noeud* fils_droit;
    struct noeud* fils_gauche;
};

static struct noeud* creer_noeud(const char* base) {
    struct noeud* res = malloc(sizeof(struct noeud));
    res->val = malloc(64); 
    res->val[0] = '\0';
    if (base != NULL) {
        strcat(res->val, base);
    }
    res->fils_droit = NULL;
    res->fils_gauche = NULL;
    return res;
}

void construction_huffmann(int32_t vecteur[], uint8_t* indice, struct noeud* racine,uint16_t* indice_dict,struct key_item dictionnaire_codes[],uint8_t profondeur) {
    if (*indice >= 16) return;

    uint32_t taille = vecteur[*indice];

    if (taille == 0) {
        profondeur++;
        (*indice)++;
        char gauche[64];/*0 et 1 tiennent sur 4 bits * 16*/
        snprintf(gauche, sizeof(gauche), "%s0", racine->val);
        racine->fils_gauche = creer_noeud(gauche);

        construction_huffmann(vecteur, indice, racine->fils_gauche,indice_dict,dictionnaire_codes,profondeur);

        char droit[64];
        snprintf(droit, sizeof(droit), "%s1", racine->val);
        racine->fils_droit = creer_noeud(droit);
        *indice=profondeur;
        construction_huffmann(vecteur,indice , racine->fils_droit,indice_dict,dictionnaire_codes,profondeur);
    }
    else if (taille == 1) {
        vecteur[*indice] -= 1;
        char code[64];
        strcpy(code,racine->val);
        strcat(code,"0");
        ajouter_element_dictionnaire(vecteur,code,*indice_dict,dictionnaire_codes);
        printf("Code: %s\n", code);
        (*indice_dict)++;
        char droit[64];
        (*indice)++;
        snprintf(droit, sizeof(droit), "%s1", racine->val);
        racine->fils_droit = creer_noeud(droit);
        construction_huffmann(vecteur, indice , racine->fils_droit,indice_dict,dictionnaire_codes,profondeur);
    }
    else {
        vecteur[*indice] -= 2;
        char code1[64], code2[64];

        snprintf(code1, sizeof(code1), "%s0", racine->val);
        snprintf(code2, sizeof(code2), "%s1", racine->val); 
        ajouter_element_dictionnaire(vecteur,code1,*indice_dict,dictionnaire_codes);
        (*indice_dict)++;
        ajouter_element_dictionnaire(vecteur,code2,*indice_dict,dictionnaire_codes);
        (*indice_dict)++;

        printf("Code  : %s\n", code1);
        printf("Code  : %s\n", code2);

    };
}

void arbre_huffman_DC(int32_t vecteur[]) {
    struct noeud* racine = creer_noeud("");
    uint8_t profondeur=0;
    uint8_t indice=0;
    construction_huffmann(vecteur, &indice, racine,&indice_dict_DC,dictionnaire_codes_DC,profondeur);
}
void arbre_huffman_AC(int32_t vecteur[]) {
    struct noeud* racine = creer_noeud("");
    uint8_t profondeur=0;
    uint8_t indice=0;
    construction_huffmann(vecteur, &indice, racine,&indice_dict_AC,dictionnaire_codes_AC,profondeur);
}

uint32_t decode_huffman_DC(struct flux_bits* flux, uint16_t* longueur_mot) {
    char mot[16] = {'\0'};  // Initialisation complète avec des null bytes
    *longueur_mot = 0;

    while (*longueur_mot <= 16) {  // Protection contre les débordements
        uint32_t bit_lu=0;
        lire_bits_flux_bits(flux, 1, &bit_lu);
        
        // Conversion du bit en caractère
        char bit_char[2] = {(bit_lu) ? '1' : '0', '\0'};
        printf("bit lu %u\n",bit_lu);
        // Concaténation 
        strcat(mot, bit_char);
        (*longueur_mot)++;
        printf("%s\n",mot);
        // Recherche dans le dictionnaire
        for (uint16_t i = 0; i < indice_dict_DC; i++) {
            if (strcmp(mot, dictionnaire_codes_DC[i].key) == 0) {
                return dictionnaire_codes_DC[i].valeur;
            }
        }
    }
    return 0;  // Aucune correspondance trouvée
}

// Cette fonction retourne une valeur Huffman décodée en AC (ex: 0x24 = 2 zéros, magnitude 4)
uint32_t decode_huffman_AC(struct flux_bits* flux, uint16_t* longueur_mot) {
    char mot[17] = {'\0'};  // 16 bits max + null terminator
    *longueur_mot = 0;

    while (*longueur_mot < 16) {
        uint32_t bit_lu = 0;
        lire_bits_flux_bits(flux, 1, &bit_lu);
        
        // Ajout du bit en fin de mot
        mot[*longueur_mot] = bit_lu ? '1' : '0';
        (*longueur_mot)++;
        mot[*longueur_mot] = '\0';  // Mise à jour du null terminator

        // Recherche dans le dictionnaire
        for (uint16_t i = 0; i < indice_dict_AC; i++) {
            if (strcmp(mot, dictionnaire_codes_AC[i].key) == 0) {
                return dictionnaire_codes_AC[i].valeur;  
            }
        }
    }
    return 0;  
}

// Conversion DC
void conversion_DC(int32_t* predicateur,struct flux_bits* flux,int bloc[64]){

    uint16_t longueur_mot=0;
    uint32_t magnitude=decode_huffman_DC(flux,&longueur_mot);
    uint32_t indice_classe=0;
    lire_bits_flux_bits(flux,magnitude,&indice_classe);
    printf("%u\n",magnitude);
    printf("indice_classe%u\n",indice_classe);

    uint32_t seuil=(1<<(magnitude-1));
    int val_DC=0;
    if (indice_classe < seuil){
        val_DC = indice_classe - (1 << magnitude) + 1;
    }
    else{
        val_DC=indice_classe;
    }
    val_DC=val_DC+*predicateur;
    bloc[0]=val_DC;
    *predicateur=val_DC;
    printf("valeur de DC =%d\n",val_DC);
}
void conversion_AC(struct flux_bits* flux,int bloc[64]){
    // conversion AC
    uint16_t longueur_mot=0;
    uint32_t indice_bloc=1;
    while (indice_bloc < 64){
    int16_t resultat_rle = decode_huffman_AC(flux, &longueur_mot);
    int val_AC = 0x0;
    uint32_t indice_classe = 0;

    if (resultat_rle==0xF0){
        printf("ZRL rencontré\n");
        indice_bloc+=16;
        lire_bits_flux_bits(flux,64,&indice_classe);
    }
    else if (resultat_rle==0x00){
        printf("EOB rencontré\n");
        break;
    }
    else{

    // Extraire run-length (4 bits poids fort) et magnitude (4 bits poids faible)
    uint32_t nbre_de_zero = (resultat_rle >> 4) & 0xF;
    uint32_t magnitude = resultat_rle & 0xF;
    indice_bloc+=nbre_de_zero;

    // Lire les bits de la valeur réelle
    lire_bits_flux_bits(flux, magnitude, &indice_classe);

    // Reconstruction de la valeur signée
    uint32_t seuil=(1<<(magnitude-1));
    if (indice_classe < seuil){
        val_AC = indice_classe - (1 << magnitude) + 1;
    }
    else{
        val_AC=indice_classe;
    }
    bloc[indice_bloc]=val_AC;
    indice_bloc++;
    printf("AC: %d (zéros avant: %u, magnitude: %u)\n", val_AC, nbre_de_zero, magnitude);
    }
    printf("valeur de AC =%d\n",val_AC);
    }
}






void quantification_inverse (int bloc[], int16_t table_quant_bitstream[]){
    for (uint32_t i=0 ; i<64;i++){
            bloc[i]= bloc[i] * table_quant_bitstream[i];
        }
    }

void zig_zag_inverse(int bloc[]){
    static const uint8_t zigzag_ordre[64] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};
    int tmp[64];
    for(int16_t i=0; i<64;i++){
        tmp[i]=bloc[i];
        };

    for(int16_t i=0; i<64;i++){
        uint8_t pos_vecteur = zigzag_ordre[i];
        bloc[i]=tmp[pos_vecteur];
        };
}



double C(int xi) {
    return (xi == 0) ? 1.0 / sqrt(2.0) : 1.0;
}

void iDCT(int bloc[64]) {
    double temp[64];// tableau temporaire 

    for (uint8_t x = 0; x < 8; x++) {
        for (uint8_t y = 0; y < 8; y++) {
            double somme = 0.0;

            for (uint8_t lambda = 0; lambda < 8; lambda++) {
                for (uint8_t mu = 0; mu < 8; mu++) {
                    double cos1 = cos((2 * x + 1) * lambda * PI / 16.0);
                    double cos2 = cos((2 * y + 1) * mu * PI / 16.0);
                    somme += C(lambda) * C(mu) * cos1 * cos2 * bloc[lambda * 8 + mu];
                }
            }

            somme *= 0.25;
            somme += 128.0;

            // Clip entre 0 et 255 et arrondi correct
            if (somme < 0.0) somme = 0.0;
            if (somme > 255.0) somme = 255.0;

            temp[x * 8 + y] = round(somme);
        }
    }

    // Copier les résultats arrondis dans bloc
    for (int i = 0; i < 64; i++) {
        bloc[i] = (int)temp[i];
    }
}





// Fonction pour générer le fichier de test pour DC et AC
void write_test_file(const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("Erreur d'ouverture du fichier de test");
        exit(EXIT_FAILURE);
    }

    // Exemple de données : Un seul octet pour DC et un pour AC
    uint8_t test_data[] = {
        0x7C,  // DC (code Huffman 0, 7 bits)
           0xd1, 0xca, 0xca, 0xdc, 0x76, 
           0xda ,0x4d ,0x6a, 0x00, 0x15, 0xed, 0x41 ,0xf1,
           0x2d ,0x3a, 0xdc, 0x70, 0x8b, 0x16, 0xbe, 0x4c, 0xc9, 0xbb, 
           0xb3 ,0x4f ,0xfb ,0x35 ,0xb8 ,0x7d ,0x13 ,0xab ,0x12 ,0x9d ,0x0f ,0x0e ,0x1f ,0x4e ,0x1d ,0xe3 // AC (un symbole AC, à titre d'exemple)
    };

    fwrite(test_data, sizeof(uint8_t), sizeof(test_data) / sizeof(test_data[0]), f);
    fclose(f);
}

int main() {
    // 1. Créer un fichier contenant un seul octet pour DC et un pour AC
    write_test_file("test_ac.bin");

    //
    int16_t table_quantification[64] = {[0 ... 63] = 1}; // Tous les éléments à 1

    // 2. Initialiser BitStream pour lire le fichier de test
    char* nom_fichier="test_ac.bin";
    struct flux_bits* flux=creer_flux_bits(nom_fichier);

    // 3. Table Huffman DC : code = 0 (sur 1 bit), symbole = "7" (7 bits)
    int32_t tailles_dc[17] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,7};

    // 4. Table Huffman AC
    int32_t tailles_ac[28] = {0, 2, 1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0x17, 0x18, 0x15, 0x08, 0x19, 0x00, 0x09, 0x13,
        0x23, 0x28, 0x29, 0x37};

    // 5. Génération de la table Huffman DC
    arbre_huffman_DC(tailles_dc);

    // 6. Génération de la table Huffman AC
    arbre_huffman_AC(tailles_ac);

    // 7. Décoder le bloc
    int bloc[64] = {0};  // Initialisation du bloc (64 coefficients)
    int32_t dc_precedent = 0;  // DC précédent
    conversion_DC(&dc_precedent,flux,bloc);
    conversion_AC(flux,bloc);
    //8. Afficher les résultats du bloc décodé
    printf("Bloc decode :\n");
    for (int i = 0; i < 64; i++) {
        printf("%04hx ", (uint16_t)(int16_t)bloc[i]);  // affichage hex sur 4 digits
    if ((i + 1) % 8 == 0) printf("\n");
    }
    for (int i = 0; i < 64; i++) {
        printf("%d ", table_quantification[i]);
        if ((i + 1) % 8 == 0) printf("\n");  // pour une sortie en 8×8
    }
    quantification_inverse (bloc, table_quantification);
    printf("Bloc quantification_inverse :\n");
    for (int i = 0; i < 64; i++) {
        printf("%04hx ", (uint16_t)(int16_t)bloc[i]);  // affichage hex sur 4 digits
    if ((i + 1) % 8 == 0) printf("\n");
    }
    zig_zag_inverse(bloc);
    quantification_inverse (bloc, table_quantification);
    printf("Bloc zigzag :\n");
    for (int i = 0; i < 64; i++) {
        printf("%04hx ", (uint16_t)(int16_t)bloc[i]);  // affichage hex sur 4 digits
    if ((i + 1) % 8 == 0) printf("\n");
    }

    iDCT(bloc);
    printf("Bloc iDCT :\n");
    for (int i = 0; i < 64; i++) {
        printf("%3d ", bloc[i]);  // affichage hex sur 4 digits
    if ((i + 1) % 8 == 0) printf("\n");
    }

    return 0;
}
