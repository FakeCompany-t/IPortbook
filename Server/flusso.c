#include "flusso.h"

/* ============================
   Creazione Flusso
   ============================ */

Flusso *createFlusso(int tipo, const char *senderid,const char *messaggio, int status)
{
    Flusso *f = malloc(sizeof(Flusso));
    if (!f)
    {
        perror("Errore allocazione Flusso");
        return NULL;
    }

    f->tipo = tipo;

    strncpy(f->senderid, senderid, 8);
    f->senderid[8] = '\0';

    if (messaggio != NULL)
    {
        f->messaggio = malloc(strlen(messaggio) + 1);
        if (!f->messaggio)
        {
            free(f);
            perror("Errore allocazione messaggio");
            return NULL;
        }
        strcpy(f->messaggio, messaggio);
    }
    else
    {
        f->messaggio = NULL;
    }

    f->status = status;
    f->next = NULL;

    return f;
}

/* ============================
   Enqueue FIFO
   ============================ */

int enqueueFlusso(Flusso **head,int tipo,const char *senderid,const char *messaggio,int status)
{
    Flusso *newFlusso = createFlusso(tipo, senderid, messaggio, status);

    if (!newFlusso)
        return 1;   // errore allocazione

    if (*head == NULL)
    {
        *head = newFlusso;
        return 0;   // successo
    }

    Flusso *temp = *head;
    while (temp->next != NULL)
        temp = temp->next;

    temp->next = newFlusso;

    return 0;   // successo
}


/* ============================
   Dequeue FIFO
   ============================ */

Flusso *dequeueFlusso(Flusso **head)
{
    if (*head == NULL)
        return NULL;

    Flusso *temp = *head;
    *head = (*head)->next;

    temp->next = NULL;
    return temp;
}

/* ============================
   Stampa Flussi
   ============================ */

void printFlussi(Flusso *head)
{
    while (head != NULL)
    {

        printf("Tipo: %d | Sender: %s | Status: %d",
               head->tipo,
               head->senderid,
               head->status);

        if (head->messaggio)
            printf(" | Messaggio: %s", head->messaggio);

        printf("\n");

        head = head->next;
    }
}

/* ============================
   Free singolo Flusso
   ============================ */

void freeFlusso(Flusso *f)
{
    if (!f)
        return;

    if (f->messaggio)
        free(f->messaggio);

    free(f);
}

/* ============================
   Free intera coda
   ============================ */

void freeAllFlussi(Flusso **head)
{
    Flusso *temp;

    while (*head != NULL)
    {
        temp = *head;
        *head = (*head)->next;
        freeFlusso(temp);
    }
}
