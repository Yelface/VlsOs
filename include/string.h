#ifndef STRING_H
#define STRING_H

#include "types.h"

/* String functions */
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
char* strchr(const char* s, int c);

/* Character functions */
int isalpha(int c);
int isdigit(int c);
int isalnum(int c);
int isspace(int c);
int isupper(int c);
int islower(int c);
int toupper(int c);
int tolower(int c);

/* Number conversion */
int atoi(const char* str);
char* itoa(int value, char* str, int base);

#endif
