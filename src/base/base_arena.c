static Arena arena_init(usize initial_size_bytes) {
    // TODO(felix): switch to reserve+commit with (virtually) no cap: reserve something like 64gb and commit pages as needed
    #if OS_WINDOWS
        Arena arena = { .mem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, initial_size_bytes) };
    #elif OS_LINUX || OS_MACOS || OS_EMSCRIPTEN
        Arena arena = { .mem = calloc(initial_size_bytes, 1) };
    #else
        #error "unsupported OS"
    #endif

    if (arena.mem == 0) panic("allocation failure");
    arena.cap = initial_size_bytes;
    asan_poison_memory_region(arena.mem, arena.cap);
    return arena;
}

static void *arena_make(Arena *arena, usize item_count, usize item_size) {
    assert(item_size > 0);
    usize byte_count = item_count * item_size;
    if (byte_count == 0) return 0;

    // TODO(felix): asan_poison alignment bytes
    usize alignment = 2 * sizeof(void *);
    usize modulo = arena->offset & (alignment - 1);
    if (modulo != 0) arena->offset += alignment - modulo;

    if (arena->offset + byte_count > arena->cap) panic("allocation failure");

    void *mem = (u8 *)arena->mem + arena->offset;
    arena->last_offset = arena->offset;
    arena->offset += byte_count;
    asan_unpoison_memory_region(mem, byte_count);
    return mem;
}

static void arena_deinit(Arena *arena) {
    asan_poison_memory_region(arena->mem, arena->cap);

    #if OS_WINDOWS
        HeapFree(GetProcessHeap(), 0, arena->mem);
    #elif OS_LINUX || OS_MACOS || OS_EMSCRIPTEN
        free(arena->mem);
    #else
        #error "unsupported OS"
    #endif

    arena->mem = 0;
    arena->offset = 0;
    arena->cap = 0;
}

static String arena_push(Arena *arena, String bytes) {
    u8 *bytes_on_arena = arena_make(arena, bytes.count, sizeof(*bytes.data));
    memcpy(bytes_on_arena, bytes.data, bytes.count);
    String result = { .data = bytes_on_arena, .count = bytes.count };
    return result;
}

static Arena_Temp arena_temp_begin(Arena *arena) {
    return (Arena_Temp){
        .arena = arena,
        .offset = arena->offset,
        .last_offset = arena->last_offset,
    };
}

static void arena_temp_end(Arena_Temp arena_temp) {
    asan_poison_memory_region((u8 *)arena_temp.arena->mem + arena_temp.offset, arena_temp.arena->cap - arena_temp.offset);
    arena_temp.arena->offset = arena_temp.offset;
    arena_temp.arena->last_offset = arena_temp.last_offset;
}
