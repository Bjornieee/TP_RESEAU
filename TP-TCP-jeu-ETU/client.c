/* =================================================================== */
// Progrmame Client à destination d'un joueur qui doit deviner la case
// du trésor. Après chaque coup le résultat retourné par le serveur est
// affiché. Le coup consite en une abcsisse et une ordonnée (x, y).
/* =================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <err.h>

#define N 10
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"

/* ====================================================================== */
/*                  Affichage du jeu en mode texte brut                   */
/* ====================================================================== */
void afficher_jeu(int jeu[N][N], int res, int points, int coups) {

    printf("\n************ TROUVEZ LE TRESOR ! ************\n");
    printf("    ");
    for (int i = 0; i < 10; i++)
        printf("  %d ", i + 1);
    printf("\n    -----------------------------------------\n");
    for (int i = 0; i < 10; i++) {
        printf("%2d  ", i + 1);
        for (int j = 0; j < 10; j++) {
            printf("|");
            switch (jeu[i][j]) {
                case -1:
                    printf(" 0 ");
                    break;
                case 0:
                    printf(GREEN " T " RESET);
                    break;
                case 1:
                    printf(YELLOW " %d " RESET, jeu[i][j]);
                    break;
                case 2:
                    printf(RED " %d " RESET, jeu[i][j]);
                    break;
                case 3:
                    printf(MAGENTA " %d " RESET, jeu[i][j]);
                    break;
            }
        }
        printf("|\n");
    }
    printf("    -----------------------------------------\n");
    printf("Pts dernier coup %d | Pts total %d | Nb coups %d\n", res, points, coups);
}

/* ====================================================================== */
/*                    Fonction principale                                 */
/* ====================================================================== */

int main(int argc, char **argv) {
    if (argc != 3)
        errx(EXIT_FAILURE, "usage : %s <ip adress> <port>", argv[0]);

    /* Init jeu */
    int jeu[N][N];
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            jeu[i][j] = -1;

    /* Creation socket TCP */
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        errx(1, "Failed to create socket");

    /* Init caracteristiques serveur distant (struct_in) */
    struct sockaddr_in serverSockAddr;
    memset(&serverSockAddr, 0, sizeof(serverSockAddr));
    serverSockAddr.sin_family = AF_INET;
    uint16_t port = strtol(argv[2], NULL, 10); // My IDE (CLion) told me to use strtol instead of atoi
    serverSockAddr.sin_port = htons(port);
    inet_pton(AF_INET, argv[1], &(serverSockAddr.sin_addr));

    /* Etablissement connexion TCP avec process serveur distant */
    if (connect(sock, (struct sockaddr *) &serverSockAddr, sizeof(serverSockAddr)) == -1)
        errx(EXIT_FAILURE, "Failed to connect");

    /* Tentatives du joueur : stoppe quand tresor trouvé */
    int lig, col, res = 0, points = 0, coups = 0;
    do {
        afficher_jeu(jeu, res, points, coups);
        printf("\nEntrer le numéro de ligne : ");
        if(scanf("%d", &lig)!=1) // IDE told me to use strtol but i preffered to keep scanf and add an error handling because with strtol my lig had lig.col value
            // example if i wanted lig = 4 and col = 5, lig would be 45
            errx(1,"You should only enter a number");
        printf("Entrer le numéro de colonne : ");
        if (scanf("%d", &col)!=1)
            errx(1,"You should only enter a number");

        /* Construction requête (serialisation en chaines de caractères) */
        char message[6]; // Doesn't support values > 99
        // but even if the value is greater than 99 it'll be 10 points in all cases, so it's not a major issue if the grid is smaller than 99x99
        snprintf(message, sizeof(message), "%d %d", lig, col);


        /* Envoi de la requête au serveur (send) */
        if (write(sock, message, sizeof(message)) < sizeof(message))
            errx(EXIT_FAILURE, "Failed to write");

        /* Réception du resultat du coup (recv) */
        char reponse[3]; // size 3 instead of 2, so that the 10 points don't send only the 1

        if (read(sock, &reponse, sizeof(reponse)) < 0)
            errx(EXIT_FAILURE, "Failed to read");
        /* Deserialisation du résultat en un entier */
        res = atoi(reponse);

        /* Mise à jour */
        if (lig >= 1 && lig <= N && col >= 1 && col <= N)
            jeu[lig - 1][col - 1] = res;
        points += res;
        coups++;

    } while (res);

    /* Fermeture connexion TCP */
    close(sock);

    /* Terminaison du jeu : le joueur a trouvé le tresor */
    afficher_jeu(jeu, res, points, coups);
    printf("\nBRAVO : trésor trouvé en %d essai(s) avec %d point(s) au total !\n\n", coups, points);
    return 0;
}
