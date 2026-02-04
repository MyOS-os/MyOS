#include <kernel/lib/string.h>

// ========================
// Функции работы с памятью
// ========================

void *memset(void *s, int c, u64 n) {
    unsigned char *p = s;
    for (u64 i = 0; i < n; i++)
        p[i] = (unsigned char)c;
    return s;
}

void *memcpy(void *dest, const void *src, u64 n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    for (u64 i = 0; i < n; i++)
        d[i] = s[i];
    return dest;
}

void *memmove(void *dest, const void *src, u64 n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    if (d < s) {
        for (u64 i = 0; i < n; i++) d[i] = s[i];
    } else {
        for (u64 i = n; i > 0; i--) d[i-1] = s[i-1];
    }
    return dest;
}

int memcmp(const void *a, const void *b, u64 n) {
    const unsigned char *pa = a;
    const unsigned char *pb = b;
    for (u64 i = 0; i < n; i++)
        if (pa[i] != pb[i]) return pa[i] - pb[i];
    return 0;
}

void *memchr(const void *s, int c, u64 n) {
    const unsigned char *p = s;
    for (u64 i = 0; i < n; i++)
        if (p[i] == (unsigned char)c) return (void*)(p + i);
    return 0;
}

// ========================
// Функции работы со строками
// ========================

u64 strlen(const char *s) {
    u64 len = 0;
    while (s[len]) len++;
    return len;
}

char *strcpy(char *dest, const char *src) {
    u64 i = 0;
    while ((dest[i] = src[i])) i++;
    return dest;
}

char *strncpy(char *dest, const char *src, u64 n) {
    for (u64 i = 0; i < n; i++) {
        if (src[i] == 0) { dest[i] = 0; break; }
        dest[i] = src[i];
    }
    return dest;
}

int strcmp(const char *a, const char *b) {
    u64 i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return (int)(a[i] - b[i]);
}

int strncmp(const char *a, const char *b, u64 n) {
    for (u64 i = 0; i < n; i++) {
        if (a[i] != b[i] || a[i] == 0 || b[i] == 0) return (int)(a[i] - b[i]);
    }
    return 0;
}

char *strcat(char *dest, const char *src) {
    u64 dlen = strlen(dest);
    u64 i = 0;
    while (src[i]) { dest[dlen + i] = src[i]; i++; }
    dest[dlen + i] = 0;
    return dest;
}

char *strncat(char *dest, const char *src, u64 n) {
    u64 dlen = strlen(dest);
    u64 i = 0;
    while (src[i] && i < n) { dest[dlen + i] = src[i]; i++; }
    dest[dlen + i] = 0;
    return dest;
}

// ========================
// Дополнительные утилиты
// ========================

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return 0;
}

char *strrchr(const char *s, int c) {
    const char *last = 0;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    return (char*)last;
}

unsigned long strspn(const char *s, const char *accept) {
    u64 count = 0;
    while (*s && strchr(accept, *s)) { count++; s++; }
    return count;
}

unsigned long strcspn(const char *s, const char *reject) {
    u64 count = 0;
    while (*s && !strchr(reject, *s)) { count++; s++; }
    return count;
}

char *strpbrk(const char *s, const char *accept) {
    while (*s) {
        if (strchr(accept, *s)) return (char*)s;
        s++;
    }
    return 0;
}

char *strstr(const char *haystack, const char *needle) {
    u64 nlen = strlen(needle);
    if (!nlen) return (char*)haystack;
    for (u64 i = 0; haystack[i]; i++) {
        if (strncmp(&haystack[i], needle, nlen) == 0) return (char*)&haystack[i];
    }
    return 0;
}

// ========================
// Конец string.c
// ========================
// Всего строк: ~100
