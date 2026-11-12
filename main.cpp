// ============================================================
//  Gestion des TD universitaires - version simplifiee (console)
//  Classes : Etudiant, Matiere, SeanceTD, GestionTD
//  Compilation : g++ -std=c++17 -Wall -Wextra -o td main.cpp
//  Execution   : ./td           (menu interactif)
//                ./td --test    (scenario de test automatique)
// ============================================================
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

using namespace std;

// ------------------------------------------------------------
//  Classe Etudiant
// ------------------------------------------------------------
class Etudiant {
private:
    string matricule;
    string nom;
    string prenom;

public:
    Etudiant(const string& matricule, const string& nom, const string& prenom)
        : matricule(matricule), nom(nom), prenom(prenom) {}

    string getMatricule() const { return matricule; }
    string getNom() const { return nom; }
    string getPrenom() const { return prenom; }

    void afficher() const {
        cout << matricule << " - " << nom << " " << prenom << endl;
    }
};

// ------------------------------------------------------------
//  Classe Matiere
// ------------------------------------------------------------
class Matiere {
private:
    string code;
    string intitule;

public:
    Matiere(const string& code, const string& intitule)
        : code(code), intitule(intitule) {}

    string getCode() const { return code; }
    string getIntitule() const { return intitule; }

    void afficher() const {
        cout << code << " - " << intitule << endl;
    }
};

// ------------------------------------------------------------
//  Classe SeanceTD
//  Une seance porte sur UNE matiere et regroupe PLUSIEURS etudiants.
// ------------------------------------------------------------
class SeanceTD {
private:
    int id;
    shared_ptr<Matiere> matiere;
    string date;
    string heure;
    string salle;
    int capacite;
    vector<shared_ptr<Etudiant>> inscrits;

public:
    SeanceTD(int id, shared_ptr<Matiere> matiere, const string& date,
             const string& heure, const string& salle, int capacite)
        : id(id), matiere(matiere), date(date), heure(heure),
          salle(salle), capacite(capacite) {}

    int getId() const { return id; }
    shared_ptr<Matiere> getMatiere() const { return matiere; }
    string getDate() const { return date; }
    string getHeure() const { return heure; }
    string getSalle() const { return salle; }
    int getCapacite() const { return capacite; }
    int getNombreInscrits() const { return static_cast<int>(inscrits.size()); }

    bool estInscrit(const string& matricule) const {
        for (const auto& e : inscrits) {
            if (e->getMatricule() == matricule) return true;
        }
        return false;
    }

    // Retourne true si l'inscription a ete faite.
    // Refuse : double inscription ou seance pleine.
    bool inscrire(shared_ptr<Etudiant> etudiant, string& raisonEchec) {
        if (estInscrit(etudiant->getMatricule())) {
            raisonEchec = "l'etudiant est deja inscrit a cette seance";
            return false;
        }
        if (getNombreInscrits() >= capacite) {
            raisonEchec = "la seance est complete";
            return false;
        }
        inscrits.push_back(etudiant);
        return true;
    }

    void afficherResume() const {
        cout << "Seance #" << id << " | " << matiere->getIntitule()
             << " | " << date << " " << heure << " | salle " << salle
             << " | " << inscrits.size() << "/" << capacite << " inscrits" << endl;
    }

    void afficherInscrits() const {
        afficherResume();
        if (inscrits.empty()) {
            cout << "  (aucun etudiant inscrit)" << endl;
            return;
        }
        int i = 1;
        for (const auto& e : inscrits) {
            cout << "  " << i++ << ". ";
            e->afficher();
        }
    }
};

// ------------------------------------------------------------
//  Classe GestionTD : regroupe les collections (vector) et les
//  operations de l'application.
// ------------------------------------------------------------
class GestionTD {
private:
    vector<shared_ptr<Etudiant>> etudiants;
    vector<shared_ptr<Matiere>> matieres;
    vector<shared_ptr<SeanceTD>> seances;
    int prochainIdSeance = 1;

    shared_ptr<Etudiant> chercherEtudiant(const string& matricule) const {
        for (const auto& e : etudiants)
            if (e->getMatricule() == matricule) return e;
        return nullptr;
    }

    shared_ptr<Matiere> chercherMatiere(const string& code) const {
        for (const auto& m : matieres)
            if (m->getCode() == code) return m;
        return nullptr;
    }

