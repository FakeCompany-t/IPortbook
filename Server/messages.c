#include "messages.h"

//[REGIS␣id␣port␣mdp+++]
int regis(char *id, int port, int mdp, Graph *g, Vertex *client)
{

    pthread_mutex_lock(&g->mutex);
    // se il numero di client registrati e' minore di 100
    if (vertexnumber(g) > 100)
    {
        int fd = client->socket_fd;
        free(client);

        pthread_mutex_unlock(&g->mutex);

        return godbye(fd);
    }

    if (id_exists(g, id))
    {
        int fd = client->socket_fd;
        free(client);

        pthread_mutex_unlock(&g->mutex);

        return godbye(fd);
    }

    // termini la popolazione del nodo
    // id
    strncpy(client->id, id, 8);
    // udp_port
    client->port_udp = port;
    // mdp
    client->mdp = mdp;
    addVertex(g, client);

    printGraph(g);

    pthread_mutex_unlock(&g->mutex);

    unsigned char resp[] = "WELCO+++";
    if (send(client->socket_fd, resp, 8, 0) == -1)
    {
        perror("Errore nell'invio di WELCO");
    }
    else
    {
        printf("[DEBUG] Inviato al client %s: ", inet_ntoa(client->ip_addr));
        for (int k = 0; k < 8; k++)
            printf("%02X ", resp[k]);
        printf("\n");
    }

    return 0;
}

/*
//[WELCO+++]
int welco(){
    //registri il client, crei il suo nodo memorizzando indirizzo IP, porta TCP e UDP, id e password
    //e mandi il messaggio [WELCO+++]
}
*/

// [GOBYE+++]
int godbye(int fd)
{
    if (send(fd, "GOBYE+++", 8, 0) == -1)
    {
        perror("Errore nell'invio di GOBYE");
    }
    else
    {
        return close(fd);
    }
}

//[CONNE␣id␣mdp+++]
int conne(char *id, int mdp, Graph *g, Vertex *client)
{

    pthread_mutex_lock(&g->mutex);

    printGraph(g);

    Vertex *v = findVertex(g, id);

    if (v == NULL)
    {
        int fd = client->socket_fd;
        remove_vertex(g,"TEMP");

        pthread_mutex_unlock(&g->mutex);
        printf("[DEBUG] Utente non trovato %s\n", id);
        return godbye(fd);
    }

    if ((uint16_t)mdp != v->mdp)
    {
        int fd = client->socket_fd;
        free(client);

        pthread_mutex_unlock(&g->mutex);

        printf("[DEBUG] Password errata per utente %s\n", id);
        return godbye(fd);
    }

    v->ip_addr = client->ip_addr;
    v->socket_fd = client->socket_fd;

    remove_vertex(g,"TEMP");

    pthread_mutex_unlock(&g->mutex);

    if (send(v->socket_fd, "HELLO+++", 8, 0) == -1)
    {
        perror("Errore nell'invio di HELLO");
    }

    return 0;
}

//[FRIE?␣id+++]
int friesendrequest(char *id, Graph *g, Vertex *sender)
{

    pthread_mutex_lock(&g->mutex);

    // se id e' presente nella lista dei client
    Vertex *dest = findVertex(g, id);

    if (dest == NULL)
    {
        pthread_mutex_unlock(&g->mutex);

        return friefailsend(sender->socket_fd);
    }
    else if (strncmp(dest->id, sender->id, 8) == 0)
    {
        pthread_mutex_unlock(&g->mutex);

        return friefailsend(sender->socket_fd);
    }

    if (enqueueFlusso(&dest->msg_head, 1, sender->id, NULL, 0)) // modifichi la codsa dei flussi del destinatario
    {
        return 1; // errore
    }

    pthread_mutex_unlock(&g->mutex);

    // manda messaggio UDP la destinatario
    return friepositivesend(sender->socket_fd);
}

//[FRIE>+++]
int friepositivesend(int fd)
{
    return send(fd, "FRIE>+++", 8, 0);
}

//[FRIE<+++]
int friefailsend(int fd)
{
    return send(fd, "FRIE<++", 8, 0);
}

