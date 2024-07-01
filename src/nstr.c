#include "nstr.h"

void capitalize(char *str)
{
    for (int i = 0; i < strlen(str); i++)
        str[i] = toupper(str[i]);
}

void trim(char *str, size_t *len)
{
    for (int i = *len-1; isspace(str[i]); (*len)--, i--);
    str[*len] = '\0';
}