    shared_ptr<SeanceTD> chercherSeance(int id) const {
        for (const auto& s : seances)
            if (s->getId() == id) return s;
        return nullptr;
    }

public:
    bool enregistrerEtudiant(const string& matricule, const string& nom,
                             const string& prenom) {
        if (matricule.empty() || nom.empty() || prenom.empty()) {
            cout << "[ERREUR] Tous les champs sont obligatoires." << endl;
            return false;
        }
        if (chercherEtudiant(matricule)) {
            cout << "[ERREUR] Le matricule " << matricule << " existe deja." << endl;
            return false;
        }
        etudiants.push_back(make_shared<Etudiant>(matricule, nom, prenom));
        cout << "[OK] Etudiant enregistre : " << nom << " " << prenom << endl;
        return true;
    }

    bool ajouterMatiere(const string& code, const string& intitule) {
        if (code.empty() || intitule.empty()) {
            cout << "[ERREUR] Tous les champs sont obligatoires." << endl;
            return false;
        }
        if (chercherMatiere(code)) {
            cout << "[ERREUR] Le code " << code << " existe deja." << endl;
            return false;
        }
        matieres.push_back(make_shared<Matiere>(code, intitule));
        cout << "[OK] Matiere ajoutee : " << intitule << endl;
        return true;
    }

    // Retourne l'id de la seance creee, ou -1 en cas d'erreur.
    int programmerSeance(const string& codeMatiere, const string& date,
                         const string& heure, const string& salle, int capacite) {
        auto m = chercherMatiere(codeMatiere);
        if (!m) {
            cout << "[ERREUR] Matiere inconnue : " << codeMatiere << endl;
            return -1;
        }
        if (capacite <= 0) {
            cout << "[ERREUR] La capacite doit etre positive." << endl;
            return -1;
        }
        int id = prochainIdSeance++;
        seances.push_back(make_shared<SeanceTD>(id, m, date, heure, salle, capacite));
        cout << "[OK] Seance #" << id << " programmee pour " << m->getIntitule() << endl;
        return id;
    }

    bool inscrireEtudiant(const string& matricule, int idSeance) {
        auto e = chercherEtudiant(matricule);
        if (!e) {
            cout << "[ERREUR] Etudiant inconnu : " << matricule << endl;
            return false;
        }
        auto s = chercherSeance(idSeance);
        if (!s) {
            cout << "[ERREUR] Seance inconnue : #" << idSeance << endl;
            return false;
        }
        string raison;
        if (!s->inscrire(e, raison)) {
            cout << "[REFUSE] Inscription impossible : " << raison << "." << endl;
            return false;
        }
        cout << "[OK] " << e->getNom() << " " << e->getPrenom()
             << " inscrit(e) a la seance #" << idSeance << endl;
        return true;
    }

    void afficherInscritsSeance(int idSeance) const {
        auto s = chercherSeance(idSeance);
        if (!s) {
            cout << "[ERREUR] Seance inconnue : #" << idSeance << endl;
            return;
        }
        s->afficherInscrits();
    }

    void listerEtudiants() const {
        if (etudiants.empty()) { cout << "(aucun etudiant)" << endl; return; }
        for (const auto& e : etudiants) e->afficher();
    }

    void listerMatieres() const {
        if (matieres.empty()) { cout << "(aucune matiere)" << endl; return; }
        for (const auto& m : matieres) m->afficher();
    }

    void listerSeances() const {
        if (seances.empty()) { cout << "(aucune seance)" << endl; return; }
        for (const auto& s : seances) s->afficherResume();
    }
};

// ------------------------------------------------------------
//  Utilitaires de saisie
// ------------------------------------------------------------
string lireLigne(const string& invite) {
    cout << invite;
    string s;
    getline(cin, s);
    return s;
}

int lireEntier(const string& invite) {
    while (true) {
        string s = lireLigne(invite);
        try {
            size_t pos;
            int v = stoi(s, &pos);
            if (pos == s.size()) return v;
        } catch (...) {}
        cout << "Veuillez saisir un nombre entier." << endl;
        if (!cin) { cin.clear(); }
    }
}

// ------------------------------------------------------------
//  Scenario de test automatique (Partie C)
// ------------------------------------------------------------
void titre(const string& t) {
    cout << "\n=== " << t << " ===" << endl;
}

