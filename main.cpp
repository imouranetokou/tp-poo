// TP4 - Systeme Integre de Gestion d'un Departement Universitaire (version corrigee)
// Compilation: g++ -std=c++17 main.cpp -o gestion

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <cstdio>

enum class CategorieSalle { COURS, TD, TP, SOUTENANCE };
enum class NiveauLibelle { L1, L2, L3, M1, M2 };
enum class Decision { VALIDE, RATTRAPAGE, AJOURNE };

struct Creneau {
    std::string debut, fin;

    // "HH:MM" ou "H:MM" -> minutes depuis minuit, -1 si invalide
    static int enMinutes(const std::string& h) {
        int hh = -1, mm = -1;
        if (std::sscanf(h.c_str(), "%d:%d", &hh, &mm) != 2) return -1;
        if (hh < 0 || hh > 23 || mm < 0 || mm > 59) return -1;
        return hh * 60 + mm;
    }
    bool valide() const {
        int d = enMinutes(debut), f = enMinutes(fin);
        return d >= 0 && f >= 0 && d < f;
    }
    bool chevauche(const Creneau& o) const {
        return enMinutes(debut) < enMinutes(o.fin) && enMinutes(o.debut) < enMinutes(fin);
    }
};

struct Departement { std::string id, nom; };
struct Filiere { std::string id, nom, idDepartement; };
struct Niveau { std::string id, idFiliere; NiveauLibelle libelle; };
struct Salle { int numero; int capacite; CategorieSalle categorie; };
struct Enseignant { std::string matricule, nom, prenom, specialite; };
struct UE { std::string code, libelle; int credit; std::string idNiveau; };
struct Matiere { std::string code, libelle; int coefficient, credit; std::string codeUE, matriculeEnseignant; };
struct Note { std::string matriculeEtud, codeMatiere; float valeur; std::string session; };

struct Etudiant {
    std::string matricule, nom, prenom, dateNaissance, idFiliere, idNiveau;
    std::vector<Note> notes;

    float calculerMoyenne(const std::map<std::string, Matiere>& matMap) const {
        float pond = 0, coefTot = 0;
        for (const auto& n : notes) {
            auto it = matMap.find(n.codeMatiere);
            if (it != matMap.end()) {
                pond += n.valeur * it->second.coefficient;
                coefTot += it->second.coefficient;
            }
        }
        return coefTot == 0 ? 0 : pond / coefTot;
    }
    int creditsValides(const std::map<std::string, Matiere>& matMap, float seuil = 10) const {
        int c = 0;
        for (const auto& n : notes) {
            if (n.valeur >= seuil) {
                auto it = matMap.find(n.codeMatiere);
                if (it != matMap.end()) c += it->second.credit;
            }
        }
        return c;
    }
};

struct Seance {
    std::string id, date, codeMatiere, idNiveau, matriculeEns;
    int numeroSalle = 0;
    Creneau creneau;

    // Conflit si meme date + creneaux qui se chevauchent + (meme salle OU meme enseignant)
    bool conflitAvec(const Seance& o) const {
        if (date != o.date || !creneau.chevauche(o.creneau)) return false;
        bool memeSalle = numeroSalle == o.numeroSalle;
        bool memeEns = !matriculeEns.empty() && matriculeEns == o.matriculeEns;
        return memeSalle || memeEns;
    }
};

struct EmploiDuTemps {
    std::string id, semestre, idNiveau;
    std::vector<Seance> seances;
};

class SystemeGestion {
public:
    std::vector<Etudiant> etudiants;
    std::map<std::string, Matiere> matieres;
    std::map<std::string, Enseignant> enseignants;
    std::vector<Salle> salles;
    std::vector<EmploiDuTemps> edts;

    SystemeGestion() {
        enseignants["ENS001"] = {"ENS001", "KONE", "Moussa", "Programmation"};
        enseignants["ENS002"] = {"ENS002", "TRAORE", "Fatou", "Algorithmique"};

        matieres["INF301"] = {"INF301", "POO C++", 4, 6, "UE1", "ENS001"};
        matieres["INF302"] = {"INF302", "Algo Avance", 3, 5, "UE1", "ENS002"};
        matieres["INF303"] = {"INF303", "BDD", 3, 5, "UE2", "ENS001"};

        Etudiant e1;
        e1.matricule = "ETD001"; e1.nom = "ILIASSOU"; e1.prenom = "Ahmed";
        e1.idFiliere = "INFO"; e1.idNiveau = "L3";
        e1.notes = {{"ETD001", "INF301", 14, "S1"}, {"ETD001", "INF302", 12, "S1"}, {"ETD001", "INF303", 15, "S1"}};

        Etudiant e2;
        e2.matricule = "ETD002"; e2.nom = "DIALLO"; e2.prenom = "Aminata";
        e2.idFiliere = "INFO"; e2.idNiveau = "L3";
        e2.notes = {{"ETD002", "INF301", 9, "S1"}, {"ETD002", "INF302", 8, "S1"}, {"ETD002", "INF303", 11, "S1"}};

        etudiants = {e1, e2};

        salles = {{101, 60, CategorieSalle::COURS}, {102, 30, CategorieSalle::TD}};

        EmploiDuTemps edt;
        edt.id = "EDT_L3_S1"; edt.semestre = "S1"; edt.idNiveau = "L3";
        edts.push_back(edt);
    }

