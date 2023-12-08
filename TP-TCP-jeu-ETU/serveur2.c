/* =================================================================== */
/* Progrmame Serveur qui calcule le résultat d'un coup joué à partir   */
/* des coordonnées reçues de la part d'un client "joueur".             */
/* Version ITERATIVE : 1 seul client/joueur à la fois                  */
/* =================================================================== */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "tresor.h"

/* =================================================================== */
/* FONCTION PRINCIPALE : SERVEUR CONCURRANT                            */
/* =================================================================== */
int main(int argc, char **argv) {
    if (argc != 2)
        errx(1, "usage : %s <number of players>", argv[0]);

    // CREATE LISTEN SOCKET
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // BINDING
    struct sockaddr_in sk_addr;
    memset(&sk_addr, 0, sizeof(sk_addr));
    sk_addr.sin_family = AF_INET;
    sk_addr.sin_port = htons(5555);
    if ((bind(sock, (const struct sockaddr *) &sk_addr, sizeof(sk_addr))) != 0)
        errx(1, "Failed to bind");

    // LISTENING
    int numPlayers = (int) strtol(argv[1], NULL, 10);
    if ((listen(sock, numPlayers)) == -1)
        errx(1, "Failed to listen");

    do {
        // ACCEPT CONNEXION
        struct sockaddr_in cliAddr;
        int newSock;
        socklen_t size = sizeof(cliAddr);
        if ((newSock = accept(sock, (struct sockaddr *) &cliAddr, &size)) == -1)
            errx(1, "Failed to create new socket");

            // CREATING SONS
        switch (fork()) {
            case -1:
                errx(1, "Failed to create subprocess");
            case 0: {
                // READ/WRITE VALUES
                char buff[6], reponse[3];
                int x, y, nbPoints;
                long testRecieve;
                while ((testRecieve = recv(newSock, buff, 6, 0)) > 0) {
                    sscanf(buff, "%d %d", &x,&y); // not handling the convert issue that may occur because the client is the one handling it, so it'll be useless to handle it 2 times
                    nbPoints = recherche_tresor(10, 5, 5, x, y);
                    snprintf(reponse, sizeof(reponse), "%d", nbPoints);
                    if (send(newSock, reponse, sizeof(reponse), 0) < 0)
                        errx(1, "Failed to send data");
                    if(nbPoints == 0) break;
                }
                if (testRecieve == -1)
                    errx(1, "Failed to read");

                // CLOSING SOCKET
                close(newSock);
                exit(EXIT_SUCCESS);
            }
        }

    } while(--numPlayers);
    while(wait(NULL)!=-1); // to wait all sons before shuting down the server
    return 0;
}