void lancerTests() {
    GestionTD app;

    titre("Test 1 : enregistrement d'etudiants");
    app.enregistrerEtudiant("E001", "AGBO", "Koffi");
    app.enregistrerEtudiant("E002", "HOUNKPATIN", "Afi");
    app.enregistrerEtudiant("E003", "DOSSOU", "Marc");
    app.enregistrerEtudiant("E001", "DOUBLON", "Test");   // matricule deja pris

    titre("Test 2 : ajout de matieres");
    app.ajouterMatiere("MAT101", "Mathematiques");
    app.ajouterMatiere("PHY101", "Physique");
    app.ajouterMatiere("INF101", "Informatique");
    app.ajouterMatiere("MAT101", "Autre");                // code deja pris

    titre("Test 3 : programmation de seances");
    int s1 = app.programmerSeance("INF101", "20/11/2026", "08:00", "Salle A1", 2);
    int s2 = app.programmerSeance("MAT101", "21/11/2026", "10:00", "Salle B2", 30);
    app.programmerSeance("XXX999", "22/11/2026", "10:00", "Salle C3", 30);  // matiere inconnue

    titre("Test 4 : inscriptions normales");
    app.inscrireEtudiant("E001", s1);
    app.inscrireEtudiant("E002", s1);
    app.inscrireEtudiant("E001", s2);

    titre("Test 5 : double inscription (doit etre refusee)");
    app.inscrireEtudiant("E001", s1);

    titre("Test 6 : seance pleine (capacite 2, doit etre refusee)");
    app.inscrireEtudiant("E003", s1);

    titre("Test 7 : etudiant ou seance inexistants");
    app.inscrireEtudiant("E999", s1);
    app.inscrireEtudiant("E003", 99);

    titre("Test 8 : liste des inscrits par seance");
    app.afficherInscritsSeance(s1);
    app.afficherInscritsSeance(s2);
}

// ------------------------------------------------------------
//  Menu interactif
// ------------------------------------------------------------
void afficherMenu() {
    cout << "\n===== GESTION DES TD =====" << endl;
    cout << "1. Enregistrer un etudiant" << endl;
    cout << "2. Ajouter une matiere" << endl;
    cout << "3. Programmer une seance de TD" << endl;
    cout << "4. Inscrire un etudiant a une seance" << endl;
    cout << "5. Afficher les inscrits d'une seance" << endl;
    cout << "6. Lister etudiants / matieres / seances" << endl;
    cout << "0. Quitter" << endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && string(argv[1]) == "--test") {
        lancerTests();
        return 0;
    }

    GestionTD app;
    int choix = -1;
    while (choix != 0) {
        afficherMenu();
        choix = lireEntier("Votre choix : ");
        if (!cin && cin.eof()) break;

        switch (choix) {
        case 1: {
            string mat = lireLigne("Matricule : ");
            string nom = lireLigne("Nom : ");
            string pre = lireLigne("Prenom : ");
            app.enregistrerEtudiant(mat, nom, pre);
            break;
        }
        case 2: {
            string code = lireLigne("Code matiere : ");
            string intitule = lireLigne("Intitule : ");
            app.ajouterMatiere(code, intitule);
            break;
        }
        case 3: {
            app.listerMatieres();
            string code = lireLigne("Code de la matiere : ");
            string date = lireLigne("Date (jj/mm/aaaa) : ");
            string heure = lireLigne("Heure (hh:mm) : ");
            string salle = lireLigne("Salle : ");
            int cap = lireEntier("Capacite : ");
            app.programmerSeance(code, date, heure, salle, cap);
            break;
        }
        case 4: {
            app.listerSeances();
            int id = lireEntier("Numero de la seance : ");
            string mat = lireLigne("Matricule de l'etudiant : ");
            app.inscrireEtudiant(mat, id);
            break;
        }
        case 5: {
            int id = lireEntier("Numero de la seance : ");
            app.afficherInscritsSeance(id);
            break;
        }
        case 6:
            cout << "\n-- Etudiants --" << endl;
            app.listerEtudiants();
            cout << "\n-- Matieres --" << endl;
            app.listerMatieres();
            cout << "\n-- Seances --" << endl;
            app.listerSeances();
            break;
        case 0:
            cout << "Au revoir." << endl;
            break;
        default:
            cout << "Choix invalide." << endl;
        }
    }
    return 0;
}
