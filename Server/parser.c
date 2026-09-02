
#include "parser.h"

#define BUFFER_SIZE 1024

int handle_regis_parse(char *buffer, Graph *g, Vertex *client);
int handle_conne_parse(char *buffer, Graph *g, Vertex **client);
int handle_frie_parse(char *buffer, Graph *g, Vertex *client);
int handle_mess_parse(char *buffer, Graph *g, Vertex *client);
int handle_consu_parse(char *buffer, Graph *g, Vertex *client);
int handle_iquit_parse(char *buffer, Graph *g, Vertex *client);

void print_buffer_debug(unsigned char *buffer, int len)
{
    printf("[DEBUG] Buffer ricevuto (%d byte): ", len);
    for (int i = 0; i < len; i++)
    {
        if (buffer[i] >= 32 && buffer[i] <= 126)
            putchar(buffer[i]); // stampabile
        else
            printf("(%d)", buffer[i]); // non stampabile
    }
    printf("\n");
}

void *client_handler(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    Vertex *client = args->client;
    Graph *g = args->graph;
    free(args);

    unsigned char buffer[BUFFER_SIZE];
    int bytes_read;

    printf("[INFO] Client connesso: socket %d\n", client->socket_fd);

    while (1)
    {
        bytes_read = recv(client->socket_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0)
        {
            printf("[INFO] Client disconnesso: socket %d\n", client->socket_fd);
            break;
        }

        // stampa debug
        print_buffer_debug(buffer, bytes_read);

        // controllo terminatore +++
        if (bytes_read < 3 || buffer[bytes_read - 3] != '+' ||
            buffer[bytes_read - 2] != '+' || buffer[bytes_read - 1] != '+')
        {
            printf("[WARN] Terminatore +++ mancante, ignorato\n");
            continue; // non uscire, continua ad ascoltare
        }

        // Estrazione comando fino al primo spazio
        char comando[10];
        int i = 0;
        while (i < sizeof(comando) - 1 && buffer[i] != ' ' && buffer[i] != '\0')
        {
            comando[i] = buffer[i];
            i++;
        }
        comando[i] = '\0';

        printf("[DEBUG] Comando estratto: '%s'\n", comando);

        // Qui manteniamo il controllo del while e comando
        if (strncmp(comando, "REGIS", 5) == 0)
        {
            printf("[INFO] Eseguo handle_regis_parse\n");
            handle_regis_parse((char *)buffer, g, client);
        }
        else if (strncmp(comando, "CONNE", 5) == 0)
        {
            if (client->logged_in)
            {
                printf("[WARN] Operazione richiesta da client loggato\n");
                godbye(client->socket_fd, client);
                return NULL;
            }
            printf("[INFO] Eseguo handle_conne_parse\n");
            handle_conne_parse((char *)buffer, g, &client);
        }
        else if (strncmp(comando, "FRIE?", 5) == 0)
        {
            if (!client->logged_in)
            {
                printf("[WARN] Operazione richiesta da client non loggato\n");
                godbye(client->socket_fd, client);
                return NULL;
            }
            printf("[INFO] Eseguo handle_frie_parse\n");
            handle_frie_parse((char *)buffer, g, client);
        }
        else if (strncmp(comando, "MESS?", 5) == 0)
        {
            if (!client->logged_in)
            {
                printf("[WARN] Operazione richiesta da client non loggato\n");
                godbye(client->socket_fd, client);
                return NULL;
            }
            printf("[INFO] Eseguo handle_mess_parse\n");
            handle_mess_parse(buffer, g, client);
        }
        else if (strncmp(comando, "CONSU", 5) == 0)
        {
            if (!client->logged_in)
            {
                printf("[WARN] Operazione richiesta da client non loggato\n");
                godbye(client->socket_fd, client);
                return NULL;
            }
            printf("[INFO] Eseguo handle_consu_parse\n");
            handle_consu_parse(buffer, g, client);
        }
        else if (strncmp(comando, "IQUIT", 5) == 0)
        {
            if (!client->logged_in)
            {
                printf("[WARN] Operazione richiesta da client non loggato\n");
                godbye(client->socket_fd, client);
                return NULL;
            }
            printf("[INFO] Eseguo handle_iquit_parse\n");
            handle_iquit_parse(buffer, g, client);
        }

        else
        {
            printf("[WARN] Comando sconosciuto: '%s'\n", comando);
        }
    }

    close(client->socket_fd);
    printf("[INFO] Thread client terminato: socket %d\n", client->socket_fd);
    return NULL;
}

