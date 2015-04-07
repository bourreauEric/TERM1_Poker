#include "../../include/Jeu/Jeu.h"

#include <iterator>

Jeu::Jeu(int nbJoueur, int blindDepart, int cave) : actions(nbJoueur,ACTION::PAS_ENCORE_D_ACTION){
	srand((unsigned)time(0));
    this->initialisationTable(nbJoueur, cave);
    this->deck = nouveauDeck();
	this->melange();
	this->blind = blindDepart;
	this->joueurCourant = 0;
	this->pot = 0;
	this->nombreDeCoup = 0;
	this->dealer = 0;
    this->etape = ETAPE_JEU::PREFLOP;
}

Jeu::~Jeu(){
    for (int i = 0; i < listeJoueurs.size(); i++) {
        delete listeJoueurs.at(i);
    }
}

void Jeu::initialisationTable(int nbJoueur, int cave){

	for(int i=0; i<nbJoueur; i++){
		if( i == 0){
            Joueur *joueur = new Joueur(true,cave,i);
            joueur->setJeu(this);
            this->listeJoueurs.push_back(joueur);
		}else{
            Joueur *ia = new IntelligenceArtificielle(false,cave,i);
            ia->setJeu(this);
            this->listeJoueurs.push_back(ia);
		}
	}
}


void Jeu::distributionMain(){

    this->resetActions();
	this->distributionBlind();
	
    for(int i =0; i< (int) (2*this->listeJoueurs.size()); i++){
       if(this->listeJoueurs.at(i % this->listeJoueurs.size())->getMain().size() != 2){
            int position = rand() % deck.size();
            this->listeJoueurs.at(i % this->listeJoueurs.size())->ajouteCarte(this->deck.at(position));
            this->deck.erase(this->deck.begin() + position);
        }
    }

    this->nouvelleEtape(ETAPE_JEU::PREFLOP);
}

void Jeu::nouvelleEtape(ETAPE_JEU etape){

    this->miseCourante = 0;

    for(int i=0; i< (int)this->listeJoueurs.size(); i++){
        this->getJoueur(i)->setMiseCourante(0);
        this->getJoueur(i)->setMisePlusHaute(0);
        this->getJoueur(i)->resetCompteurActions();
    }

    this->resetActions();

    int nbCartes = 0;
    if (etape == ETAPE_JEU::FLOP) {
        nbCartes = 3;
    }
    else if (etape == ETAPE_JEU::TURN || etape == ETAPE_JEU::RIVER) {
        nbCartes = 1;
    }

    if(this->tableTmp.empty()){
        for(int i=0; i<nbCartes; i++){
                int position = rand() % deck.size();
                this->table.push_back(this->deck.at(position) );
                this->deck.erase(this->deck.begin() + position);
        }
    }else{
        for(int i=0; i<nbCartes; i++){
                this->table.push_back(this->tableTmp.at(0));
                this->deck.erase(this->tableTmp.erase(this->tableTmp.begin()));
        }
    }

    IntelligenceArtificielle *ia = static_cast<IntelligenceArtificielle*>(this->getJoueur(1));
    ia->estimationChancesDeGain();
}

void Jeu::distributionBlind(){

    this->miser((this->getDealer() + 1) % this->listeJoueurs.size(), this->getBlind());
    this->actions[(this->getDealer() + 1) % this->listeJoueurs.size()] = ACTION::PETITE_BLIND;
	
    this->relancer((this->getDealer() + 2) % this->listeJoueurs.size(),this->getBlind() );
    this->actions[(this->getDealer() + 2) % this->listeJoueurs.size()] = ACTION::GROSSE_BLIND;
	
    this->joueurCourant = (this->getDealer() + 3)  % this->listeJoueurs.size();

    for(int i=0; i<this->listeJoueurs.size(); i++){
        this->getJoueur(i)->resetCompteurActions();
    }
}

int Jeu::getResultatPartie() const {
    return this->resultatPartie;
}

int Jeu::getDealer(){
	return this->dealer;
}


void Jeu::miseAJourBlind(){
	this->blind = this->blind * 2;
}


std::vector<Carte> Jeu::nouveauDeck(){

	std::vector<Carte> deck;

    for(int i = COULEUR_CARTE::PIQUE; i <= COULEUR_CARTE::CARREAU; i++ ){
        for(int j = RANG_CARTE::AS; j<=RANG_CARTE::K; j++){
            Carte carte(j,i);
            deck.push_back(carte);
		}
	}

	return deck;
}
	
	
void Jeu::melange(){
	std::random_shuffle(this->deck.begin(), this->deck.end());
}


int Jeu::getBlind() const{
	return this->blind;
}

std::vector<Carte> Jeu::getDeck(){
    return this->deck;
}

int Jeu::getJoueurCourant() const{
	return this->joueurCourant;
}

