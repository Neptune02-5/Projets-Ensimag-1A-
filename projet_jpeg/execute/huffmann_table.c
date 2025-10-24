#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "flux_bits.h"
#include <math.h>


// structure couple clé valeur
struct key_item{
    char* key;
    uint32_t valeur;
};


void ajouter_element_dictionnaire(int32_t vecteur[], char code[], uint16_t indice_dict, struct key_item dictionnaire_codes[]) {
    // allouer la longueur du code qu'on veut stocker
    dictionnaire_codes[indice_dict].key = malloc(strlen(code));
    // copier le code dans notre dictionnaire 
    strcpy(dictionnaire_codes[indice_dict].key, code);
    // on lui associe son symbole 
    dictionnaire_codes[indice_dict].valeur = (uint32_t)vecteur[16 + indice_dict];
}


void construire_huffman(int *vecteur, struct key_item *dictionnaire, uint16_t *indice_dict) {
    uint16_t code = 0;
    int k = 0;

    for (int longueur = 1; longueur <= 16; longueur++) {
        int nb_codes = vecteur[longueur - 1];
        for (int i = 0; i < nb_codes; i++) {
            // Convertir code en string binaire de longueur `longueur`
            char key[17];
            for (int b = longueur - 1; b >= 0; b--) {
                key[longueur - 1 - b] = ((code >> b) & 1) ? '1' : '0';
            }
            key[longueur] = '\0';

            dictionnaire[k].key = strdup(key);
            dictionnaire[k].valeur = (uint32_t)vecteur[16 + k];
            k++;
            code++;
        }
        code <<= 1; // Ajoute un bit à gauche à chaque changement de longueur
    }
    *indice_dict = k;
}



void arbre_huffman_DC(int *vecteur, struct key_item *dico, uint16_t *idx) { // construction de l'arbre de huffmann DC
    construire_huffman(vecteur, dico, idx);
}

void arbre_huffman_AC(int *vecteur, struct key_item *dico, uint16_t *idx) { // contruction de l'arbre de huffmann AC 
    construire_huffman(vecteur, dico, idx);
}


uint32_t decode_huffman_DC(struct flux_bits* flux, uint16_t* longueur_mot,struct key_item* dictionnaire_codes_DC,uint16_t indice_dict_DC) {
    char mot[17] = {'\0'};

    while (*longueur_mot < 16) {  // Protection contre les débordements
        uint32_t bit_lu=0;

        //Lecture bit par bit 

        lire_bits_flux_bits(flux, 1, &bit_lu);
        
        // Conversion du bit en caractère
        char bit_char[2] = {(bit_lu) ? '1' : '0', '\0'};
        // Concaténation 
        strcat(mot, bit_char);
        (*longueur_mot)++;
        // Recherche dans le dictionnaire
        for (uint16_t i = 0; i < indice_dict_DC; i++) {
            if (strcmp(mot, dictionnaire_codes_DC[i].key) == 0) { // verifie si le mot est égal à l'un de nos clés 
                return dictionnaire_codes_DC[i].valeur;
            }
        }
    }
    return 0;  // Aucune correspondance trouvée
}

// Cette fonction retourne une valeur Huffman décodée en AC (ex: 0x24 = 2 zéros, magnitude 4)
uint32_t decode_huffman_AC(struct flux_bits* flux, uint16_t* longueur_mot,struct key_item* dictionnaire_codes_AC,uint16_t indice_dict_AC) {
    char mot[17] = {'\0'};  // 16 bits max + null terminator
    *longueur_mot=0;

    while (*longueur_mot < 16) {
        uint32_t bit_lu = 0;
        // Lecture bit par bit

        lire_bits_flux_bits(flux, 1, &bit_lu);
        
        // Ajout du bit en fin de mot
        mot[*longueur_mot] = bit_lu ? '1' : '0';
        (*longueur_mot)++;
        mot[*longueur_mot] = '\0';  

        // Recherche dans le dictionnaire
        for (uint16_t i = 0; i < indice_dict_AC; i++) {
            if (strcmp(mot, dictionnaire_codes_AC[i].key) == 0) {// verifie si le mot est égal à l'un de nos clés 
                return dictionnaire_codes_AC[i].valeur;  
            }
        }
    }
    return 0;  
}

// Conversion DC
void conversion_DC(int32_t* predicateur,struct flux_bits* flux,int bloc[64],struct key_item* dictionnaire_codes_DC, uint16_t indice_dict_DC){

    uint16_t longueur_mot=0; // la longueur du mot peut en effet etre prise comme une variable locale dans la fct decode_huffmann_DC(AC) mais je me suis dit qu'il serait intéressant de pouvoir savoir la longueur du mot qu'on décode 
    uint32_t magnitude=decode_huffman_DC(flux,&longueur_mot, dictionnaire_codes_DC, indice_dict_DC);
    uint32_t indice_classe=0;

    // Extraire l'indice de classe après avoir su qu'on doit lire magnitude bits 

    lire_bits_flux_bits(flux,magnitude,&indice_classe);
    
    
    uint32_t seuil=(1<<(magnitude-1)); // seuil 
    int val_DC=0;
    if (magnitude==0){
        val_DC =0;
    }
    else if (indice_classe < seuil){
        val_DC = indice_classe - (1 << magnitude) + 1;
    }
    else{
        val_DC=indice_classe;
    }
    val_DC=val_DC+*predicateur;

    bloc[0]=val_DC; // on positionne la valeur de notre DC

    *predicateur=val_DC; // on met à jour notre prédicatur 
}
void conversion_AC(struct flux_bits* flux,int bloc[64],struct key_item* dictionnaire_codes_AC, uint16_t indice_dict_AC){
    // conversion AC
    uint16_t longueur_mot=0;
    uint32_t indice_bloc=1; // premier coefficient est déjà consacré à DC
    while (indice_bloc < 64){
    int16_t resultat_rle = decode_huffman_AC(flux, &longueur_mot, dictionnaire_codes_AC, indice_dict_AC);
    int val_AC = 0x0;
    uint32_t indice_classe = 0;

    if (resultat_rle==0xF0){ // symbole ZOF rencontré
        indice_bloc+=16; // les 16 prochains coefficients sont nuls donc suffit de sauter l'indice_bloc avec un pas de 16
    }
    else if (resultat_rle==0x00){ // symbole EOB rencontré 
        break;
    }
    else{

    // Extraire (4 bits poids fort) et magnitude (4 bits poids faible)
    uint32_t nbre_de_zero = (resultat_rle >> 4) & 0xF;
    uint32_t magnitude = resultat_rle & 0xF;
    indice_bloc+=nbre_de_zero;

    // Extraire l'indice de classe après avoir su qu'on doit lire magnitude bits 

    lire_bits_flux_bits(flux, magnitude, &indice_classe);

    // Reconstruction de la valeur signée

    uint32_t seuil=(1<<(magnitude-1)); // seuil 

    if (indice_classe < seuil){
        val_AC = indice_classe - (1 << magnitude) + 1;
    }
    else{
        val_AC=indice_classe;
    }
    bloc[indice_bloc]=val_AC;
    indice_bloc++;
    }
    }
}

// NB : je pouvais pour les fcts arbre_huffmann et decode_huffmann en mettre seulement une qui marche autant pour la génération des tables AC et DC

// NB_2: j'ai essayer au début de créer un arbre de huffmann, cela marchais correctement pour toutes les premières images mais il y a eu un problème lors de la génération des tables dans les images qui suivent, du coup je me suis redirigé vers une autre méthode (celle au dessus)