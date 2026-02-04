#pragma once

// ========================
// Базовые типы ядра
// ========================
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  u64;

typedef char           s8;
typedef short          s16;
typedef int            s32;
typedef long           s64;

// ========================
// Булев тип
// ========================
typedef u8 bool;
#define true  1
#define false 0

// ========================
// Константы
// ========================
#define NULL ((void*)0)
#define MAX_U8  0xFF
#define MAX_U16 0xFFFF
#define MAX_U32 0xFFFFFFFF
#define MAX_U64 0xFFFFFFFFFFFFFFFF

// ========================
// Минимальные макросы
// ========================
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ABS(x)   ((x) < 0 ? -(x) : (x))

// ========================
// Простейшие структуры
// ========================
typedef struct {
    u32 x, y;
} point_t;

typedef struct {
    u32 width;
    u32 height;
} rect_t;

typedef struct {
    void *start;
    void *end;
} mem_range_t;

typedef struct {
    u64 flags;
    u64 id;
} thread_t;

typedef struct {
    u64 count;
    u64 limit;
} counter_t;

// ========================
// Заглушки для будущих типов
// ========================
typedef struct {
    u64 dummy[4];
} placeholder_t;

typedef struct {
    u8 data[16];
} uuid_t;

// ========================
// Статические проверки
// ========================
static inline bool is_null(void *p) { return p == NULL; }
static inline bool is_even(u64 x) { return x % 2 == 0; }
static inline bool is_odd(u64 x) { return x % 2 != 0; }

// ========================
// Функции-шаблоны для тестов
// ========================
static inline void nop_void(void) {}
static inline u64 nop_u64(u64 x) { return x; }

// ========================
// Дополнительные типы для расширения
// ========================
typedef struct {
    u32 a,b,c,d,e,f,g,h;
} wide_t;

typedef struct {
    u8 a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p;
} byte16_t;

typedef struct {
    u64 a,b,c,d,e,f,g,h;
} quad64_t;

typedef struct {
    void *ptr;
    u64 size;
} buf_t;

typedef struct {
    u64 flags;
    void *data;
} object_t;

// ========================
// Конец types.h
// ========================
// Всего строк: ~100