Joueur* Jeu::getJoueur(int i){
    return this->listeJoueurs.at(i);
}


void Jeu::setJoueur(Joueur *joueur){
    this->listeJoueurs.push_back(joueur);
}


int Jeu::getPot() const{
	return this->pot;
}


void Jeu::setPot(int jetons){
	this->pot = jetons;
}

void Jeu::miser(int posJoueur, int jetons){

    this->setPot(this->getPot() + jetons);
    this->getJoueur(posJoueur)->retireJetons(jetons);
    this->miseCourante = jetons;
    this->getJoueur(posJoueur)->setMiseCourante(jetons);

    if (jetons > this->getJoueur(posJoueur)->getMisePlusHaute()) {
        this->getJoueur(posJoueur)->setMisePlusHaute(jetons);
    }

    this->getJoueur(posJoueur)->setMiseTotale(this->getJoueur(posJoueur)->getMiseTotale() + jetons);
    this->actions[this->getJoueur(posJoueur)->getPosition()] = ACTION::MISER;

    this->getJoueur(posJoueur)->getCompteurActions()[0]++;
}

void Jeu::tapis(int posJoueur){

    this->setPot(this->getPot() + this->getJoueur(posJoueur)->getCave());
	
    if(this->getJoueur(posJoueur)->getCave() > this->miseCourante){
        this->miseCourante = this->getJoueur(posJoueur)->getCave();
        this->getJoueur(posJoueur)->setMiseCourante(this->getJoueur(posJoueur)->getCave());
	}
	
    if(this->getJoueur(posJoueur)->getCave() > this->getJoueur(posJoueur)->getMisePlusHaute()){
            this->getJoueur(posJoueur)->setMisePlusHaute(this->getJoueur(posJoueur)->getCave());
    }

    this->getJoueur(posJoueur)->setMiseTotale(this->getJoueur(posJoueur)->getMiseTotale() + this->getJoueur(posJoueur)->getCave());
	
    this->getJoueur(posJoueur)->retireJetons(this->getJoueur(posJoueur)->getCave());

    this->actions[this->getJoueur(posJoueur)->getPosition()] = ACTION::TAPIS;

    this->getJoueur(posJoueur)->getCompteurActions()[0]++;
}


void Jeu::relancer(int posJoueur, int jetons){
	
    this->setPot(this->getPot() +  (this->getMiseCourante() - this->getJoueur(posJoueur)->getMisePlusHaute()));
    this->getJoueur(posJoueur)->setMiseTotale(this->getJoueur(posJoueur)->getMiseTotale() + (this->getMiseCourante() - this->getJoueur(posJoueur)->getMisePlusHaute()));

    this->getJoueur(posJoueur)->retireJetons(this->getMiseCourante() - this->getJoueur(posJoueur)->getMisePlusHaute());

    this->setPot(this->getPot() + jetons);
    this->getJoueur(posJoueur)->retireJetons(jetons);

    this->miseCourante = this->getMiseCourante() + jetons;
   // this->getJoueur(posJoueur)->setMiseCourante(this->getJoueur(posJoueur)->getMiseCourante() + jetons);
    this->getJoueur(posJoueur)->setMiseCourante(this->getMiseCourante());
    this->getJoueur(posJoueur)->setMiseTotale(this->getJoueur(posJoueur)->getMiseTotale() + jetons);

    if (jetons > this->getJoueur(posJoueur)->getMisePlusHaute()) {
        this->getJoueur(posJoueur)->setMisePlusHaute(jetons);
    }

    this->actions[this->getJoueur(posJoueur)->getPosition()] = ACTION::RELANCER;
    this->getJoueur(posJoueur)->getCompteurActions()[0]++;

}


void Jeu::suivre(int posJoueur){
	
    // Si on a assez d'argent on suit
    if(this->getJoueur(posJoueur)->getCave() >= this->miseCourante){

        int jetonsAAjouter = this->miseCourante - this->getJoueur(posJoueur)->getMiseCourante();

        this->setPot(this->getPot() + jetonsAAjouter);
        this->getJoueur(posJoueur)->setMiseCourante(this->miseCourante);
        this->getJoueur(posJoueur)->setMiseTotale(this->getJoueur(posJoueur)->getMiseTotale() + jetonsAAjouter);
        this->getJoueur(posJoueur)->retireJetons(jetonsAAjouter);
        this->actions[this->getJoueur(posJoueur)->getPosition()] = ACTION::SUIVRE;
        this->getJoueur(posJoueur)->getCompteurActions()[1]++;
	}
    else{      // Sinon on fait tapis
        this->tapis(posJoueur);
    }
}


void Jeu::checker(int posJoueur){
    this->actions[this->getJoueur(posJoueur)->getPosition()] = ACTION::CHECKER;
    this->getJoueur(posJoueur)->getCompteurActions()[2]++;
}

