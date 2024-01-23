#ifndef NSTR_H
#define NSTR_H

#include <ctype.h>
#include <string.h>

//converts every char of str to uppercase
void capitalize(char *str);

// removes any ascii whitespace chars ('\n', '\r', ' ', '\f', '\t', '\v')  at the end of <<str>>, updates the <<len>> 
void trim(char *str, size_t *len);

#endif
