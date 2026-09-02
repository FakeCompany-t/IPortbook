#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint16_t converti_mdp(unsigned char *p) {
    uint16_t mdp_finale;

    // Primo byte (LSB - Least Significant Byte)
    unsigned char low = p[0]; 
    // Secondo byte (MSB - Most Significant Byte)
    unsigned char high = p[1]; 

    // Li uniamo: spostiamo il byte "alto" a sinistra di 8 bit e facciamo l'OR col "basso"
    mdp_finale = (uint16_t)low | ((uint16_t)high << 8);

    return mdp_finale;
}

unsigned char* spacchetta_mdp(uint16_t mdp) {
    // Alloca 2 byte nella memoria "Heap" (quella che non muore a fine funzione)
    unsigned char* output = (unsigned char*) malloc(2 * sizeof(unsigned char));

    if (output == NULL) return NULL;

    // Primo byte (Little Endian)
    output[0] = (unsigned char)(mdp & 0xFF);

    // Secondo byte (Most Significant Byte)
    output[1] = (unsigned char)(mdp >> 8);

    return output;
}


int converti_udp(char *port_ptr) {
    char temp[5]; // 4 cifre + 1 per lo zero finale

    // Copiamo solo i 4 byte della porta
    memcpy(temp, port_ptr, 4);
    temp[4] = '\0'; // Qui diciamo ad atoi dove fermarsi

    return atoi(temp); 
}


char* spacchetta_udp(int port) {
    
    char* s = (char*) malloc(5 * sizeof(char));
    if (s == NULL) {
        perror("Errore nell'allocazione della memoria");
        return NULL;
    }

    // Converte l'intero in stringa
    if (snprintf(s,sizeof(s), "%04d", port) < 0) {
        perror("Errore nella conversione");
        free(s);
        return NULL;
    }

    return s;
}


int areFriends(Vertex *a, Vertex *b)
{
    Edge *e;

    if (a == NULL || b == NULL)
        return 0;

    for (e = a->adj_head; e != NULL; e = e->next)
    {
        if (strncmp(e->id, b->id, 8) == 0)
            return 1;
    }

    return 0;
}
