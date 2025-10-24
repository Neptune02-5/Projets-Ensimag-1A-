#!/usr/bin/env python3
"""
compute tailles of all connected components.
sort and display.
"""

from timeit import timeit
from sys import argv

from geo.point import Point

import time


def load_instance(filename):
    """
    loads .pts file.
    returns distance limit and points.
    """
    with open(filename, "r") as instance_file:
        lines = iter(instance_file)
        distance = float(next(lines))
        points = [Point([float(f) for f in l.split(",")]) for l in lines]

    return distance, points





def print_components_tailles(distance, points):
    visited = {point:False for point in points}
    tailles = []
    def construire_kd_arbre(points,profondeur=0):
        if not points : return []
        axe = profondeur %2 # soit on divise selon l axe des abscisses ou celui des ordoonnées
        points_1=sorted(points,key =lambda p:p.coordinates[axe]) #on trie selon l'axe des abscisses ou des ordonnées pour choisir ou couper le plan
        mediane=len(points_1)//2
        noeud=points_1[mediane]
        arbre_gauche=construire_kd_arbre(points_1[:mediane],profondeur+1)
        arbre_droit=construire_kd_arbre(points_1[mediane+1::],profondeur+1)
        return [noeud,[arbre_gauche,arbre_droit]]

    kd_arbre=construire_kd_arbre(points)
    def parcours_profondeur(noeud,liste):
        pile=[noeud]
        visited[noeud]=True

        while pile:
            p1 = pile.pop()
            """ Trouver les voisins en s'aidant du KD-arbre """
            def trouver_voisins(kd_arbre, point, distance,liste, profondeur=0):
                if not kd_arbre or len(kd_arbre) < 1:# Notre arbre n'est pas vide ou mal struturé
                    return []
                noeud = kd_arbre[0]
                axe = profondeur % 2
                # Vérifier le noeud courant
                if not visited[noeud] and point.distance_to(noeud) <= distance:
                    visited[noeud]=True
                    liste.append(noeud)
                    pile.append(noeud)
                # choisir partie gauche ou droite de l hyperplan (partie au dessus ou au dessous selon axe)(recherche fils droit ou gauche ou les deux)
                if point.coordinates[axe] - distance <= noeud.coordinates[axe]:# cette comparaison nous indique qu'il y a possibilité de trouver des points voisins dans cette partie du plan
                    trouver_voisins( kd_arbre[1][0],point, distance,liste, profondeur+1)
                
                if point.coordinates[axe] + distance >= noeud.coordinates[axe]:
                    trouver_voisins(kd_arbre[1][1],point, distance,liste, profondeur+1)
            trouver_voisins(kd_arbre, p1, distance,liste)

    for noeud in points: 
        if not visited[noeud]:
            liste=[noeud]
            parcours_profondeur(noeud,liste)
            tailles.append(len(liste))
        
    
    print(sorted(tailles, reverse=True))



def main():
    """
    ne pas modifier: on charge une instance et on affiche les tailles
    """
    for instance in argv[1:]:
        distance, points = load_instance(instance)
        print_components_tailles(distance, points)
        
main()