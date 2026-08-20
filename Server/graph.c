#include "graph.h"


/* ===== CREAZIONE GRAFO ===== */
Graph *createGraph() {
    Graph *g = malloc(sizeof(Graph));
    g->vertices = NULL;
    pthread_mutex_init(&g->mutex, NULL);
    return g;
}

/* ===== CREAZIONE VERTICE ===== */
Vertex *createVertex(const char *id) {
    Vertex *v = malloc(sizeof(Vertex));
    if (!v) return NULL;  // sempre buona pratica controllare malloc

    // Copia l'id con terminazione
    strncpy(v->id, id, 8);
    v->id[8] = '\0';

    // Inizializza i campi
    v->mdp = 0;
    v->ip_addr.s_addr = 0;
    v->port_udp = 0;         // ora è un int
    v->socket_fd = -1;

    v->adj_head = NULL;
    v->msg_head = NULL;
    v->next_vertex = NULL;

    return v;
}

/* ===== RICERCA VERTICE ===== */
Vertex *findVertex(Graph *g, const char *id) {
    Vertex *curr = g->vertices;

    while (curr != NULL) {
        if (strcmp(curr->id, id) == 0)
            return curr;
        curr = curr->next_vertex;
    }

    return NULL;
}

/* ===== AGGIUNGI VERTICE ===== */
void addVertex(Graph *g, Vertex *v) {
    if (findVertex(g, v->id) != NULL)
        return;

    v->next_vertex = g->vertices;
    g->vertices = v;
}

/* ===== CREAZIONE MEZZO ARCO ===== */
Edge *createHalfEdge(const char *id) {
    Edge *e = malloc(sizeof(Edge));
    if (e == NULL) {
        perror("Errore nell'allocazione della memoria");
        return NULL;
    }

    e->id = strdup(id);  // crea una copia della stringa
    if (e->id == NULL) {
        perror("Errore nell'allocazione della memoria per id");
        free(e);
        return NULL;
    }

    e->next = NULL;
    return e;
}

/* ===== INSERIMENTO ARCO ORIENTATO===== */
void addEdge(Graph *g, const char *src, const char *dest) {
    Vertex *v = findVertex(g, src);
    if (!v) return;

    Edge *e = createHalfEdge(dest);
    if (!e) return;

    e->next = v->adj_head;  // inserimento in testa
    v->adj_head = e;
}

/* ===== INSERIMENTO ARCO NON ORIENTATO===== */
void addUndirectedEdge(Graph *g, const char *v1, const char *v2) {
    addEdge(g, v1, v2);
    addEdge(g, v2, v1);
}

/* ===== STAMPA GRAFO===== */
void printGraph(Graph *g) {
    for (Vertex *v = g->vertices; v != NULL; v = v->next_vertex) {
        printf("%s: ", v->id);
        for (Edge *e = v->adj_head; e != NULL; e = e->next) {
            printf("%s -> ", e->id);
        }
        printf("NULL\n");
    }
}

void freeGraph(Graph *g) {
    if (!g) return;
    pthread_mutex_destroy(&g->mutex);
    Vertex *v = g->vertices;
    while (v) {
        Vertex *tmp_v = v;
        v = v->next_vertex;

        freeEdges(tmp_v->adj_head);  // libera tutti gli archi del vertice
        free(tmp_v);
    }

    free(g);
}

void freeEdge(Edge *e) {
    if (e) {
        free(e->id);
        free(e);
    }
}

void freeEdges(Edge *head) {
    while (head) {
        Edge *tmp = head;
        head = head->next;
        freeEdge(tmp);
    }
}

int vertexnumber(Graph *g){
    int counter = 0;

    Vertex *v = g->vertices;
    while (v!=NULL)
    {
        counter++;
        v=v->next_vertex;
    }

    return counter;
}


int remove_vertex(Graph *g, const char *vertex_id) {
    if (!g || !g->vertices) return -1;

    Vertex *prev = NULL;
    Vertex *curr = g->vertices;

    while (curr) {
        if (strcmp(curr->id, vertex_id) == 0) {
            // Copia del socket_fd
            int socket_copy = dup(curr->socket_fd);
            if (socket_copy < 0) {
                perror("dup");
                return -1;
            }

            // Rimuove il vertice dalla lista
            if (prev) {
                prev->next_vertex = curr->next_vertex;
            } else {
                g->vertices = curr->next_vertex;
            }

            // Libera memoria
            Edge *e = curr->adj_head;
            while (e) {
                Edge *tmp = e;
                e = e->next;
                free(tmp->id);
                free(tmp);
            }

            free(curr);
            return socket_copy;
        }
        prev = curr;
        curr = curr->next_vertex;
    }

    // Vertice non trovato
    return -1;
}

bool id_exists(Graph *g, const char *id) {
    if (g == NULL || id == NULL)
        return false;

    Vertex *curr = g->vertices;
    while (curr != NULL) {
        if (strcmp(curr->id, id) == 0) {
            return true;
        }
        curr = curr->next_vertex;
    }

    return false;
}


int number_of_vertexs(Graph *g){

    int counter = 0;

    Vertex *aux = g->vertices;

    while (aux!=NULL)
    {
        counter++;
        aux=aux->next_vertex;
    }
    
    return counter;
}