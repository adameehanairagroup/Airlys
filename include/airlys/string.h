#ifndef AIRLYS_STRING_H
#define AIRLYS_STRING_H

#include <airlys/types.h>

size_t kstrlen(const char* s);
int kstrcmp(const char* s1, const char* s2);
int kstrncmp(const char* s1, const char* s2, size_t n);
void kstrncpy(char* dest, const char* src, size_t n);
void* kmemcpy(void* dest, const void* src, size_t n);
void* kmemset(void* s, int c, size_t n);

#endif /* AIRLYS_STRING_H */
