#include <kernel/lib/memory.h>
#include <kernel/lib/string.h>

#define HEAP_FALLBACK_SIZE (1024 * 1024)
#define HEAP_ALIGN 16

typedef struct heap_block {
    u64 size;
    bool free;
    struct heap_block *next;
} heap_block_t;

static u8 fallback_heap[HEAP_FALLBACK_SIZE];
static void *heap_base = fallback_heap;
static u64 heap_size = HEAP_FALLBACK_SIZE;
static heap_block_t *heap_head = NULL;

static inline u64 align_up(u64 value, u64 align) {
    return (value + align - 1) & ~(align - 1);
}

void memory_init(void *base, u64 size) {
    if (base && size >= sizeof(heap_block_t) + HEAP_ALIGN) {
        heap_base = base;
        heap_size = size;
    }

    heap_head = (heap_block_t *)heap_base;
    heap_head->size = heap_size - sizeof(heap_block_t);
    heap_head->free = true;
    heap_head->next = NULL;
}

static heap_block_t *find_free_block(u64 size) {
    heap_block_t *current = heap_head;
    while (current) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void split_block(heap_block_t *block, u64 size) {
    u64 aligned = align_up(size, HEAP_ALIGN);
    if (block->size <= aligned + sizeof(heap_block_t) + HEAP_ALIGN) {
        return;
    }

    u8 *block_start = (u8 *)block;
    heap_block_t *new_block = (heap_block_t *)(block_start + sizeof(heap_block_t) + aligned);
    new_block->size = block->size - aligned - sizeof(heap_block_t);
    new_block->free = true;
    new_block->next = block->next;

    block->size = aligned;
    block->next = new_block;
}

void *kmalloc(u64 size) {
    if (size == 0) {
        return NULL;
    }

    if (!heap_head) {
        memory_init(heap_base, heap_size);
    }

    u64 aligned = align_up(size, HEAP_ALIGN);
    heap_block_t *block = find_free_block(aligned);
    if (!block) {
        return NULL;
    }

    split_block(block, aligned);
    block->free = false;

    return (u8 *)block + sizeof(heap_block_t);
}

void *kcalloc(u64 count, u64 size) {
    if (count == 0 || size == 0) {
        return NULL;
    }

    u64 total = count * size;
    if (size != 0 && total / size != count) {
        return NULL;
    }

    void *ptr = kmalloc(total);
    if (ptr) {
        kmemset(ptr, 0, total);
    }
    return ptr;
}

static void merge_adjacent(void) {
    heap_block_t *current = heap_head;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(heap_block_t) + current->next->size;
            current->next = current->next->next;
            continue;
        }
        current = current->next;
    }
}

void kfree(void *ptr) {
    if (!ptr) {
        return;
    }

    heap_block_t *block = (heap_block_t *)((u8 *)ptr - sizeof(heap_block_t));
    block->free = true;
    merge_adjacent();
}

void *krealloc(void *ptr, u64 new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    heap_block_t *block = (heap_block_t *)((u8 *)ptr - sizeof(heap_block_t));
    if (block->size >= new_size) {
        split_block(block, new_size);
        return ptr;
    }

    void *new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return NULL;
    }

    kmemcpy(new_ptr, ptr, block->size);
    kfree(ptr);
    return new_ptr;
}

void kmemset(void *ptr, u8 val, u64 size) {
    memset(ptr, val, size);
}

void kmemcpy(void *dest, const void *src, u64 size) {
    memcpy(dest, src, size);
}
