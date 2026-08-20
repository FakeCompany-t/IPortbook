#ifndef FLUSSO_H
#define FLUSSO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================
   STRUTTURA FLUSSO
   ============================ */

typedef struct flusso {
    int tipo;               // 1=EIRF, 2=MESS, 3=AIRC, 4=OOLF
    char senderid[9];       // ID mittente (8 + '\0')
    char *messaggio;        // Dinamico (NULL se non serve)
    int status;             // 1=accetta, 2=rifiuta
    struct flusso *next;    // Prossimo nella coda
} Flusso;


/* ============================
   PROTOTIPI FUNZIONI
   ============================ */

// Creazione nodo
Flusso* createFlusso(int tipo,
                     const char *senderid,
                     const char *messaggio,
                     int status);

// Inserimento FIFO nella coda del vertice
int enqueueFlusso(Flusso **head,
                   int tipo,
                   const char *senderid,
                   const char *messaggio,
                   int status);

// Rimozione FIFO
Flusso* dequeueFlusso(Flusso **head);

// Stampa coda
void printFlussi(Flusso *head);

// Liberazione singolo nodo
void freeFlusso(Flusso *f);

// Liberazione intera coda
void freeAllFlussi(Flusso **head);

#endif
