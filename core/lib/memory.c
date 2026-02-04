#include <kernel/lib/memory.h>
#include <kernel/lib/string.h>

// ========================
// Простейший аллокатор
// ========================

static unsigned char *heap_start = (unsigned char*)0x100000;
static unsigned long heap_offset = 0;
static const unsigned long HEAP_SIZE = 0x100000; // 1 MB

void *kmalloc(unsigned long size) {
    if (heap_offset + size > HEAP_SIZE) {
        return 0; // нет памяти
    }
    void *ptr = heap_start + heap_offset;
    heap_offset += size;
    return ptr;
}

void kfree(void *ptr) {
    (void)ptr; // Пока пусто, позже можно реализовать free
}

// ========================
// Вспомогательные функции
// ========================

void kmemset(void *ptr, unsigned char val, unsigned long size) {
    memset(ptr, val, size);
}

void kmemcpy(void *dest, const void *src, unsigned long size) {
    memcpy(dest, src, size);
}

// ========================
// Простейшие структуры
// ========================

typedef struct {
    void *addr;
    unsigned long size;
    unsigned long flags;
} mem_block_t;

static mem_block_t blocks[16];
static unsigned long blocks_count = 0;

mem_block_t *alloc_block(void *addr, unsigned long size) {
    if (blocks_count >= 16) return 0;
    blocks[blocks_count].addr = addr;
    blocks[blocks_count].size = size;
    blocks[blocks_count].flags = 0;
    return &blocks[blocks_count++];
}

// ========================
// Заглушки для тестов
// ========================

void test_kmalloc(void) {
    char *p1 = kmalloc(32);
    char *p2 = kmalloc(64);
    kmemset(p1, 0xAA, 32);
    kmemcpy(p2, p1, 32);
}

void memory_nop(void) {}
unsigned long memory_nop_u64(unsigned long x) { return x; }

// ========================
// Заполнение памяти шаблонными данными
// ========================

unsigned char memory_fill[64];

void memory_fill_example(void) {
    for (unsigned long i=0; i<64; i++)
        memory_fill[i] = (unsigned char)(i*2);
}

// ========================
// Конец memory.c
// ========================
// Всего строк: ~100
