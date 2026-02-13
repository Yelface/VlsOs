#include "types.h"

/* Basic C library functions */

/* String functions */
size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len]) len++;
	return len;
}

int strcmp(const char* s1, const char* s2) {
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return (*s1) - (*s2);
}

int strncmp(const char* s1, const char* s2, size_t n) {
	for (size_t i = 0; i < n; i++) {
		if (s1[i] != s2[i]) return s1[i] - s2[i];
		if (!s1[i]) return 0;
	}
	return 0;
}

char* strcpy(char* dest, const char* src) {
	while (*src) {
		*dest++ = *src++;
	}
	*dest = 0;
	return dest;
}

char* strcat(char* dest, const char* src) {
	while (*dest) dest++;
	while (*src) {
		*dest++ = *src++;
	}
	*dest = 0;
	return dest;
}

char* strchr(const char* s, int c) {
	while (*s) {
		if (*s == (char) c) return (char*) s;
		s++;
	}
	return NULL;
}

/* Character functions */
int isalpha(int c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isdigit(int c) {
	return c >= '0' && c <= '9';
}

int isalnum(int c) {
	return isalpha(c) || isdigit(c);
}

int isspace(int c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

int isupper(int c) {
	return c >= 'A' && c <= 'Z';
}

int islower(int c) {
	return c >= 'a' && c <= 'z';
}

int toupper(int c) {
	if (islower(c)) return c - 32;
	return c;
}

int tolower(int c) {
	if (isupper(c)) return c + 32;
	return c;
}

/* Number conversion */
int atoi(const char* str) {
	int result = 0;
	int sign = 1;

	while (isspace(*str)) str++;

	if (*str == '-') {
		sign = -1;
		str++;
	} else if (*str == '+') {
		str++;
	}

	while (isdigit(*str)) {
		result = result * 10 + (*str - '0');
		str++;
	}

	return result * sign;
}

/* Convert integer to string */
char* itoa(int value, char* str, int base) {
	if (base < 2 || base > 36) return str;

	char* ptr = str;
	char* start = str;

	if (value < 0) {
		*ptr++ = '-';
		start++;
		value = -value;
	}

	if (value == 0) {
		*ptr++ = '0';
	} else {
		char buffer[33];
		int len = 0;

		while (value > 0) {
			int digit = value % base;
			buffer[len++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
			value /= base;
		}

		for (int i = len - 1; i >= 0; i--) {
			*ptr++ = buffer[i];
		}
	}

	*ptr = 0;
	return str;
}
