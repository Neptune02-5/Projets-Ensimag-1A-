# Décodeur JPEG – Projet Ensimag

Ce projet consiste à réaliser un décodeur d’images JPEG (Baseline, séquentiel), avec gestion du niveau de gris, de la couleur, et du sous-échantillonnage.

---
## Remarque importante

Nous avons mis tout le long du projet tous les fichiers .c .h et les images dans un seul répertoire où on compilait et générait nos images avec gcc. Lorsque qu'on a voulu utiliser le Makefile nous rencontrons des segmentation fault à l'execution de jpeg2ppm malgré que le code marche très bien dans notre répertoire. On a essayé de résoudre cela mais en vain et il était trop tard donc on a décidé d'ajouter un répertoire nommé /execute avec un fichier .txt où on a écrit les commandes pour compiler à partir de lui en soutenance. Les mêmes fcts se trouvent aussi dans src et include et images comme le veut le respect de l'architecture du projet.

---

## 1. Architecture modulaire

Le projet est découpé en modules, chacun réalisant une étape clé du décodage JPEG :

| Module                    | Entrées                         | Sorties                          | Rôle                                                       |
|--------------------------|----------------------------------|----------------------------------|------------------------------------------------------------|
| `flux_bits.c`            | Nom du fichier JPEG             | Flux binaire                     | Lecture des bits                                           |
| `interpreteur_jpeg.c`    | Flux binaire                    | `ImageInfos`                     | Analyse des marqueurs (SOI, DQT, DHT, SOF0, SOS, etc.)     |
| `huffmann_table.c`       | Tables Huffman brutes           | Dictionnaires `key_item[]`       | Construction des dictionnaires de décodage                |
| `quantification_inverse.c` | Blocs quantifiés              | Blocs déquantifiés               | Applique la table de quantification inverse               |
| `zig_zag.c`              | Blocs zig-zag                   | Blocs réordonnés                 | Réorganise les coefficients DCT                           |
| `iDCT.c`                 | Blocs fréquentiels              | Blocs spatiaux                   | Transformée inverse DCT                                   |
| `upsampling.c`           | Blocs Cb/Cr sous-échantillonnés | Blocs upsamplés                  | Gère les cas d’échantillonnage vertical et horizontal     |
| `YCbCr_to_RGB.c`         | Blocs Y, Cb, Cr                 | Blocs RGB                        | Conversion YCbCr → RGB                                    |
| `extraction_bloc.c`      | RGB par blocs                   | Image complète                   | Reconstruit l’image finale ligne par ligne                |
| `ecriture_ppm.c`         | Image RGB                       | Fichier `.ppm`                   | Écriture de l’image au format PPM binaire                 |

Le point d’entrée est dans `gestion_complete_decodage.c`.

---


## 2. Répartition des tâches

| Membre             | Tâches principales                                                                 |
|--------------------|-------------------------------------------------------------------------------------|
| Younes AIT MOHAMED | Lecture des entêtes JPEG, interprétation des marqueurs, extraction des infos, écriture PPM |
| Mamoune KATNI      | Huffman, iDCT, quantification inverse, zig-zag, conversion RGB                     |
| Yasser NAMATY      | Upsampling, iDCT rapide, extraction des blocs                                      |
| Tous               | Intégration dans le main, tests, correction des bugs                               |

Les tâches ont été réparties selon les compétences et disponibilités de chacun.

---

## 3. Déroulement

| Version | Objectif                                     | Date  |
|---------|----------------------------------------------|--------|
| V1      | Image 8×8 niveau de gris                     | J+7    |
| V2      | Image en niveaux de gris multi-blocs        | J+8    |
| V3      | Image couleur (YCbCr → RGB)                 | J+13   |
| V4      | Gestion du sous-échantillonnage 4:2:2/4:2:0 | J+14   |

---

## 4. Suivi quotidien

Chaque jour :
- Bilan rapide des avancées de la veille
- Plan d'action du jour : quels modules à implémenter, quand se retrouver à l’Ensimag et quand travailler chez soi

---

## 5. Témoignages personnels

