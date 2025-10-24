#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846

double C(int xi) {
    return (xi == 0) ? 1.0 / sqrt(2.0) : 1.0;
}

void iDCT(int bloc[64]) {
    double temp[64];// tableau temporaire 

    // application de la formule 
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

            // gestion du dépassement 
            if (somme < 0.0) somme = 0.0;
            if (somme > 255.0) somme = 255.0;

            temp[x * 8 + y] = round(somme);
        }
    }

    // Copier les résultats arrondis dans le bloc
    for (int i = 0; i < 64; i++) {
        bloc[i] = (int)temp[i];
    }
}
