#include "main.hpp" 

using namespace std;

//######### Expression Reguliere ###########
regex patternArbre ("(Tree Description:[\\s]+)(.+;)");
regex patternGraph ("[=\\s]+[=\\s\\,\\|\\`\\d]+=\\d*[|]?");
regex patternNodeZoneScore ("node\\s+([0-9]+):[,?\\s+[A-Za-z]+(:?)\\s*[0-9]*\\.?[0-9]*(,)?]*");
regex patternNodeZone ("node\\s+([0-9]+):\\s+([A-Za-z]+)");
regex patternZoneScore ("([A-Za-z]+):\\s+([0-9]+\\.[0-9]+)");
regex patternScore ("([A-Za-z]+):\\s+([0|1]\\.\\d+)");
regex chiffreLigne ("=.*\\d");
regex carBizarres ("[^ \\dA-Za-z\\|=\\`\\,]");

//######### Fonctions ###########

//------------------------------------------------------------------------
//Verification fichier Mesquite
bool formatFichier (const char* fileName) {
	ifstream fichier(fileName, ios::in);
	string firstLine;
	bool mesquite = false;
	if ( fichier ) {
		getline(fichier, firstLine);
		if ( (firstLine[0] == 'M') && (firstLine[3] == 'q') ){ // verifie dans 1er ligne, position 0 et 3, char 'M' et 'q' presents
			mesquite = true; //fichier Mesquite ok
		}
	}
	return mesquite;
}

//------------------------------------------------------------------------
//recup arbres Newick dans fichier Mesquite
string lectureNewick (const char* fileName) {
	string outputArbre;
	ifstream fichier(fileName, ios::in);
	smatch match; // les differents (groupes) dans la RegEx
	if ( fichier ) {
		for (string s; getline(fichier, s); ) {		
			if ( regex_search(s, match, patternArbre) ) {
				outputArbre = match.str(2);			//recup' l'arbre Newick 					
			}
		}	
	}
	cout << "Arbre Newick : \n" << outputArbre << endl;
	return outputArbre;
}

