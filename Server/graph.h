#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <netinet/in.h>  // per struct in_addr
#include <pthread.h>
#include <unistd.h>

#include "flusso.h"

/* ===== STRUTTURE ===== */

// Struttura per un arco
typedef struct Edge {
    char *id;           // id del vertice di destinazione
    struct Edge *next;  // arco successivo nella lista di adiacenza
} Edge;

// Struttura per un messaggio (non usata nelle funzioni principali ma presente nei vertici)
typedef struct Message {
    char *content;
    struct Message *next;
} Message;

// Struttura per un vertice
typedef struct Vertex {
    char id[9];             // id del vertice (max 8 + terminatore)
    int mdp;                // campo generico
    struct in_addr ip_addr; // indirizzo IP
    int port_udp;           // porta UDP
    int socket_fd;          // file descriptor socket

    Edge *adj_head;         // lista archi adiacenti
    Flusso *msg_head;      // lista messaggi
    struct Vertex *next_vertex; // vertice successivo nella lista del grafo
} Vertex;

// Struttura per un grafo
typedef struct Graph {
    Vertex *vertices;       // lista dei vertici
    pthread_mutex_t mutex;  // mutex per accesso concorrente
} Graph;

/* ===== FUNZIONI ===== */

// Creazione grafo e vertice
Graph *createGraph();
Vertex *createVertex(const char *id);

// Ricerca vertice
Vertex *findVertex(Graph *g, const char *id);
bool id_exists(Graph *g, const char *id);

// Inserimento vertici
void addVertex(Graph *g, Vertex *v);

// Creazione arco e inserimento
Edge *createHalfEdge(const char *id);
void addEdge(Graph *g, const char *src, const char *dest);
void addUndirectedEdge(Graph *g, const char *v1, const char *v2);

// Stampa e liberazione memoria
void printGraph(Graph *g);
void freeGraph(Graph *g);
void freeEdge(Edge *e);
void freeEdges(Edge *head);

// Operazioni sui vertici
int vertexnumber(Graph *g);
int remove_vertex(Graph *g, const char *vertex_id);

int number_of_vertexs(Graph *g);


#endif // GRAPH_H
