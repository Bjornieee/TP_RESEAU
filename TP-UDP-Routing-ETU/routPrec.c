/*================================================*/
// Programme simulant un protocole de routage dynamique simplifié
//  Ce programme code uniquement le comportement
//  de récpeteur d'une annonce de routage
//  émise depuis UN SEUL routeur voisin pour UN échange initial de routes
//  T. Desprats - Novembre 2022

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h> // struct sockaddr_in
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <err.h>

#include "tabrout.h"

#define BUF_SIZE_IN 64     // we should receive less...
#define IPV4_ADR_STRLEN 16 // == INET_ADDRSTRLEN
#define LOCALHOST "127.0.0.1"
#define NO_BASE_PORT 17900 // base number for computing real port number

/* =================================================================== */
/* FONCTION PRINCIPALE : PEER PROCESSUS DE ROUTAGE ROLE RECEPTEUR ONLY */
/* =================================================================== */
int main(int argc, char **argv)
{

    // Usage routPrec IDIP@ssRouter  MyNumberRouter NeigborNumberRouter
    // Example routPrec 10.1.1.1 1 2

    char idInitConfigFile[20]; // Id of the configuration file of the router
    char myId[32];             // String array representing the whole id of the Router
    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        errx(1, "Failed to create socket");

    routing_table_t myRoutingTable; // Routing TABLE

    // BINDING
    struct sockaddr_in sk_addr;
    memset(&sk_addr, 0, sizeof(sk_addr));
    sk_addr.sin_family = AF_INET;
    sk_addr.sin_port = htons(NO_BASE_PORT);
    if ((bind(sock, (const struct sockaddr *)&sk_addr, sizeof(sk_addr))) != 0)
        errx(1, "Failed to bind");

    /* Building ID Router from command args */
    sprintf(myId, "R%s %s", argv[2], argv[1]);
    printf("ROUTEUR : %s\n", myId);

    /* Building Config File ID from command args */
    sprintf(idInitConfigFile, "R%sCfg", argv[2]);
    strcat(idInitConfigFile, ".txt");

    /* Loading My Routing Table from Initial Config file */
    init_routing_table(&myRoutingTable, idInitConfigFile);
    printf("ROUTEUR : %d entrées initialement chargées \n", myRoutingTable.nb_entry);
    display_routing_table(&myRoutingTable, myId);

    char message[3], message2[35];
    socklen_t adr_len = sizeof(sk_addr);
    recvfrom(sock, message, sizeof(message) + 1, 0, (struct sockaddr *)&sk_addr, &adr_len);
    for (int i = 0; i < atoi(message); i++)
    {
        if (recvfrom(sock, message2, sizeof(message2), 0, (struct sockaddr *)&sk_addr, &adr_len) == -1)
            errx(1, "Failed to recieve");
        add_entry_routing_table(&myRoutingTable, message2);
    }
    display_routing_table(&myRoutingTable, myId);
    exit(EXIT_SUCCESS);
}
