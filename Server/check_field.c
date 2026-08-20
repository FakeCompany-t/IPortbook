#include "check_field.h"

#define DIM_ID 8
#define DIM_PORT 4
#define DIM_MDP 2
#define DIM_MESS 200
#define DIM_NUM_ITEM 3

bool check_id(char *id)
{
    if (strlen(id) == DIM_ID)
    {
        if (strstr(id, "+++") != NULL)
            return false;
        else
            return true;
    }
    else
        return false;
}

bool check_port(char *port)
{
    if (strlen(port) == DIM_PORT)
    {
        for (int i = 0; i < 4; i++)
        {
            if (!isdigit(port[i]))
                return false;
        }
        return true;
    }
    else
        return false;
}

/*
 bool check_mdp(char *mdp){


 }
*/

bool check_mess(char *mess)
{
    if (strlen(mess) <= DIM_MESS)
    {
        if (strstr(mess, "+++") != NULL)
            return false;
        else
            return true;
    }
    else
        return false;
}

bool num_item(char *num_item)
{
    if (strlen(num_item) == DIM_NUM_ITEM)
    {
        for (int i = 0; i < 4; i++)
        {
            if (!isdigit(num_item[i]))
                return false;
        }
        return true;
    }
    else
        return false;
}
