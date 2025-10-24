#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

void conversion(int Y[], int Cb[], int Cr[], uint8_t bloc_RGB[64 * 3]) {
    for (int i = 0; i < 64; i++) {

        //application directe de la formule 

        int y = Y[i];
        int cb = Cb[i] - 128;
        int cr = Cr[i] - 128;

        
        int r = round(y + 1.402 * cr);
        int g = round(y - 0.34414 * cb - 0.71414 * cr);
        int b = round(y + 1.772 * cb);


        // gestion des débordements
        if (r < 0) r = 0; if (r > 255) r = 255;
        if (g < 0) g = 0; if (g > 255) g = 255;
        if (b < 0) b = 0; if (b > 255) b = 255;

        // je stocke mes valeurs dans mon nouveau bloc préalablement alloué  

        bloc_RGB[3 * i + 0] = (uint8_t)r; // R
        bloc_RGB[3 * i + 1] = (uint8_t)g; // G
        bloc_RGB[3 * i + 2] = (uint8_t)b; // B 
    }
}