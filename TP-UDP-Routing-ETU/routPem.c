/*=============================================================*/
//Programme simulant un protocole de routage dynamique simplifié
// Ce programme code uniquement le comportement
// d'émetteur d'une annonce de routage
// vers UN SEUL routeur voisin pour UN échange initial de routes
// T. Desprats - Novembre 2022

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h> 
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <err.h>

#include "tabrout.h"

#define LOCALHOST "127.0.0.1"
#define NO_BASE_PORT 17900


/* =================================================================== */
/* FONCTION PRINCIPALE : PEER PROCESSUS DE ROUTAGE ROLE EMETTEUR ONLY  */
/* =================================================================== */

int main(int argc, char **argv) {

    // Usage routPem IDIP@ssRouter  MyNumberRouter NeigborNumberRouter
    // Example routPem 10.1.1.1 1 2
    char idInitConfigFile[20]; 
    char myId[32]; 
    routing_table_t myRoutingTable; 
    int sock;
    
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        errx(1, "Failed to create socket");
    
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(NO_BASE_PORT);
    inet_pton(AF_INET, LOCALHOST, &(dest.sin_addr));

    /* Building ID Router from command args */
    sprintf(myId, "R%s %s", argv[2], argv[1]);
    printf("ROUTEUR : %s\n", myId);
    //printf("construction id fichier\n");
    /* Building Config File ID from command args */
    sprintf(idInitConfigFile, "R%sCfg", argv[2]);
    strcat(idInitConfigFile, ".txt");
    //printf("\n Nom fichier Configuration : %s",idInitConfigFile);
    /* Loading My Routing Table from Initial Config file */
    init_routing_table(&myRoutingTable, idInitConfigFile);
    printf("ROUTEUR : %d entrées initialement chargées \n", myRoutingTable.nb_entry);
    display_routing_table(&myRoutingTable, myId);
    char message[3];
    sprintf(message,"%hd",myRoutingTable.nb_entry);
    socklen_t adr_len = sizeof(dest);
    sendto(sock, message, strlen(message)+1, 0, (struct sockaddr *) &dest, adr_len);
    for(int i = 0;i<myRoutingTable.nb_entry;i++){
        if(sendto(sock, myRoutingTable.tab_entry[i], strlen(myRoutingTable.tab_entry[i]), 0, (const struct sockaddr *) &dest, adr_len) == -1)
            errx(1,"Failed to send");

    }

    exit(EXIT_SUCCESS);
}