//------------------------------------------------------------------------
//recup arbre graphique dans fichier Mesquite
string lectureGraph (const char* fileName) {
	string outputGraph;
	ifstream fichier(fileName, ios::in);
	smatch match; // les differents (groupes) dans la RegEx
	if ( fichier ) {
		for (string s; getline(fichier, s); ) {
			//s.erase(remove(s.begin(), s.end(), '\r'), s.end()); //supprime les '\r' de Windows 			
			if ( regex_search(s, match, chiffreLigne) ) {			//supprime toutes les lignes ne comportant pas de chiffre
				string result;
				regex_replace(back_inserter(result), s.begin(), s.end(), carBizarres, ""); //supprime caracteres non desires
		   		s = result;
				if ( regex_search(s, match, patternGraph) ) {
					outputGraph += match.str(0) + '\n'; 		//recup l'arbre graphique
		
				}
			}
 		}	
	}
	return outputGraph;
}
//------------------------------------------------------------------------
//recup zones geographiques inferees dans fichier Mesquite
void lectureGeographie (const char* fileName) {
	float meilleureProba;
	int nbrNoeuds = ( noeudsInferes (fileName) );
	size_t tailleTab = ( nbrNoeuds+2 );
	bool mVraisemblance = ( maximumVraisemblance (fileName));
	//string tabNoeudZone[nbrNoeuds+2]; //declaration +2 car pas d'indice 0/1
	vector<string> tabNoeudZone(tailleTab); // probleme compilation c++11 lorsque taille est une variable, remplacer tab par vector
		for ( int l = 2; l<nbrNoeuds+2; l++ )
			tabNoeudZone[l] = " ";	//initialisé a vide	
	ifstream fichier(fileName, ios::in);
	smatch match; // les differents groupes () dans la RegEx	
	if ( nbrNoeuds != -1 ) {	//verifie si tout les noeuds ont été bien lus		
	
	//associe noeud du graph avec aires geographiques		
		if ( fichier ) {
			// 1- Resultat de Parcimonie
			if ( !mVraisemblance ) { //si resultat par Parcimonie
				//cout << "Parcimonie suprême !" << endl; //debug 
				for (string ss; getline(fichier, ss); ) { //pour chaque ligne
					if ( regex_search(ss, match, patternNodeZoneScore) ) { //zone 'inference geographique' du fichier
						if ( regex_search(ss, match, patternNodeZone) ) { //si match aire geo/noeud
							int nodeTemp = stoi(match.str(1)); //convertion string/int
							const string valeur = match.str(2);
							//tabNoeudZone[nodeTemp] = match.str(2); // remplace tab par vector
							tabNoeudZone.insert ( tabNoeudZone.begin()+nodeTemp, valeur ); //stock aire geo a l'indice du noeud
						}//fin liaison noeud/zone*/			
					}//fin donnee geographique
				}//fin parcours fichier			
					
			//*/debug parcours tabNoeudZone
			cout << "Tableau 'NoeudZone' : " << endl;
			for (int l = 2; l<nbrNoeuds+2; l++){ // cas 0 et 1 jamais presents
				cout << l << " dans la zone : " << tabNoeudZone[l] << endl;
			}//fin debug */
						
			} //fin cas Parcimonie							
			// 2 - Resultat par ML
			else { // cas ML
				//cout << "Vraisemblance 4ever !" << endl; //debug
				for (string ss; getline(fichier, ss); ) { //pour chaque ligne
					if ( regex_search(ss, match, patternNodeZoneScore) ) {	//zone 'inference geographique' du fichier
						regex_search(ss, match, patternNodeZone); 
						int nodeTemp = stoi(match.str(1));				
						auto debut_ss = sregex_iterator(ss.begin(), ss.end(), patternScore);
						auto fin_ss = sregex_iterator();
						if ( distance(debut_ss, fin_ss)>1 ) { // si plusieurs zone possible 
							meilleureProba = 0.0;
							sregex_iterator(ss.begin(), ss.end(), patternScore); // recup zone et proba de cette zone			
							for (sregex_iterator ite = debut_ss; ite!=fin_ss; ++ite) {// parcours les matchs de la ligne
								smatch match = *ite;                                                
			 					float probaTemp = stof(match.str(2)); // convertion string to float
			 					if ( probaTemp > meilleureProba ) { //si nouvelle proba rencontree superieur a proba stockee
			 						meilleureProba = probaTemp; //nouvelle proba devien meilleure proba
									const string valeur = match.str(1);
			 						//tabNoeudZone[nodeTemp] = match.str(1);
									tabNoeudZone.insert ( tabNoeudZone.begin()+nodeTemp, valeur ); //aire avec la nouvelle meilleure proba stockee
      							} //fin comparaison proba
      						} //fin parcours de proba
      					} //fin cas multiple zones
      					else { //cas une unique zone
							const string valeur = match.str(2);
							tabNoeudZone.insert ( tabNoeudZone.begin()+nodeTemp, valeur ); //stock directement la zone (pas plusieurs choix)
							//tabNoeudZone[nodeTemp] = match.str(2); 
						} //fin une seul zone				
					}//fin recherche NodeZoneScore		
				}//fin parcours fichier	
				
				//*/debug parcours tabNoeudZone
				cout << "\nTableau 'NoeudZone' : " << endl;
				for (int l = 2; l<nbrNoeuds+2; l++){ // cas 0 et 1 jamais presents
					cout << l << " dans la zone : " << tabNoeudZone[l] << endl;
				}//fin debug */
				
			} //fin cas ML	
		}//fin fichier	
	} //fin verification des noeuds
	else { //si le nombre de noeud ne correspond pas
		cerr << "Erreur : dans l'algo ou avec le fichier !" << endl;
	}
}

//------------------------------------------------------------------------
//verifie si resultat parcimonie ou vraisemblances
bool maximumVraisemblance ( const char* fileName ) {
	bool mVraisemblance;
	ifstream fichier(fileName, ios::in);
	smatch match; // les differents (groupes) dans la RegEx
	if ( fichier ) {
		for (string s; getline(fichier, s); ) {
			if ( regex_search(s, match, patternNodeZoneScore) ) { 
				if ( match[2].compare(":") == 0 ) { //verifie si ML					
					mVraisemblance = true;
				}// fin Vraisemblance
				else { //cas obtenue par Parcimonie
					mVraisemblance = false;								
				}// fin parcimonie	
			}// fin d'un match RegEx
		}// fin parcous du fichier ligne a ligne
	}// fin fichier
	return mVraisemblance; 
}

