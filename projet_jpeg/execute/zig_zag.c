#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

void zig_zag_inverse(int bloc[]){
    
    // je pose dans un tableau l'ordre dans lequel je dois stocker mes éléments 
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
    //je copie les données de mon bloc car lors de la modification les valeurs seront écrasées 
    int tmp[64];
    for(int16_t i=0; i<64;i++){
        tmp[i]=bloc[i];
        };

    // je modifie sur place mon bloc en suivant l'ordre zig-zag
    for(int16_t i=0; i<64;i++){
        uint8_t pos_vecteur = zigzag_ordre[i];
        bloc[i]=tmp[pos_vecteur];
        };
}