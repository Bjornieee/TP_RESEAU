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

#define BUF_SIZE_IN 64
#define IPV4_ADR_STRLEN 16
#define LOCALHOST "127.0.0.1"
#define NO_BASE_PORT 17900

void sendTable(routing_table_t myRoutingTable, int sock, struct sockaddr_in sk_addr)
{
    char message[3];
    sprintf(message, "%hd", myRoutingTable.nb_entry);
    socklen_t adr_len = sizeof(sk_addr);
    sendto(sock, message, strlen(message) + 1, 0, (struct sockaddr *)&sk_addr, adr_len);
    for (int i = 0; i < myRoutingTable.nb_entry; i++)
    {
        if (sendto(sock, myRoutingTable.tab_entry[i], strlen(myRoutingTable.tab_entry[i]), 0, (const struct sockaddr *)&sk_addr, adr_len) == -1)
            errx(1, "Failed to send");
    }
}

void receiveTable(routing_table_t myRoutingTable, int NeighborNumber)
{
    int sock1;
    if ((sock1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        errx(1, "Failed to create socket");
    char message[3], message2[35];
    struct sockaddr_in sk_addr;
    memset(&sk_addr, 0, sizeof(sk_addr));
    sk_addr.sin_family = AF_INET;
    int port = NeighborNumber + NO_BASE_PORT;
    printf("%d\n", port);
    sk_addr.sin_port = htons(port);
    if ((bind(sock1, (const struct sockaddr *)&sk_addr, sizeof(sk_addr))) == -1)
        errx(1, "Failed to bind");
    socklen_t adr_len = sizeof(sk_addr);
    recvfrom(sock1, message, sizeof(message) + 1, 0, (struct sockaddr *)&sk_addr, &adr_len);
    for (int i = 0; i < atoi(message); i++)
    {
        if (recvfrom(sock1, message2, sizeof(message2), 0, (struct sockaddr *)&sk_addr, &adr_len) == -1)
            errx(1, "Failed to recieve");
        add_entry_routing_table(&myRoutingTable, message2);
    }
    close(sock1);
}

int main(int argc, char **argv)
{
    if (argc < 4)
        errx(1, "Usage : %s <MyIPAdresseID> <myNumber> <NeighborNumber>", argv[0]);
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
    int port;
    sscanf(argv[2], "%d", &port);
    port += NO_BASE_PORT;
    sk_addr.sin_port = htons(port);
    inet_pton(AF_INET, LOCALHOST, &(sk_addr.sin_addr));
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

    while (1)
    {
        int neighbor;
        for (int i = 0; 0 < argc - 3; i++)
        {
            neighbor = atoi(argv[3 + i]);
            switch (fork())
            {
            case -1:
                errx(1, "Failed to create a subprocess");

            case 0:
                printf("hey\n");
                switch (fork()) // Son to recieve
                {
                case -1:
                    errx(1, "Failed to create a subprocess");

                case 0:
                    printf("hey\n");
                    sendTable(myRoutingTable, sock, sk_addr);
                    break;
                }

                switch (fork()) // Son to send
                {
                case -1:
                    errx(1, "Failed to create a subprocess");

                case 0:
                    printf("hey\n");
                    receiveTable(myRoutingTable, neighbor);
                    break;
                }
                break;
            }
            wait(NULL);
        }
        sleep(30);
    }
}
