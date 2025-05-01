structdef(Arena) {
    void *mem;
    usize offset;
    usize cap;
    usize last_offset;
};

structdef(Arena_Temp) {
    Arena *arena;
    usize offset;
    usize last_offset;
};

static void *arena_make(Arena *arena, usize item_count, usize item_size);
static void  arena_deinit(Arena *arena);

// TODO(felix): remove this function. A zeroed reserve+commit arena will be valid and will grow on demand
static Arena arena_init(usize initial_size_bytes);

static String arena_push(Arena *arena, String bytes);

static Arena_Temp arena_temp_begin(Arena *arena);
static void       arena_temp_end(Arena_Temp arena_temp);

#if BUILD_ASAN
    #define asan_poison_memory_region(address, byte_count)   __asan_poison_memory_region(address, byte_count)
    #define asan_unpoison_memory_region(address, byte_count) __asan_unpoison_memory_region(address, byte_count)
    #include <sanitizer/asan_interface.h>
#else
    #define asan_poison_memory_region(address, byte_count)   statement_macro( discard(address); discard(byte_count); )
    #define asan_unpoison_memory_region(address, byte_count) statement_macro( discard(address); discard(byte_count); )
#endif // BUILD_ASAN