**Younes AIT MOHAMED**  
Lors des deux premiers jours, je me suis concentré sur la lecture attentive du sujet et la compréhension des structures de données à manipuler, notamment ImageInfos et la gestion du flux binaire. Ensuite, j'ai commencé par écrire les fonctions de lecture des entêtes JPEG et l’interprétation des différents marqueurs (SOI, DQT, SOF0, DHT, SOS), ce qui m’a permis de bien cerner le fonctionnement global du format JPEG.

J’ai également travaillé sur la fonction lire_jpeg, qui est essentielle car elle centralise toutes les informations nécessaires pour les étapes suivantes du décodage. Cette partie m’a demandé beaucoup de rigueur car une petite erreur pouvait compromettre tout le reste du pipeline. Je ne l'ai fini qu'à J+5.

Après avoir obtenu des informations fiables à partir des entêtes, j’ai développé la partie écriture de l’image dans un fichier .ppm. Cette étape m’a permis de visualiser les premiers résultats concrets du projet, notamment pour les images en niveaux de gris. Faite à J+6.

En parallèle, j’ai aussi participé à l’intégration des modules des autres membres du groupe, en veillant à la cohérence des structures et des appels de fonctions, mais aussi à adapter mes modules précédents pour le cas des images couleurs en écriture et à débugger certains problèmes relatifs à un décalage de bits. Cela nous a demandé plusieurs ajustements pour aligner nos fonctions respectives. 


**Mamoune KATNI**  
Fonctions codées : Huffman, quantification inverse, zig-zag, iDCT, upsampling, et gestion complète du décodage.

Déroulement du projet :
Lors des deux premiers jours, je me suis contenté de bien lire le sujet et de comprendre les objets que je devrais manipuler par la suite. Le troisième jour, nous avons réparti les tâches au sein du groupe, et j'ai commencé par les fonctions "faciles" à implémenter, en faisant continuellement des allers-retours sur le sujet pour mieux saisir l'enchaînement des étapes du décodage.

Le reste de la semaine, je me suis concentré uniquement sur la génération des tables (en utilisant un arbre) et sur le décodage des coefficients AC et DC. Le lundi de la deuxième semaine, nous avons essayé de synchroniser nos fonctions (utiliser les mêmes objets en paramètres, etc.), tout en effectuant de petits tests pour vérifier leur bon fonctionnement.

Le mardi, nous avons généré les images en noir et blanc et avons commencé à traiter le cas des images en couleur. Et là... nous sommes restés bloqués pendant 4 jours sur un même bug, qui s’est finalement avéré provenir de la table de Huffman, mal générée. J’ai donc abandonné la structure en arbre de Huffman et tenté de générer directement les codes.

Après avoir réglé ce problème, le lundi suivant, nous avons obtenu nos premières images "zig-zag" et "thumb", et j’ai corrigé ma fonction d’upsampling pour enfin générer les images nécessitant un suréchantillonnage.

Le dernier jour a été consacré uniquement à la relecture du code et à son "nettoyage".

**Yasser NAMATY**  
Durant la première semaine, je reconnais ne pas avoir été très sérieux, mais j'ai tout de même réalisé les fonctions d'extraction de blocs, d'upsampling et d'iDCT rapide.

À cette période, chacun travaillait de son côté, ce qui a entraîné un manque de cohérence dans les variables utilisées entre les différentes fonctions. Ce n'est qu'à partir de la deuxième semaine et jusqu'à la fin que nous avons harmonisé notre travail.

Cette première semaine m'a surtout permis de bien comprendre le sujet, les différentes fonctions et leurs objectifs avant de développer mes propres fonctions.

Lors de la deuxième semaine, nous avons commencé à collaborer. Les images en niveaux de gris étaient simples à traiter, mais les images couleur nous ont posé d'importantes difficultés. Nous sommes restés bloqués pendant quatre jours sur ce problème. Après avoir résolu ce bug, nous avons dû faire face à un problème d'échantillonnage, que nous avons finalement surmonté pour achever toutes les fonctions.

Il ne nous reste plus qu'un seul problème : pour les grandes images, le programme ne fonctionne pas correctement (erreur de segmentation). Selon Valgrind, il s'agit d'une erreur de mémoire. Nous avons tenté de la résoudre en utilisant free(), mais sans succès pour l'instant.

---