void Jeu::seCoucher(int posJoueur){
    this->actions[this->getJoueur(posJoueur)->getPosition()] = ACTION::SE_COUCHER;

    IntelligenceArtificielle *ia = static_cast<IntelligenceArtificielle*>(this->getJoueur(1));
    ia->remplissageDonneesProfilage();

    finPartie();
}    

bool Jeu::debutTour(){

    for(int i=0; i< (int) this->actions.size(); i++){
        if(this->actions.at(i) != ACTION::PAS_ENCORE_D_ACTION){
            return false;
        }
    }
        return true;

}


bool Jeu::finDuTour(){

	int i = 1;

    while(  i <= (int) this->listeJoueurs.size() - 1){
        if( this->actions.at( (this->getJoueurCourant() + i) % this->listeJoueurs.size() ) != ACTION::CHECKER
        &&  this->actions.at( (this->getJoueurCourant() + i) % this->listeJoueurs.size() ) != ACTION::SUIVRE
        &&  this->actions.at( (this->getJoueurCourant() + i) % this->listeJoueurs.size() ) != ACTION::SE_COUCHER){
			return false;
		}
		
		i++;
	}
	
    return (this->actions.at(this->getJoueurCourant()) != ACTION::PAS_ENCORE_D_ACTION && this->actions.at(this->getJoueurCourant()) != ACTION::GROSSE_BLIND);
}


ACTION Jeu::getAction() const{
    return this->actions.at(this->getJoueurCourant());
}

ETAPE_JEU Jeu::getEtape() const{
    return this->etape;
}

bool Jeu::prochainJoueur(){
	
    this->joueurCourant = (this->joueurCourant + 1) % this->listeJoueurs.size();

    if (this->finDuTour()) {
        IntelligenceArtificielle *ia = static_cast<IntelligenceArtificielle*>(this->getJoueur(1));

        ia->remplissageDonneesProfilage();

        // Fin de la partie
        if (this->getEtape() == ETAPE_JEU::RIVER) {
            finPartie();
            return false;
        }
        else {
            // On incrémente l'étape courante en passant par des entiers
            if (this->etape < ETAPE_JEU::NB_ETAPES) {
                this->etape = static_cast<ETAPE_JEU>(static_cast<int>(this->etape) + 1);
            }

            this->nouvelleEtape(this->getEtape());
        }
    }

    return true;
}

void Jeu::resetActions(){
	for(int i=0; i< (int) this->actions.size(); i++){
        this->actions.at(i) = ACTION::PAS_ENCORE_D_ACTION;
        this->getJoueur(i)->setMiseCourante(0);
        this->getJoueur(i)->setMisePlusHaute(0);
        this->getJoueur(i)->setMiseTotale(0);
	}
}

void Jeu::finPartie() {

    std::vector<Joueur*> joueursRestants;

    for(Joueur *joueur : this->listeJoueurs){
        if(this->estCouche(joueur->getPosition())){
            joueursRestants.push_back(joueur);
        }
    }

    int retour;
    if(joueursRestants.size() != 1){

        int comparaisonMains = Evaluateur::comparerMains(this->getTable(), this->getJoueur(0)->getMain(), this->getJoueur(1)->getMain());

        if(comparaisonMains == GAGNE){
            this->getJoueur(0)->ajouteJetons(this->getPot());
            retour = GAGNE;
        }else if(comparaisonMains == PERDU){
            this->getJoueur(1)->ajouteJetons(this->getPot());
            retour = PERDU;
        }else{
            retour = EGALITE;
            if(this->getPot() % 2 == 0){
                this->getJoueur(0)->ajouteJetons(this->getPot() / 2 );
                this->getJoueur(1)->ajouteJetons(this->getPot() / 2 );
            }else{
                this->setPot(this->getPot() -1);
                this->getJoueur(0)->ajouteJetons(this->getPot() / 2 );
                this->getJoueur(1)->ajouteJetons(this->getPot() / 2 );
            }
        }
    }else{

        if(joueursRestants.at(0)->getPosition() == 0){
            retour = GAGNE;
        }else{
            retour = PERDU;
        }
        this->getJoueur(joueursRestants.at(0)->getPosition())->ajouteJetons(this->getPot());
    }

    resultatPartie = retour;

    IntelligenceArtificielle *ia = static_cast<IntelligenceArtificielle*>(this->getJoueur(1));  
    ia->calculProfilGlobalJoueur();

    ia->ecritureScenariosDeTests();

    this->etape = ETAPE_JEU::PREFLOP;
}

std::vector<Carte> Jeu::getTable() const{
    return this->table;
}

std::vector<ACTION>  Jeu::getListeActions() const{
    return this->actions;
}

