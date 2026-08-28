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
        ssize_t sent = send(sock, (char *)buffer + total, length - total, 0);

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
   MESS?
   ========================= */
void send_mess(int sock, char *friend_id, char *messaggio)
{
    char buffer[BUFFER_SIZE];
    int offset = 0;

    memcpy(buffer + offset, "MESS? ", 6);
    offset += 6;

    /* ID destinatario - 8 byte */
    memcpy(buffer + offset, friend_id, 8);
    offset += 8;

    buffer[offset++] = ' ';

    /* Messaggio */
    memcpy(buffer + offset, messaggio, strlen(messaggio));
    offset += strlen(messaggio);

    /* Terminatore */
    memcpy(buffer + offset, "+++", 3);
    offset += 3;

    send_all(sock, buffer, offset);
}

/* =========================
   CONSU
   ========================= */
void send_consu(int sock)
{
    char buffer[BUFFER_SIZE];

    memcpy(buffer, "CONSU+++", 8);


    send_all(sock, buffer, 8);
}

/* =========================
   OKIRF
   ========================= */
void send_okirf(int sock)
{
    char buffer[8];

    memcpy(buffer, "OKIRF+++", 8);

    send_all(sock, buffer, 8);
}

/* =========================
   NOKRF
   ========================= */
void send_nokrf(int sock)
{
    char buffer[8];

    memcpy(buffer, "NOKRF+++", 8);

    send_all(sock, buffer, 8);
}

/* =========================
   IQUIT
   ========================= */
void send_iquit(int sock)
{
    char buffer[8];

    memcpy(buffer, "IQUIT+++", 8);

    send_all(sock, buffer, 8);
}


/* =========================
   GESTIONE EIRF
   ========================= */
void handle_eirf(int sock, char *response)
{
    char id[9];

    /*
     * Formato:
     *
     * EIRF> id+++
     *
     * 6 byte "EIRF> "
     * 8 byte ID
     * 3 byte +++
     */

    memcpy(id, response + 6, 8);
    id[8] = '\0';

    printf("\n=================================\n");
    printf("       RICHIESTA DI AMICIZIA\n");
    printf("=================================\n");
    printf("Richiesta ricevuta da: %s\n", id);
    printf("1. Accetta\n");
    printf("2. Rifiuta\n");
    printf("Scelta: ");

    int scelta;

    scanf("%d", &scelta);

    if (scelta == 1)
    {
        printf("Invio OKIRF...\n");
        send_okirf(sock);
    }
    else if (scelta == 2)
    {
        printf("Invio NOKRF...\n");
        send_nokrf(sock);
    }
    else
    {
        printf("Scelta non valida. Richiesta rifiutata.\n");
        send_nokrf(sock);
    }

    /*
     * Dopo OKIRF/NOKRF il server
     * deve rispondere con ACKRF.
     */

    char ack[BUFFER_SIZE];

    int n = recv(sock, ack, BUFFER_SIZE - 1, 0);

    if (n > 0)
    {
        ack[n] = '\0';

        printf("Risposta server: ");

        /*
         * ACKRF è composto da 8 byte e non
         * è necessariamente una stringa C.
         */
        if (n >= 8 && memcmp(ack, "ACKRF+++", 8) == 0)
        {
            printf("ACKRF+++\n");
        }
        else
        {
            printf("%s\n", ack);
        }
    }
    else if (n == 0)
    {
        printf("Il server ha chiuso la connessione.\n");
    }
    else
    {
        perror("recv");
    }
}

/* =========================
   MAIN
   ========================= */
int main(int argc, char *argv[])
{
    
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
       MENU INFINITO
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
        printf("4. Invio messaggio (MESS?)\n");
        printf("5. Consultazione flussi (CONSU)\n");
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

                printf("Inserisci porta UDP (4 cifre): ");
                scanf("%4s", port_str);

                printf("Inserisci password (0-65535): ");
                scanf("%u", &mdp);

                if (mdp > 65535)
                {
                    printf("Password non valida.\n");
                    break;
                }

                printf("\nInvio REGIS...\n");

                send_regis(sock,
                           id,
                           port_str,
                           (uint16_t)mdp);

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

                send_conne(sock,
                           id,
                           (uint16_t)mdp);

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
               MESS?
               ========================= */
            case 4:
            {
                char friend_id[9];
                char messaggio[201];

                printf("\n--- INVIO MESSAGGIO ---\n");

                printf("Inserisci ID destinatario (8 caratteri): ");
                scanf("%8s", friend_id);

                /*
                 * Elimino il newline lasciato da scanf
                 * prima di usare fgets().
                 */
                getchar();

                printf("Inserisci messaggio (massimo 200 caratteri): ");
                fgets(messaggio, sizeof(messaggio), stdin);

                /*
                 * Rimuovo il newline di fgets().
                 */
                messaggio[strcspn(messaggio, "\n")] = '\0';

                if (strlen(messaggio) > 200)
                {
                    printf("Messaggio troppo lungo.\n");
                    break;
                }

                printf("\nInvio MESS?...\n");

                send_mess(sock,
                          friend_id,
                          messaggio);

                break;
            }

            /* =========================
               CONSU
               ========================= */
            case 5:
            {
                printf("\n--- CONSULTAZIONE FLUSSI ---\n");
                printf("Invio CONSU...\n");

                send_consu(sock);

                break;
            }

            /* =========================
               USCITA
               ========================= */
            case 0:
            {
                printf("IQUIT...\n");

                send_iquit(sock);
                break;
            }

            /* =========================
               SCELTA NON VALIDA
               ========================= */
            default:
            {
                printf("Scelta non valida.\n");
                continue;
            }
        }

        /* =========================
           RICEZIONE RISPOSTA SERVER
           ========================= */

        char response[BUFFER_SIZE];

        int n = recv(sock, response, BUFFER_SIZE - 1, 0);

        if (n > 0)
        {
            /*
             * EIRF è un messaggio speciale:
             * richiede una risposta dell'utente.
             */
            if (n >= 6 && memcmp(response, "EIRF> ", 6) == 0)
            {
                handle_eirf(sock, response);
            }
            else
            {
                /*
                 * Per i normali messaggi del server.
                 */
                response[n] = '\0';

                printf("\nRisposta server: %s\n", response);
            }
        }
        else if (n == 0)
        {
            printf("\nIl server ha chiuso la connessione.\n");
            break;
        }
        else
        {
            perror("recv");
            break;
        }
    }

    close(sock);

    return 0;
}