//------------------------------------------------------------------------
//verifie si tous les noeuds sont bien lus
int noeudsInferes ( const char* fileName ) {
	int nbrNoeudsInferes = 0, nbrNoeudsVraisemblance = 0, nbrNoeudsParcimonie = 0;	
	ifstream fichier(fileName, ios::in);
	smatch match; // les differents (groupes) dans la RegEx
	if ( fichier ) {
		for (string s; getline(fichier, s); ) {
			if ( regex_search(s, match, patternNodeZoneScore) ) { 
				nbrNoeudsInferes++;
				if ( match[2].compare(":") == 0 ) { //verifie si ML					
					nbrNoeudsVraisemblance++;
				}// fin Vraisemblance
				else { //cas obtenue par Parcimonie
					nbrNoeudsParcimonie++;										
				}// fin parcimonie	
			}// fin d'un match RegEx
		}// fin parcous du fichier ligne a ligne
	}// fin fichier
	if ( (nbrNoeudsInferes == nbrNoeudsVraisemblance) || (nbrNoeudsInferes == nbrNoeudsParcimonie) ) {
		return nbrNoeudsInferes;
	}
	else {
		return -1;
	}
}	

//------------------------------------------------------------------------
//verifie si noeud est orphelin
bool  EstOrphelin ( int node, int pere[] ) {
	return pere[node] == -1;
}

//------------------------------------------------------------------------
//verifie le nombre de noeud dans le graph
int compteurNoeuds ( char matrice[][NbMaxNoeuds], int colMax, int lineMax ) {
	int nbrNoeuds = 0;
	for (int c = colMax; c>=0; c--) {	//parcours chq colonne commencant par la derniere	
		for (int l = 0; l<=lineMax; l++) { //parcours chaque ligne de la colonne en cours
			if ( (isdigit(matrice[l][c])) && (!(isdigit(matrice[l][c+1]))) ) { //si le char est un chiffre
				nbrNoeuds++;
			}
		}
	}
	return nbrNoeuds;
}

