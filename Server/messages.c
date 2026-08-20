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

    if (id_exists(g,id))
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

    Vertex *v = findVertex(g, id);

    if (v == NULL)
    {
        int fd = client->socket_fd;
        free(v);
        free(client);

        pthread_mutex_unlock(&g->mutex);

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

    free(client);

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
    /*
    Flusso *flux;
    flux = dequeueFlusso(client->msg_head);
    
    switch (flux->tipo)
    {
        case 1: // 1=EIRF
        
        eirf(flux->senderid, client->socket_fd, g, client->id);
        break;
        
        case 2: // 2=MESS
        
        ssem(flux->senderid, flux->messaggio, client->socket_fd);
        break;
        
        case 3: // 3=AIRC
        
        break;
        
        case 4: // 4=OOLF
        doolf(flux->senderid, flux->messaggio, client->socket_fd);
        break;
        
        default:
        break;
    }
    */ 
}

//[SSEM>␣id␣mess+++]
int ssem(char *id, char *mess, int fd)
{
    char buffer[215];
    int offset = 0;

    // "SSEM "
    memcpy(buffer + offset, "SSEM ", 6);
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
    /*
    char buffer[17];

    int offset = 0;

    // "CONNE "
    memcpy(buffer + offset, "EIRF ", 6);
    offset += 6;

    // id (8 byte)
    memcpy(buffer + offset, id, 8);
    offset += 8;

    // terminatore +++
    memcpy(buffer + offset, "+++", 3);
    offset += 3;

    send(fd, buffer, sizeof(buffer), 0);

    char response[8];
    int received = 0;

    while (received < 8)
    {
        int n = recv(fd, response + received, 8 - received, 0);
        if (n <= 0)
            return -1;
        received += n;
    }

    if (strncmp(response, "OKIRF+++", 8) == 0)
    {
        send(fd, "ACKRF+++",8, 0);
        okirf(findVertex(g, id), consuid, g);
    }
    else if (strncmp(response, "NOKRF+++", 8) == 0)
    {
        send(fd, "ACKRF+++", 8, 0);
        nokrf(findVertex(g, id), consuid);
    }
    */
}

void okirf(Vertex *client, char *id, Graph *g)
{/*
    enqueueFlusso(client->msg_head, 3, id, NULL, 1);
    addUndirectedEdge(g, client->id, id);
    // UDP
    */
}
void nokrf(Vertex *client, char *id)
{
 //   enqueueFlusso(client->msg_head, 3, id, NULL, 2);
}

//[FRIEN␣id+++]
int frien(char *id)
{
}

//[NOFRI␣id+++]
int nofriend(char *id)
{
}

//[NOCON+++]
int nocon(int fd)
{
    return send(fd, "NOCON+++", 8, 0);
}

//[IQUIT+++]
int quit(int fd)
{
    if (send(fd, "IQUIT+++", 8, 0) == -1)
    {
        perror("Errore nell'invio di GOBYE");
    }
    char *buf[8];
    if (recv(fd, buf, 8, 0) == -1)
    {
        perror("Errore nella ricezione");
    }

    return close(fd);
}
