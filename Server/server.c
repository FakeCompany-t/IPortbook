//gcc server.c graph.c parser.c flusso.c check_field.c messages.c utility.c -o server -pthread && ./server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "graph.h"
#include "parser.h"

#define PORT 8080
#define BACKLOG 10

int main()
{

    Graph *g = createGraph();

    int server_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server_sock, BACKLOG) < 0)
    {
        perror("listen");
        exit(1);
    }

    printf("Server in ascolto sulla porta %d...\n", PORT);

    while (1)
    {
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);

        if (client_sock < 0)
        {
            perror("accept");
            continue;
        }

        printf("Nuovo client connesso: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        Vertex *client = createVertex("TEMP");
        client->socket_fd = client_sock;
        client->ip_addr = client_addr.sin_addr;

        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->client = client;
        args->graph = g;

        pthread_t tid;
        int rc = pthread_create(&tid, NULL, client_handler, args);
        if (rc != 0)
            perror("pthread_create");
        else
            printf("Thread creato per il client.\n");

        pthread_detach(tid);
    }

    close(server_sock);
    return 0;
}