int handle_regis_parse(char *buffer, Graph *g, Vertex *client)
{
    char id[10] = {0};
    char udp_port[6] = {0};
    uint16_t mdp_value = 0;

    int i = 6; // salto "REGIS "
    int j = 0;

    printf("[DEBUG] Parsing REGIS...\n");

    // =========================
    // ID
    // =========================
    while (buffer[i] != ' ' && buffer[i] != '\0' && j < sizeof(id) - 1)
    {
        id[j] = buffer[i];
        i++;
        j++;
    }
    id[j] = '\0'; // assicuro terminazione stringa
    printf("[DEBUG] ID estratto: '%s'\n", id);

    if (!check_id(id))
    {
        printf("[ERROR] ID non valido\n");
        return EXIT_FAILURE;
    }

    if (buffer[i] != ' ')
    {
        printf("[ERROR] Spazio mancante dopo ID\n");
        return EXIT_FAILURE;
    }
    i++; // salto spazio

    // =========================
    // PORTA UDP
    // =========================
    j = 0;
    while (buffer[i] != ' ' && buffer[i] != '\0' && j < sizeof(udp_port) - 1)
    {
        udp_port[j] = buffer[i];
        i++;
        j++;
    }
    udp_port[j] = '\0';
    printf("[DEBUG] Porta UDP estratta: '%s'\n", udp_port);

    if (!check_port(udp_port))
    {
        printf("[ERROR] Porta UDP non valida\n");
        return EXIT_FAILURE;
    }

    if (buffer[i] != ' ')
    {
        printf("[ERROR] Spazio mancante dopo porta\n");
        return EXIT_FAILURE;
    }
    i++; // salto spazio

    // =========================
    // MDP (2 byte little endian)
    // =========================
    if (buffer[i + 4] == '\0')
    {
        printf("[ERROR] Buffer troppo corto per MDP\n");
        return EXIT_FAILURE;
    }

    mdp_value = converti_mdp((unsigned char *)(buffer + i));
    printf("[DEBUG] MDP estratto: %u\n", mdp_value);
    i += 2; // salto i 2 byte mdp

    // =========================
    // Terminatore +++
    // =========================
    if (buffer[i] != '+' || buffer[i + 1] != '+' || buffer[i + 2] != '+')
    {
        printf("[ERROR] Terminatore +++ mancante\n");
        return EXIT_FAILURE;
    }
    printf("[DEBUG] Terminatore +++ presente\n");

    // =========================
    // Chiamata a regis()
    // =========================
    int udp = converti_udp(udp_port);
    printf("[INFO] Chiamata regis(id='%s', port=%d, mdp=%u)\n", id, udp, mdp_value);

    return regis(id, udp, mdp_value, g, client);
}

int handle_conne_parse(char *buffer, Graph *g, Vertex **client)
{

    char id[10];
    uint16_t mdp_value;

    int i = 6;
    int j = 0;

    // =========================
    // Estrazione ID
    // =========================
    while (buffer[i] != ' ' && buffer[i] != '\0' && j < sizeof(id) - 1)
    {
        id[j] = buffer[i];
        i++;
        j++;
    }
    id[j] = '\0'; // terminazione stringa
    printf("[DEBUG] ID estratto: '%s'\n", id);

    if (!check_id(id))
    {
        printf("[ERROR] ID non valido\n");
        return EXIT_FAILURE;
    }

    i++;

    // Controllo che ci siano almeno 2 byte + "+++"
    // buffer: [mdp_low][mdp_high]['+']['+']['+']
    if (buffer[i + 2] == '\0')
        return EXIT_FAILURE;

    // Converto i 2 byte little endian
    mdp_value = converti_mdp((unsigned char *)(buffer + i));

    i += 2; // salto i 2 byte mdp

    /* =========================
       CONTROLLO TERMINATORE "+++"
       ========================= */

    if (buffer[i] != '+' || buffer[i + 1] != '+' || buffer[i + 2] != '+')
    {
        return EXIT_FAILURE;
    }

    // chiamata a conne
    return conne(id, mdp_value, g, client);
}