    bool reserverSalle(const std::string& date, const Creneau& cr, int numSalle,
                       const std::string& codeMat, const std::string& idNiv) {
        if (!cr.valide()) {
            std::cout << "ERREUR creneau invalide " << cr.debut << "-" << cr.fin << "\n";
            return false;
        }
        if (std::none_of(salles.begin(), salles.end(),
                         [&](const Salle& s) { return s.numero == numSalle; })) {
            std::cout << "ERREUR salle " << numSalle << " inexistante\n";
            return false;
        }
        auto itMat = matieres.find(codeMat);
        if (itMat == matieres.end()) {
            std::cout << "ERREUR matiere " << codeMat << " inconnue\n";
            return false;
        }
        auto itEdt = std::find_if(edts.begin(), edts.end(),
                                  [&](const EmploiDuTemps& e) { return e.idNiveau == idNiv; });
        if (itEdt == edts.end()) {
            std::cout << "ERREUR aucun emploi du temps pour le niveau " << idNiv << "\n";
            return false;
        }

        Seance s;
        s.id = "S" + std::to_string(++compteurSeance);
        s.date = date; s.creneau = cr; s.numeroSalle = numSalle;
        s.codeMatiere = codeMat; s.idNiveau = idNiv;
        s.matriculeEns = itMat->second.matriculeEnseignant;

        // Verification sur TOUS les emplois du temps (une salle/un enseignant est partage)
        for (const auto& edt : edts) {
            for (const auto& ex : edt.seances) {
                if (ex.conflitAvec(s)) {
                    bool salle = ex.numeroSalle == s.numeroSalle;
                    std::cout << "CONFLIT " << (salle ? "salle " + std::to_string(numSalle)
                                                      : "enseignant " + s.matriculeEns)
                              << " deja occupe(e) le " << date << " " << cr.debut << "-" << cr.fin << "\n";
                    return false;
                }
            }
        }
        itEdt->seances.push_back(s);
        std::cout << "Reservation OK " << s.id << " salle " << numSalle << " (" << codeMat << ")\n";
        return true;
    }

    static Decision deliberer(float moyenne) {
        if (moyenne >= 10) return Decision::VALIDE;
        if (moyenne >= 8) return Decision::RATTRAPAGE;
        return Decision::AJOURNE;
    }
    static const char* libelle(Decision d) {
        switch (d) {
            case Decision::VALIDE: return "VALIDE";
            case Decision::RATTRAPAGE: return "RATTRAPAGE";
            default: return "AJOURNE";
        }
    }

    void afficherMoyennes() const {
        std::cout << "\n=== MOYENNES ===\n" << std::fixed << std::setprecision(2);
        for (const auto& e : etudiants) {
            float m = e.calculerMoyenne(matieres);
            std::cout << e.matricule << " " << e.nom << " Moy:" << m
                      << " Credits:" << e.creditsValides(matieres)
                      << " " << libelle(deliberer(m)) << "\n";
        }
    }

    void genererReleve(const std::string& mat) const {
        auto it = std::find_if(etudiants.begin(), etudiants.end(),
                               [&](const Etudiant& e) { return e.matricule == mat; });
        if (it == etudiants.end()) {
            std::cout << "Etudiant " << mat << " introuvable\n";
            return;
        }
        std::ofstream out("releve_" + mat + ".txt");
        if (!out) { std::cout << "Impossible de creer le releve\n"; return; }
        out << std::fixed << std::setprecision(2);
        out << "RELEVE " << it->nom << " " << it->prenom << "\n"
            << "Moyenne:" << it->calculerMoyenne(matieres)
            << " Credits:" << it->creditsValides(matieres) << "\n";
        for (const auto& n : it->notes) out << n.codeMatiere << " " << n.valeur << "\n";
        std::cout << "Releve genere: releve_" << mat << ".txt\n";
    }

    void afficherDeliberation() const {
        std::cout << "\nDeliberation:\n";
        for (const auto& e : etudiants)
            std::cout << e.matricule << ": " << libelle(deliberer(e.calculerMoyenne(matieres))) << "\n";
    }

private:
    int compteurSeance = 0;
};

int main() {
    std::cout << "=== TP4 Systeme Integre ===\n";
    SystemeGestion sys;
    sys.reserverSalle("2026-05-15", {"08:00", "10:00"}, 101, "INF301", "L3");
    sys.reserverSalle("2026-05-15", {"09:00", "11:00"}, 101, "INF302", "L3"); // conflit salle
    sys.reserverSalle("2026-05-15", {"09:00", "11:00"}, 102, "INF302", "L3"); // OK
    sys.reserverSalle("2026-05-15", {"09:30", "10:30"}, 102, "INF303", "L3"); // conflit salle 102
    sys.reserverSalle("2026-05-15", {"09:00", "10:00"}, 999, "INF301", "L3"); // salle inexistante
    sys.reserverSalle("2026-05-15", {"11:00", "10:00"}, 101, "INF301", "L3"); // creneau invalide
    sys.afficherMoyennes();
    sys.genererReleve("ETD001");
    sys.genererReleve("ETD002");
    sys.afficherDeliberation();
    return 0;
}
