#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#define BUFFER_SIZE 1024

/* =========================
   Funzione per inviare tutto
   ========================= */
void send_all(int sock, void *buffer, size_t length)
{
    size_t total = 0;

    while (total < length)
    {
        ssize_t sent = send(sock, buffer + total, length - total, 0);

        if (sent <= 0)
        {
            perror("Errore send");
            exit(EXIT_FAILURE);
        }

        total += sent;
    }
}

/* =========================
   REGIS
   ========================= */
void send_regis(int sock, char *id, char *port_str, uint16_t mdp)
{
    char buffer[BUFFER_SIZE];
    int offset = 0;

    /* REGIS */
    memcpy(buffer + offset, "REGIS ", 6);
    offset += 6;

    /* ID - 8 byte */
    memcpy(buffer + offset, id, 8);
    offset += 8;

    buffer[offset++] = ' ';

    /* PORTA - 4 byte ASCII */
    memcpy(buffer + offset, port_str, 4);
    offset += 4;

    buffer[offset++] = ' ';

    /* PASSWORD - 2 byte little endian */
    buffer[offset++] = mdp & 0xFF;
    buffer[offset++] = (mdp >> 8) & 0xFF;

    /* Terminatore */
    memcpy(buffer + offset, "+++", 3);
    offset += 3;

    send_all(sock, buffer, offset);
}

/* =========================
   CONNE
   ========================= */
void send_conne(int sock, char *id, uint16_t mdp)
{
    char buffer[BUFFER_SIZE];
    int offset = 0;

    memcpy(buffer + offset, "CONNE ", 6);
    offset += 6;

    /* ID - 8 byte */
    memcpy(buffer + offset, id, 8);
    offset += 8;

    buffer[offset++] = ' ';

    /* PASSWORD - 2 byte little endian */
    buffer[offset++] = mdp & 0xFF;
    buffer[offset++] = (mdp >> 8) & 0xFF;

    /* Terminatore */
    memcpy(buffer + offset, "+++", 3);
    offset += 3;

    send_all(sock, buffer, offset);
}

/* =========================
   FRIE?
   ========================= */
void send_frie(int sock, char *friend_id)
{
    char buffer[BUFFER_SIZE];
    int offset = 0;

    memcpy(buffer + offset, "FRIE? ", 6);
    offset += 6;

    /* ID amico - 8 byte */
    memcpy(buffer + offset, friend_id, 8);
    offset += 8;

    /* Terminatore */
    memcpy(buffer + offset, "+++", 3);
    offset += 3;

    send_all(sock, buffer, offset);
}

/* =========================
   MAIN
   ========================= */
int main(int argc, char *argv[])
{
    /*
     * IP e porta del server rimangono
     * parametri di esecuzione.
     */
    if (argc != 3)
    {
        printf("Uso: %s <ip_server> <porta>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    /* =========================
       CREAZIONE SOCKET
       ========================= */
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    /* =========================
       CONFIGURAZIONE SERVER
       ========================= */
    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server.sin_addr) <= 0)
    {
        perror("Indirizzo IP non valido");
        close(sock);
        return 1;
    }

    /* =========================
       CONNESSIONE
       ========================= */
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect");
        close(sock);
        return 1;
    }

    printf("\nConnesso al server!\n");

    /* =========================
       MENU
       ========================= */

    while (1)
    {
        int scelta;

        printf("\n=============================\n");
        printf("          MENU CLIENT\n");
        printf("=============================\n");
        printf("1. Registrazione (REGIS)\n");
        printf("2. Connessione (CONNE)\n");
        printf("3. Richiesta amicizia (FRIE?)\n");
        printf("0. Esci\n");
        printf("=============================\n");

        printf("Scelta: ");
        scanf("%d", &scelta);

        switch (scelta)
        {
            /* =========================
       REGIS
       ========================= */
        case 1:
        {
            char id[9];
            char port_str[5];
            unsigned int mdp;

            printf("\n--- REGISTRAZIONE ---\n");

            printf("Inserisci ID (8 caratteri): ");
            scanf("%8s", id);

            printf("Inserisci porta (4 cifre): ");
            scanf("%4s", port_str);

            printf("Inserisci password (0-65535): ");
            scanf("%u", &mdp);

            if (mdp > 65535)
            {
                printf("Password non valida.\n");
                break;
            }

            printf("\nInvio REGIS...\n");

            send_regis(sock, id, port_str, (uint16_t)mdp);

            break;
        }

        /* =========================
        CONNE
        ========================= */
        case 2:
        {
            char id[9];
            unsigned int mdp;

            printf("\n--- CONNESSIONE ---\n");

            printf("Inserisci ID (8 caratteri): ");
            scanf("%8s", id);

            printf("Inserisci password (0-65535): ");
            scanf("%u", &mdp);

            if (mdp > 65535)
            {
                printf("Password non valida.\n");
                break;
            }

            printf("\nInvio CONNE...\n");

            send_conne(sock, id, (uint16_t)mdp);

            break;
        }

        /* =========================
        FRIE?
        ========================= */
        case 3:
        {
            char friend_id[9];

            printf("\n--- RICHIESTA DI AMICIZIA ---\n");

            printf("Inserisci ID dell'amico (8 caratteri): ");
            scanf("%8s", friend_id);

            printf("\nInvio FRIE?...\n");

            send_frie(sock, friend_id);

            break;
        }

        /* =========================
        USCITA
        ========================= */
        case 0:
        {
            printf("Chiusura client...\n");
            close(sock);
            return 0;
        }

        /* =========================
        SCELTA NON VALIDA
        ========================= */
        default:
        {
            printf("Scelta non valida.\n");
            break;
        }
        }

        /* =========================
        RICEZIONE RISPOSTA SERVER
        ========================= */

        char response[BUFFER_SIZE];

        int n = recv(sock, response, BUFFER_SIZE - 1, 0);

        if (n > 0)
        {
            response[n] = '\0';
            printf("\nRisposta server: %s\n", response);
        }
        else if (n == 0)
        {
            printf("\nIl server ha chiuso la connessione.\n");
        }
        else
        {
            perror("recv");
        }
    }
    close(sock);
    
    return 0;
}