int handle_frie_parse(char *buffer, Graph *g, Vertex *client)
{
    printf("Informazioni Client:");
    printf("id:%s", client->id);
    printf("logged-in:%d", client->logged_in);

    char id[10] = {0}; // inizializzo a zero
    int i = 6;         // salto "FRIE? "
    int j = 0;

    printf("[DEBUG] Parsing FRIE?...\n");

    // =========================
    // Estrazione ID
    // =========================
    while (buffer[i] != '+' && buffer[i] != '\0' && j < sizeof(id) - 1)
    {
        id[j] = buffer[i];
        i++;
        j++;
    }
    id[j] = '\0'; // terminazione stringa
    printf("[DEBUG] ID estratto: '%s'\n", id);

    if (!check_id(id))
    {
        printf("[ERROR] ID non valido\n");
        return EXIT_FAILURE;
    }

    // =========================
    // Controllo terminatore +++
    // =========================
    if (buffer[i] != '+' || buffer[i + 1] != '+' || buffer[i + 2] != '+')
    {
        printf("[ERROR] Terminatore +++ mancante per FRIE?\n");
        return EXIT_FAILURE;
    }
    printf("[DEBUG] Terminatore +++ presente\n");

    // =========================
    // Chiamata a friesendrequest()
    // =========================
    printf("[INFO] Chiamata friesendrequest(id='%s')\n", id);
    return friesendrequest(id, g, client);
}

int handle_mess_parse(char *buffer, Graph *g, Vertex *client)
{

    char id[10] = {0}; // inizializzo a zero
    int i = 6;
    int j = 0;
    char mes[200];

    printf("[DEBUG] Parsing MESS?...\n");

    // =========================
    // Estrazione ID
    // =========================
    while (buffer[i] != ' ' && buffer[i] != '\0' && j < sizeof(id) - 1)
    {
        id[j] = buffer[i];
        i++;
        j++;
    }
    id[j] = '\0'; // terminazione stringa
    printf("[DEBUG] ID estratto: '%s'\n", id);

    if (!check_id(id))
    {
        printf("[ERROR] ID non valido\n");
        return EXIT_FAILURE;
    }

    i++;

    j = 0;
    // Estrazione MESS
    while (buffer[i] != '\0' && j < sizeof(mes) - 1)
    {
        if (buffer[i] == '+' && buffer[i + 1] == '+' && buffer[i + 2] == '+')
        {
            break;
        }

        mes[j] = buffer[i];
        i++;
        j++;
    }
    mes[j] = '\0'; // terminazione stringa
    printf("[DEBUG] MESS estratto: '%s'\n", mes);

    if (!check_mess(mes))
    {
        printf("[ERROR] ID non valido\n");
        return EXIT_FAILURE;
    }

    // =========================
    // Controllo terminatore +++
    // =========================
    if (buffer[i] != '+' || buffer[i + 1] != '+' || buffer[i + 2] != '+')
    {
        printf("[ERROR] Terminatore +++ mancante\n");
        return EXIT_FAILURE;
    }
    printf("[DEBUG] Terminatore +++ presente\n");

    return mess(id, mes, g, client);
}

int handle_consu_parse(char *buffer, Graph *g, Vertex *client)
{
    printf("[DEBUG] Parsing CONSU...\n");

    /*
     * CONSU non ha parametri.
     * La richiesta deve essere:
     *
     * CONSU+++
     */

    if (buffer[5] != '+' ||
        buffer[6] != '+' ||
        buffer[7] != '+')
    {
        printf("[ERROR] Terminatore +++ mancante per CONSU\n");
        return EXIT_FAILURE;
    }

    printf("[DEBUG] Formato CONSU corretto\n");

    /*
     * Chiamata alla funzione che gestisce
     * la consultazione del primo flusso.
     */
    return consu(client, g);
}

int handle_iquit_parse(char *buffer, Graph *g, Vertex *client)
{
    printf("[DEBUG] Parsing IQUIT...\n");

    if (buffer[5] != '+' ||
        buffer[6] != '+' ||
        buffer[7] != '+')
    {
        printf("[ERROR] Terminatore +++ mancante per CONSU\n");
        return EXIT_FAILURE;
    }

    printf("[DEBUG] Formato IQUIT corretto\n");

    client->logged_in = 0;
    return godbye(client->socket_fd, client);
}
