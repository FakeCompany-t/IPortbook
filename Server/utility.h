#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>
#include <string.h>

// ==========================
// Conversione MDP (2 byte)
// ==========================

// Converte 2 byte (little endian) in uint16_t
uint16_t converti_mdp(unsigned char *p);

// Converte uint16_t in 2 byte (allocati dinamicamente)
// ATTENZIONE: la memoria va liberata con free()
unsigned char* spacchetta_mdp(uint16_t mdp);


// ==========================
// Conversione UDP (porta 4 cifre)
// ==========================

// Converte 4 byte ASCII in int
int converti_udp(char *port_ptr);

// Converte int in stringa di 4 cifre (allocata dinamicamente)
// ATTENZIONE: la memoria va liberata con free()
char* spacchetta_udp(int port);

#endif
