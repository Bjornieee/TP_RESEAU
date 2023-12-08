
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

#include "tresor.h"

/* =================================================================== */
/* FONCTION PRINCIPALE : SERVEUR ITERATIF                              */
/* =================================================================== */
int main() {
    // CREATE LISTEN SOCKET
    int sock = socket(AF_INET,SOCK_STREAM,0);

    // BINDING
    struct sockaddr_in sk_addr;
    memset(&sk_addr, 0, sizeof(sk_addr));
    sk_addr.sin_family = AF_INET;
    sk_addr.sin_port = htons(5555);
    if((bind(sock, (const struct sockaddr *) &sk_addr, sizeof(sk_addr))) != 0)
        errx(1,"Failed to bind");

    // LISTENING
    if((listen(sock,1)) == -1)
        errx(1,"listen");

    // ACCEPT CONNEXION
    struct sockaddr_in cliAddr;
    int newSock;
    socklen_t size = sizeof(cliAddr);
    if((newSock = accept(sock, (struct sockaddr *) &cliAddr, &size)) == -1)
        errx(1,"Failed to create new socket");

    // READ/WRITE VALUES
    char buff[6], reponse[3];
    int x, y, nbPoints;
    long testRecieve;
    while((testRecieve = recv(newSock, buff, 6, 0)) > 0){
        sscanf(buff,"%d %d",&x,&y); // not handling the convert issue that may occur because the client is the one handling it, so it'll be useless to handle it 2 times
        nbPoints = recherche_tresor(10,5,5,x,y);
        snprintf(reponse, sizeof(reponse),"%d",nbPoints);
        if(send(newSock, reponse, sizeof(reponse), 0)<0)
            errx(1,"Failed to send data");
    }
    if(testRecieve == -1)
        errx(1,"Failed to read");

    // CLOSING SOCKET
    close(newSock);
    return 0;
}
