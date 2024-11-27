#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>


//Cette classe représente un compte bancaire avec les attributs essentiels comme le numéro de compte, le nom du client, le solde et un état de verrouillage.
class CompteBancaire {
public:
    int numeroCompte;
    std::string nomClient;
    double solde;
    bool verrouille;

    CompteBancaire(int numero, const std::string &nom, double soldeInitial, bool verrouille = false)
        : numeroCompte(numero), nomClient(nom), solde(soldeInitial), verrouille(verrouille) {}
};


class Banque {
private:
    sqlite3 *db;

    static int afficherCallback(void *NotUsed, int argc, char **argv, char **azColName) {
        for (int i = 0; i < argc; i++) {
            std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << "\n";
        }
        std::cout << "\n";
        return 0;
    }
//Connecte à la base SQLite et crée la table des comptes si elle n'existe pas.

public:
    Banque(const std::string &dbName) {
        if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Erreur lors de l'ouverture de la base de donnees: " << sqlite3_errmsg(db) << "\n";
        } else {
             std::cout << "***********************************\n";
            std::cout <<"* SIMULATEUR DE GESTION DE BANQUE *\n";
            std::cout << "***********************************\n\n\n";
            std::cout <<"Base de donnees ouverte avec succes \n\n";

            const char *sql = "CREATE TABLE IF NOT EXISTS comptes ("
                              "numeroCompte INTEGER PRIMARY KEY, "
                              "nomClient TEXT, "
                              "solde REAL, "
                              "verrouille INTEGER);";
            sqlite3_exec(db, sql, 0, 0, 0);
        }
    }

    ~Banque() {
        sqlite3_close(db);
    }
    //Ajoute un nouveau compte dans la base de données
    void creerCompte(int numero, const std::string &nom, double soldeInitial) {
        std::string sql = "INSERT INTO comptes (numeroCompte, nomClient, solde, verrouille) VALUES (" +
                          std::to_string(numero) + ", '" + nom + "', " + std::to_string(soldeInitial) + ", 0);";
        char *errMsg = 0;
        if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
            std::cerr << "Erreur lors de la creation du compte: " << errMsg << "\n";
            sqlite3_free(errMsg);
        } else {
            std::cout << "Compte cree pour " << nom << " avec le numero " << numero << "\n";
        }
    }
    //Affiche les détails d’un compte bancaire en se basant sur son numéro
    void afficherCompte(int numero) {
        std::string sql = "SELECT * FROM comptes WHERE numeroCompte = " + std::to_string(numero) + ";";
        sqlite3_exec(db, sql.c_str(), afficherCallback, 0, 0);
    }
     //Effectuer une opération de dépôt sur un compte
    void depot(int numero, double montant) {
        std::string sql = "UPDATE comptes SET solde = solde + " + std::to_string(montant) +
                          " WHERE numeroCompte = " + std::to_string(numero) + ";";
        char *errMsg = 0;
        if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
            std::cerr << "Erreur lors du depot: " << errMsg << "\n";
            sqlite3_free(errMsg);
        } else {
            std::cout << "Depot de " << montant << " effectue sur le compte " << numero << "\n";
        }
    }
    //Effectuer une opération de retrait sur un compte si le solde de celui-ci est suffisant
    void retrait(int numero, double montant) {
        std::string sql = "UPDATE comptes SET solde = solde - " + std::to_string(montant) +
                          " WHERE numeroCompte = " + std::to_string(numero) + " AND solde >= " + std::to_string(montant) + ";";
        char *errMsg = 0;
        if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
            std::cerr << "Erreur lors du retrait: " << errMsg << "\n";
            sqlite3_free(errMsg);
        } else {
            std::cout << "Retrait de " << montant << " effectue sur le compte " << numero << "\n";
        }
    }

    void rechercherCompte(const std::string &recherche) {
    std::string sql = "SELECT * FROM comptes WHERE CAST(numeroCompte AS TEXT) LIKE '%" + recherche +
                      "%' OR nomClient LIKE '%" + recherche + "%';";
    char *errMsg = 0;

    std::cout << "Resultats pour la recherche: \"" << recherche << "\"\n";
    if (sqlite3_exec(db, sql.c_str(), afficherCallback, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "Erreur lors de la recherche: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}
    // Vérouiller ou déverouiller un compte
    void verrouillerCompte(int numero, bool verrouiller) {
        std::string sql = "UPDATE comptes SET verrouille = " + std::to_string(verrouiller ? 1 : 0) +
                          " WHERE numeroCompte = " + std::to_string(numero) + ";";
        char *errMsg = 0;
        if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
            std::cerr << "Erreur lors du verrouillage/deverrouillage: " << errMsg << "\n";
            sqlite3_free(errMsg);
        } else {
            std::cout << "Compte " << numero << (verrouiller ? " verrouille." : " deverrouille.") << "\n";
        }
    }
};
// Programme principale d'affichage de l'interface utilisateur (du Menu)
int main() {
    Banque banque("banque.db");
    int choix;

    do {
                std::cout << "*****************\n";
        std::cout <<"* Menu Principal\n*****************\n\n1. Creer un compte\n2. Depot\n3. Retrait\n4. Afficher compte\n5. Verrouiller/Deverrouiller compte\n6.Rechercher un compte\n7.Quitter\n\nChoisissez une option: ";
        std::cin >> choix;

        int numero;
        std::string nom;
        double montant;
        bool verrouiller;

        switch (choix) {
            case 1:
                std::cout << "Numero de compte: ";
                std::cin >> numero;
                std::cout << "Nom du client: ";
                std::cin >> nom;
                std::cout << "Solde initial: ";
                std::cin >> montant;
                banque.creerCompte(numero, nom, montant);
                break;
            case 2:
                std::cout << "Numero de compte: ";
                std::cin >> numero;
                std::cout << "Montant a deposer: ";
                std::cin >> montant;
                banque.depot(numero, montant);
                break;
            case 3:
                std::cout << "Numero de compte: ";
                std::cin >> numero;
                std::cout << "Montant a retirer: ";
                std::cin >> montant;
                banque.retrait(numero, montant);
                break;
            case 4:
                std::cout << "Numero de compte: ";
                std::cin >> numero;
                banque.afficherCompte(numero);
                break;
            case 5:
                std::cout << "Numero de compte: ";
                std::cin >> numero;
                std::cout << "1 pour verrouiller, 0 pour deverrouiller: ";
                std::cin >> verrouiller;
                banque.verrouillerCompte(numero, verrouiller);
                break;
            case 6:
    std::cout << "Entrez le numero ou le nom du client a rechercher: ";
    std::cin >> nom; // Utilisez `nom` comme variable pour simplifier.
    banque.rechercherCompte(nom);
    break;

            case 7:
                std::cout << "Au revoir!\n";
                break;
            default:
                std::cout << "Option invalide.\n";
        }
    } while (choix != 6);

    return 0;
}