//------------------------------------------------------------------------
//transforme graph biogeographie (outputGraph) en un tableau
void tableauGraph ( string outputGraph ) {
	int numLine = 0, numCol = 0, colMax = 0, lineMax = 0; //compteur de ligne et colonne	
	char matrice [NbMaxNoeuds][NbMaxNoeuds]; // tableau 2D dynamique impossible: ISO C++ forbids variable length array
	
	for (int j = 0; j<NbMaxNoeuds; j++) 
		for (int i = 0; i<NbMaxNoeuds; i++)
			matrice[i][j]=' '; //initialise la matrice avec des espaces
	for (unsigned c = 0; c<outputGraph.length(); c++) { //parcours le graph, char par char
		if ( outputGraph[c] == '\n' ) { //si saut de ligne, +1 au compteur de ligne
			numLine++;
				
			if ( numCol > colMax ) { //recupere le nombre de colonne maximale
					colMax = numCol;
			} 
			numCol = 0; //remet a zero compteur de colonne à chaque fin de ligne
			}
			else {
				matrice [numLine][numCol] = outputGraph[c]; //char stocké dans matrice
				numCol++; //+1 au compteur de colonne
			}
		lineMax = numLine; //recup le nombre total de ligne	
	}
	
	/*/debug parcours matrice
	cout << "Matrice : " << endl;
	for (int l = 0; l<lineMax; l++) { 
		for (int c = 0; c<colMax; c++) {		
			cout << matrice[l][c];
		}
		cout << endl;		
	} //fin debug */
		
	// declarations variables
	int nbrNoeuds = (compteurNoeuds (matrice, colMax, lineMax))+2; //compte le nombre de noeud dans graph
	//cout << "Nombre de noeuds de l'arbre_graphique (+2) : " << nbrNoeuds <<endl; //debug
	int pere [nbrNoeuds]; //tableau pour identifier le pere d'un noeud/feuille
	int fils [nbrNoeuds][2]; //tableau pour identifier les descendants d'un noeud
	
	for (int i = 0; i<nbrNoeuds; i++) {
	    pere[i] = fils[i][0] = fils[i][1] = -1 ; //initialisation tableaux à -1
	}
	
	bool adopte_haut = false, adopte_bas = false; //booleen pour savoir si descendants d'un noeud trouves 
	char nodeTemp[4]; //variable temporaire (stock noeud en cours)
	char noeud_haut[4]; //variable temporaire (stock 1er orphelin)
	char noeud_bas[4]; //variable temporaire (stock 2e orphelin)

	for (int colonne = colMax; colonne>=0; colonne--) {	//parcours chq colonne commencant par la derniere	
		for (int ligne = 0; ligne<=lineMax; ligne++) { //parcours chaque ligne de la colonne en cours
			
			if ( isdigit(matrice[ligne][colonne]) ) { //si le char est un chiffre
				if ( (isdigit(matrice[ligne][colonne+1])) ) { //si char dans la colonne+1 est un chiffre
					break; // passe à la ligne suivante (  for (chq lignes) )
				}
				
				// nouveau noeud non encore traité
				nodeTemp[2] = matrice[ligne][colonne];  //stocke le chiffre (des unites)
				if ( isdigit(matrice[ligne][colonne-1]) ){  //verifie s'il y a un chiffre des dizaines
					nodeTemp[1] = matrice[ligne][colonne-1]; //stock le chiffre des dizaine 
					if ( isdigit(matrice[ligne][colonne-2]) ){  //verifie s'il y a un chiffre des centaines
						nodeTemp[0] = matrice[ligne][colonne-2]; //stock chiffre centaine
					}
					else { //pas de chiffres des centaines
						nodeTemp[0] = '0'; //0 dans la 1ere case
					}						
				}
				else { // pas de chiffre des dizaines
					nodeTemp[1] = '0'; //0 dans la 2e case
				}
				nodeTemp[3] = '\0'; //fin de string avant convertion
                 	int IndNodeTemp = atoi(nodeTemp); //convertion en int
                 	//cout << '\n' << "IndNodeTemp : " << IndNodeTemp << endl; //debug
//#pb bool toujours vrai				//bool noeud = ( colonne != colMax ) ; //verifie si noeud, sinon = feuille sans fils 
				bool noeud = ( (matrice[ligne][colonne+1]) == '|' ); //noeud = digit'|' ; feuille = digit' '
				
				if ( noeud ) { // on va chercher les fils	
					//cout << "Je ne suis pas une feuille." << endl; //debug
					
					// 1 - on va chercher le fils du haut
               		int lhaut = ligne-1;
               		while ( (lhaut>=0) && (!adopte_haut) ) { //parcours chaque ligne inferieure a la ligne en cours
               			int chaut = colonne+1;
               			while ( (chaut<=colMax) && (!adopte_haut) ) { //parcours chaque colonne de ces lignes inferieures	
							if ( isdigit(matrice[lhaut][chaut]) ) { //si le char est un chiffre
								if ( (isdigit(matrice[lhaut][chaut-1])) ) { //si char dans la colonne-1 est un chiffre
									break; // passe à la colonne suivante (  for (chq c_haut) )
								}
								if ( isdigit(matrice[lhaut][chaut+1]) ) { //verifie si chiffre ou nombre à 2 chiffres
    									if ( isdigit(matrice[lhaut][chaut+2]) ) { //verifie si nombre à 2 ou 3 chiffres
    										noeud_haut[0] = matrice[lhaut][chaut]; //stock le chiffre des dizaines (!) dans 1ere case
    										noeud_haut[1] = matrice[lhaut][chaut+1]; //stock 2e chiffre dans 2e case
    										noeud_haut[2] = matrice[lhaut][chaut+2]; //stock 3e chiffre dans 1e case
    									}//fin if nombre a 3 chiffres
    									else { //cas d'un nombre a 2 chiffres
    										noeud_haut[1] = matrice[lhaut][chaut]; //stock le chiffre des dizaines (!) dans 1ere case
    										noeud_haut[2] = matrice[lhaut][chaut+1]; //stock 2e chiffre dans 2e case
    										noeud_haut[0] = '0'; //0 dans la 1ere case  									
    									}
    								}//fin if nombre a 2 chiffres
								else { // cas d'un numéro sur un seul digit
									noeud_haut[2] = matrice[lhaut][chaut];
									noeud_haut[0] = '0';
									noeud_haut[1] = '0';								    
								}
								noeud_haut[3] = '\0'; //fin de string avant convertion
								int Indnoeud_haut = atoi(noeud_haut); //convertion en int
								//cout << Indnoeud_haut << " est orphelin ?" << endl; //debug
								if ( EstOrphelin(Indnoeud_haut, pere) ) { //si on rencontre un orphelin 
									pere[Indnoeud_haut] = IndNodeTemp;
									fils[IndNodeTemp][0] = Indnoeud_haut;
									adopte_haut = true; // stoppe la recherche de descendant haut
									//cout << "Non, fils de " << IndNodeTemp << '\n' << endl; //debug
								} // on a trouve le fils du haut
							} // on avait rencontré un chiffre (un noeud = fils potentiel)
							chaut++ ;
						} //parcours les colonnes de la ligne en cours vers la dte
						lhaut --;
					} // parcours les lignes vers le haut
				  
				  	// 2 - on va chercher le fils du bas
               		int lbas = ligne+1;
               		while ( (lbas<=lineMax) && (!adopte_bas) ) { //parcours chaque ligne superieur a la ligne en cours
               			int cbas = colonne+1;
               			while ( (cbas<=colMax) && (!adopte_bas) ) { //parcours chaque colonne de ces ligne superieures	
							if ( isdigit(matrice[lbas][cbas]) ) { //si le char est un chiffre
								if ( (isdigit(matrice[lbas][cbas-1])) ) { //si char dans la colonne-1 est un chiffre
									break; // passe à la colonne suivante (  for (chq c_bas) )
								}
								if ( isdigit(matrice[lbas][cbas+1]) ) { //verifie si chiffre ou nombre à 2 chiffres
									if ( isdigit(matrice[lbas][cbas+2]) ) { //verifie si chiffre ou nombre à 3 chiffres
    										noeud_bas[0] = matrice[lbas][cbas]; //stock le chiffre des dizaines (!) dans 1ere case
    										noeud_bas[1] = matrice[lbas][cbas+1]; //stock 2e chiffre dans 2e case
    										noeud_bas[2] = matrice[lbas][cbas+2];
    									}
    									else { //cas d'un nombre à 2 chiffres
    										noeud_bas[1] = matrice[lbas][cbas]; //stock le chiffre des dizaines (!) dans 1ere case
    										noeud_bas[2] = matrice[lbas][cbas+1]; //stock 2e chiffre dans 2e case
    										noeud_bas[0] = '0'; //0 dans la case centaine		
    									}	
    								}
    								else { // cas d'un numéro sur un seul digit
								    noeud_bas[2] = matrice[lbas][cbas];
								    noeud_bas[0] = '0';
								    noeud_bas[1] = '0'; 
								}
								noeud_bas[3] = '\0'; //fin de string avant convertion
								int Indnoeud_bas = atoi(noeud_bas); //convertion en int
								//cout << Indnoeud_bas << " est orphelin ?" << endl; //debug
								if ( EstOrphelin(Indnoeud_bas, pere) ) { //si on rencontre un orphelin 
									pere[Indnoeud_bas] = IndNodeTemp;
									fils[IndNodeTemp][1] = Indnoeud_bas;
									adopte_bas = true; // stoppe la recherche de descendant bas		
									//cout << "Non, 2e fils de " << IndNodeTemp << '\n' << endl;//debug				
								} // on a trouve le fils bas
							} // on avait rencontre un noeud = fils potentiel
							cbas++ ;
						} //parcours les colonnes de la ligne en cours vers la dte
						lbas ++;
					} // parcours les lignes inferieur
					adopte_bas = false; //changment de noeud
					adopte_haut = false;					
				} // FIN DE J'avais rencontre un noeud (et non une feuille) dont on cherchait les fils
			} // le char de la matrice examiné était un digit
		}  // on va voir une ligne plus bas
	}  // on va voir une colonne plus à gche
	
	//*//debug parcours pere
	cout << "\nTableau 'Pere' : " << endl;
	for (int l = 2; l<nbrNoeuds; l++){ // cas 0 et 1 jamais présents
		if (pere[l] != -1)
		cout << l << " a pour pere : " << pere[l] << '\t' << endl;
	}//fin debug */ 

	//*/debug parcours fils
	cout << "\nTableau 'Fils' : " << endl;
	for (int l = 2; l<nbrNoeuds; l++){ // cas 0 et 1 jamais présents
		for (int c = 0; c<2; c++){ 
			if (fils[l][c] != -1)
			cout << l << " a pour fils : " << fils[l][c] << endl;
		}
	}//fin debug */
}
	
//------------------------------------------------------------------------
//######### Main ###########
int main ( int argc, char**argv ){

	if ( argc == 1 ){
  	cerr << "Vous devez executer ce programme avec des fichiers 'Mesquite' en paramètre." << endl;
    	return 1;
 	}
	
	for (int i = 1; i<argc; i++){
		bool format = formatFichier( argv[i] );
		if ( format ){ 
			cout << "Fichier(s) au format Mesquite.\n" << endl;
			lectureNewick( argv[i] );
			tableauGraph( lectureGraph( argv[i] ) );
			lectureGeographie( argv[i] );
		}
		else{
		  	cerr << "Erreur : le fichier " << argv[i] << " n'est pas au format Mesquite !" << endl;
			//return 1; //stop le programme si fichier pas au bon format
		}
	}
	return 0;
}
