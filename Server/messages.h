#ifndef MESSAGES_H
#define MESSAGES_H

#include "graph.h"
#include "flusso.h"
#include "check_field.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


/* ================= REGISTRAZIONE / CONNESSIONE ================= */

// [REGIS id port mdp+++]
int regis(char *id, int port, int mdp, Graph *g, Vertex *client);

// [GOBYE+++]
int godbye(int fd);

// [CONNE id mdp+++]
int conne(char *id, int mdp, Graph *g, Vertex *client);

// [NOCON+++]
int nocon(int fd);

// [IQUIT+++]
int quit(int fd);


/* ================= FRIEND REQUEST ================= */

// [FRIE? id+++]
int friesendrequest(char *id, Graph *g, Vertex *sender);

// [FRIE>+++]
int friepositivesend(int fd);

// [FRIE<+++]
int friefailsend(int fd);

// [EIRF> id+++]
int eirf(char *id, int fd, Graph *g,char *consuid);

// [OKIRF handling]
void okirf(Vertex *client, char *id, Graph *g);

// [NOKRF handling]
void nokrf(Vertex *client, char *id);

// [ACKRF+++]
int ackrf();

// [FRIEN id+++]
int frien(char *id);

// [NOFRI id+++]
int nofriend(char *id);


/* ================= MESSAGGI PRIVATI ================= */

// [MESS? id mess+++]
int mess(char *id, char *mess, Graph *g, Vertex *sender);

// [MESS>+++]
int messpositivesend(int fd);

// [MESS<+++]
int messfailsend(int fd);

// [SSEM> id mess+++]
int ssem(char *id, char *mess, int fd);


/* ================= FLOOD ================= */

// [FLOO? mess+++]
int flood(char *mess, Graph *g, Vertex *sender);

// [FLOO>+++]
int floodpositivesend();

// [OOLF> id mess+++]
int doolf(char *id, char *mess, int fd);


/* ================= LISTA UTENTI ================= */

// [LIST?+++]
// [RLIST num-item+++]
int rlist(Graph *g, int fd);

// [LINUM id+++]
void linum(int num_item, Graph *g, int fd);


/* ================= CONSULTA FLUSSI ================= */

// [CONSU+++]
int consu(Vertex *client, Graph *g);


#endif