//[MESS?␣id␣mess+++]
int mess(char *id, char *mess, Graph *g, Vertex *sender)
{
    pthread_mutex_lock(&g->mutex);

    // se id e' presente nella lista dei client
    Vertex *dest = findVertex(g, id);

    if (dest == NULL)
    {
        pthread_mutex_unlock(&g->mutex);

        return messfailsend(sender->socket_fd);
    }

    if (enqueueFlusso(&dest->msg_head, 2, sender->id, mess, 0)) // modifichi la codsa dei flussi del destinatario
    {
        return 1; // errore
    }

    pthread_mutex_unlock(&g->mutex);

    // manda messaggio UDP la destinatario
    return messpositivesend(sender->socket_fd);
}

//[MESS>+++]
int messpositivesend(int fd)
{
    return send(fd, "MESS>+++", 8, 0);
}

//[MESS<+++]
int messfailsend(int fd)
{
    return send(fd, "MESS<+++", 8, 0);
}

//[FLOO?␣mess+++]
int flood(char *mess, Graph *g, Vertex *sender)
{
}

//[FLOO>+++]
int floodpositivesend()
{
}

//[LIST?+++]

//[RLIST␣num-item+++]
int rlist(Graph *g, int fd)
{
    char buffer[16];

    int num_item = number_of_vertexs(g);

    snprintf(buffer, sizeof(buffer), "RLIST %03d+++", num_item);
    send(fd, buffer, sizeof(buffer), 0);

    return send(fd, buffer, sizeof(buffer), 0);
}

//[LINUM␣id+++]
void linum(int num_item, Graph *g, int fd)
{
    Vertex *aux = g->vertices;
    for (int i = 0; i < num_item; i++)
    {
        send(fd, aux->id, 8, 0);
        aux = aux->next_vertex;
    }
}

//[CONSU+++]
int consu(Vertex *client, Graph *g)
{

    Flusso *flux;
    flux = dequeueFlusso(&client->msg_head);
    if (flux == NULL)
    {
        nocon(client->socket_fd);
        return 0;
    }

    switch (flux->tipo)
    {
    case 1: // 1=EIRF

        eirf(flux->senderid, client->socket_fd, g, client->id);
        break;

    case 2: // 2=MESS

        ssem(flux->senderid, flux->messaggio, client->socket_fd);
        break;

    case 3: // 3=AIRC - risposta a richiesta di amicizia

        if (flux->status == 1)
        {
            frien(flux->senderid, client->socket_fd);
        }
        else if (flux->status == 2)
        {
            nofriend(flux->senderid, client->socket_fd);
        }
        break;

    case 4: // 4=OOLF
        doolf(flux->senderid, flux->messaggio, client->socket_fd);
        break;

    default:
        break;
    }

    /*
     * Il flusso è stato consultato,
     * quindi deve essere eliminato.
     */
    freeFlusso(flux);
    return 0;
}

//[SSEM>␣id␣mess+++]
// [SSEM> id mess+++]
int ssem(char *id, char *mess, int fd)
{
    char buffer[218];
    int offset = 0;
    int mess_len;

    /* =========================
       Controllo messaggio
       ========================= */
    if (id == NULL || mess == NULL)
    {
        return -1;
    }

    mess_len = strlen(mess);

    if (mess_len > 200)
    {
        mess_len = 200;
    }

    /* =========================
       "SSEM "
       ========================= */
    memcpy(buffer + offset, "SSEM ", 5);
    offset += 5;

    /* =========================
       ID - 8 byte
       ========================= */
    memcpy(buffer + offset, id, 8);
    offset += 8;

    /* =========================
       Spazio
       ========================= */
    buffer[offset++] = ' ';

    /* =========================
       Messaggio
       ========================= */
    memcpy(buffer + offset, mess, mess_len);
    offset += mess_len;

    /* =========================
       Terminatore +++
       ========================= */
    memcpy(buffer + offset, "+++", 3);
    offset += 3;

    /* =========================
       Invio
       ========================= */
    send(fd, buffer, offset, 0);

    return 0;
}

//[OOLF>␣id␣mess+++]
int doolf(char *id, char *mess, int fd)
{
    char buffer[215];
    int offset = 0;

    // "OOLF "
    memcpy(buffer + offset, "OOLF ", 6);
    offset += 6;

    // id (8 byte)
    memcpy(buffer + offset, id, 8);
    offset += 8;

    memcpy(buffer + offset, mess, 200);
    offset += 200;

    // terminatore +++
    memcpy(buffer + offset, "+++", 3);
    offset += 3;

    send(fd, buffer, sizeof(buffer), 0);
}

