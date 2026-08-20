#ifndef CHECK_FIELD_H
#define CHECK_FIELD_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Definizioni delle costanti
#define DIM_ID 8
#define DIM_PORT 4
#define DIM_MDP 2
#define DIM_MESS 200
#define DIM_NUM_ITEM 3

// Dichiarazioni delle funzioni
bool check_id(char *id);
bool check_port(char *port);
// bool check_mdp(char *mdp); // Da implementare
bool check_mess(char *mess);
bool num_item(char *num_item);

#endif // CHECK_FIELD_H