int Jeu::getMiseCourante(){
    return this->miseCourante;
}

void Jeu::nouvelleMain(){

	this->setPot(0);
	this->table.clear();
	
    for(int i =0; i< (int) this->listeJoueurs.size(); i++){
        this->getJoueur(i)->videMain();
	}
	
	this->deck = nouveauDeck();

    this->getJoueur(this->dealer)->changeDealer();
    this->dealer = (this->dealer + 1) % this->listeJoueurs.size();
    this->getJoueur(this->dealer)->changeDealer();

    this->etape = ETAPE_JEU::PREFLOP;
}


bool Jeu::peutChecker(int posJoueur){
    //On peut checker quand le joueur précédent a checké ou suivi.
    if(posJoueur==0){
        //Si l'action de l'autre joueur est miser, relancer ou grosse blinde, on retourne false
        if(actions[1]==ACTION::MISER || actions[1]==ACTION::RELANCER || actions[1]==ACTION::GROSSE_BLIND){

            return false;
        }
    }
    else{
        if(actions[0]==ACTION::MISER || actions[0]==ACTION::RELANCER || actions[0]==ACTION::GROSSE_BLIND){
            return false;
        }
    }
    return true;
}

bool Jeu::peutMiser(int posJoueur, int jetons){

    //On peut miser quand le joueur précédent a checké
    if(posJoueur==0){
        if(actions[1]==ACTION::MISER || actions[1]==ACTION::RELANCER || actions[1]==ACTION::GROSSE_BLIND || actions[1]==ACTION::SUIVRE){

            return false;
        }
    }
    else{
        if(actions[0]==ACTION::MISER || actions[0]==ACTION::RELANCER || actions[0]==ACTION::GROSSE_BLIND || actions[0]==ACTION::SUIVRE){
            return false;
        }
    }

    // On regarde si on a assez d'argent
    if(jetons==0){
        return false;
    }
    else if (this->getJoueur(posJoueur)->getCave() >= jetons) {
        return true;
    }

    return false;
}


bool Jeu::peutRelancer(int posJoueur, int jetons){

    //On peut pas relancer quand le joueur précédent a checké
    if(posJoueur==0){
        if(actions[1]==ACTION::CHECKER){

            return false;
        }
    }


    if((getMiseCourante()*2)<=getJoueur(posJoueur)->getCave()){

    // On regarde si on a assez d'argent
        if (this->getJoueur(posJoueur)->getCave() >= jetons) {
            return true;
        }
        else{
            return false;
        }
    }
    else{
        return false;
    }

    return false;
}

bool Jeu::peutSuivre(int posJoueur){

    //On peut suivre quand le joueur précédent a misé, relancé ou grosse blind

    if(posJoueur==0){
        if(actions[1]==ACTION::CHECKER || actions[1]==ACTION::PETITE_BLIND){
            return false;
        }
    }

    return true;
}

bool Jeu::estCouche(int posJoueur) const {
    return (this->getListeActions().at(posJoueur) != ACTION::SE_COUCHER);
}

void Jeu::executerAction(int posJoueur, ACTION action, int mise){
    int relance;
    switch (action) {
        case ACTION::CHECKER:
            if (peutChecker(posJoueur)) {
                checker(posJoueur);
            }
            break;

        case ACTION::MISER:
            if (peutMiser(posJoueur, mise)) {
                miser(posJoueur, mise);
            }
            break;

        case ACTION::SUIVRE:
            if (peutSuivre(posJoueur)) {
                suivre(posJoueur);
            }
            break;

        case ACTION::RELANCER:
            relance = (mise < 2 * getMiseCourante()) ? 2 * getMiseCourante() : mise;
            if (peutRelancer(posJoueur, relance)) {
                relancer(posJoueur, relance);
            }
            break;

        case ACTION::SE_COUCHER:
            seCoucher(posJoueur);
            break;

        default:
            break;
    }
}

void Jeu::affectationCarte(std::vector<int> listeId){

    int pos = 0;

    for(int i=0; i< 9; i++){

        if(listeId.at(i) != -1){
            pos = 0;
            for(Carte carte : this->getDeck()){
                if(carte.getId() == listeId.at(i)){
                    if(i<2){
                        this->listeJoueurs.at(0)->ajouteCarte(this->deck.at(pos));
                        this->deck.erase(this->deck.begin() + pos);
                        pos--;
                    }else if (i<4){
                        this->listeJoueurs.at(1)->ajouteCarte(this->deck.at(pos));
                        this->deck.erase(this->deck.begin() + pos);
                        pos--;
                    }else{
                        this->tableTmp.push_back(this->deck.at(pos));
                        this->deck.erase(this->deck.begin() + pos);
                        pos--;
                    }
                }

                pos++;
            }
        }
    }
}