//[EIRF>␣id+++]
int eirf(char *id, int fd, Graph *g, char *consuid)
{

    char buffer[17];
    int offset = 0;

    // "EIRF "
    memcpy(buffer + offset, "EIRF> ", 6);
    offset += 6;

    // id (8 byte)
    memcpy(buffer + offset, id, 8);
    offset += 8;

    // terminatore +++
    memcpy(buffer + offset, "+++", 3);
    offset += 3;

    send(fd, buffer, sizeof(buffer), 0);

    /*
     * Attesa della risposta:
     *
     * OKIRF+++
     * oppure
     * NOKRF+++
     */

    char response[9];
    int received = 0;

    while (received < 8)
    {
        int n = recv(fd, response + received, 8 - received, 0);
        if (n <= 0)
            return -1;
        received += n;
    }

    response[8] = '\0';

    /*
     * RICHIESTA ACCETTATA
     */
    if (strcmp(response, "OKIRF+++") == 0)
    {
        /*
         * ACK al client che ha ricevuto
         * la richiesta.
         */
        send(fd, "ACKRF+++", 8, 0);

        /*
         * Troviamo il client che aveva
         * inviato la richiesta.
         */
        Vertex *requester = findVertex(g, id);

        if (requester == NULL)
        {
            return -1;
        }

        /*
         * Aggiunta dell'amicizia.
         */
        addUndirectedEdge(g, consuid, id);

        /*
         * Creazione del flusso AIRC sul
         * client che aveva fatto la richiesta.
         *
         * senderid = client che ha risposto.
         * status = 1 -> accettata.
         */
        enqueueFlusso(&requester->msg_head,
                      3,
                      consuid,
                      NULL,
                      1);

        /*
         * QUI va la notifica UDP 1XX
         * al client requester.
         */
        /*
         * notify_udp(requester, 1);
         */

        return 0;
    }

    /*
     * RICHIESTA RIFIUTATA
     */
    if (strcmp(response, "NOKRF+++") == 0)
    {
        /*
         * ACK al client che ha ricevuto
         * la richiesta.
         */
        send(fd, "ACKRF+++", 8, 0);

        /*
         * Troviamo il richiedente.
         */
        Vertex *requester = findVertex(g, id);

        if (requester == NULL)
        {
            return -1;
        }

        /*
         * Creiamo il flusso AIRC sul richiedente.
         *
         * senderid = client che ha rifiutato.
         * status = 2 -> rifiutata.
         */
        enqueueFlusso(&requester->msg_head,
                      3,
                      consuid,
                      NULL,
                      2);

        /*
         * QUI va la notifica UDP 2XX
         * al client requester.
         */
        /*
         * notify_udp(requester, 2);
         */

        return 0;
    }

    /*
     * Risposta non valida.
     */
    return -1;
}

/*

void okirf(Vertex *client, char *id, Graph *g)
{
     enqueueFlusso(client->msg_head, 3, id, NULL, 1);
     addUndirectedEdge(g, client->id, id);
     // UDP
}
void nokrf(Vertex *client, char *id)
    {
        //   enqueueFlusso(client->msg_head, 3, id, NULL, 2);
    }
*/

// [FRIEN id+++]
int frien(char *id, int fd)
{
    char buffer[17];
    int offset = 0;

    memcpy(buffer + offset, "FRIEN ", 6);
    offset += 6;

    memcpy(buffer + offset, id, 8);
    offset += 8;

    memcpy(buffer + offset, "+++", 3);
    offset += 3;

    send(fd, buffer, offset, 0);

    return 0;
}

// [NOFRI id+++]
int nofriend(char *id, int fd)
{
    char buffer[17];
    int offset = 0;

    memcpy(buffer + offset, "NOFRI ", 6);
    offset += 6;

    memcpy(buffer + offset, id, 8);
    offset += 8;

    memcpy(buffer + offset, "+++", 3);
    offset += 3;

    send(fd, buffer, offset, 0);

    return 0;
}

//[NOCON+++]
int nocon(int fd)
{
    return send(fd, "NOCON+++", 8, 0);
}

//[IQUIT+++]
int quit(int fd)
{
    if (send(fd, "GOBYE+++", 8, 0) == -1)
    {
        perror("Errore nell'invio di GOBYE");
    }
    return close(fd);
}
