#ifndef MAIN_HPP
#define MAIN_HPP

#include <fstream> //ifstream
#include <iostream> //cout cin cerr
#include <stdio.h> //printf
#include <stdlib.h> //fonctions pour tab
#include <regex> //EXpression REGuliere
#include <string> //fonctions pour string
#include <string.h> //fonctions pour string

//######### Variable globale ###########
const int NbMaxNoeuds = 200;

//######### Fonctions cpp ###########
bool formatFichier (const char* fileName);
std::string lectureNewick (const char* fileName);
void lectureGeographie (const char* fileName);
std::string lectureGraph (const char* fileName);
bool EstOrphelin (int node, int pere[]);
int compteurNoeuds (char matrice[][NbMaxNoeuds], int colMax, int lineMax);
void tableauGraph (std::string outputGeo);
int noeudsInferes (const char* fileName);
bool maximumVraisemblance (const char* fileName);

#endif
