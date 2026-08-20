#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <netinet/in.h>
#include "graph.h"
#include "check_field.h"
#include "messages.h"
#include "utility.h"

typedef struct {
    Vertex *client;
    Graph *graph;
} ThreadArgs;

// funzione eseguita dal thread
void *client_handler(void *arg);

#endif
