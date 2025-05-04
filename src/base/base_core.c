// TODO(felix): add slice_equal/slice_compare

static void array_ensure_capacity_explicit_item_size(Array_void *array, usize item_count, usize item_size, bool non_zero) {
    if (array->capacity >= item_count) return;

    // TODO(felix): should this be a power of 2?
    usize new_capacity = clamp_low(1, array->capacity * 2);
    while (new_capacity < item_count) new_capacity *= 2;

    u8 *new_memory = arena_make(array->arena, new_capacity, item_size);
    if (array->count > 0) memcpy(new_memory, array->data, array->count * item_size);

    if (!non_zero) {
        usize old_capacity = array->capacity;
        usize growth_byte_count = item_size * (new_capacity - old_capacity);
        u8 *beginning_of_new_memory = new_memory + (item_size * old_capacity);
        memset(beginning_of_new_memory, 0, growth_byte_count);
    }

    array->data = new_memory;
    array->capacity = new_capacity;
}

static inline void array_push_explicit_item_size(Array_void *array, void *item, usize item_size) {
    Slice_void slice = { .data = item, .count = 1 };
    array_push_slice_explicit_item_size(array, &slice, item_size);
}

static inline void array_push_explicit_item_size_assume_capacity(Array_void *array, void *item, usize item_size) {
    usize new_len = array->count + 1;
    assert(new_len <= array->capacity);
    // TODO(felix): should this be memmove? if not, add assert to check for overlap
    memcpy((u8 *)array->data + (array->count * item_size), item, item_size);
    array->count = new_len;
}

static void array_push_slice_explicit_item_size(Array_void *array, Slice_void *slice, usize item_size) {
    assert(array->arena != 0);
    usize new_len = array->count + slice->count;
    array_ensure_capacity_explicit_item_size(array, new_len, item_size, false);
    // TODO(felix): should this be memmove? if not, add assert to check for overlap
    memcpy((u8 *)array->data + (array->count * item_size), slice->data, slice->count * item_size);
    array->count = new_len;
}

static void array_push_slice_explicit_item_size_assume_capacity(Array_void *array, Slice_void *slice, usize item_size) {
    usize new_len = array->count + slice->count;
    assert(new_len <= array->capacity);
    // TODO(felix): should this be memmove? if not, add assert to check for overlap
    memcpy((u8 *)array->data + (array->count * item_size), slice->data, slice->count * item_size);
    array->count = new_len;
}

// TODO(felix): rename/replace (see base_core.h)
static inline int memcmp_(void *a_, void *b_, usize byte_count) {
    u8 *a = a_, *b = b_;
    assert(a != 0);
    assert(b != 0);
    for (usize i = 0; i < byte_count; i += 1) {
        if (a[i] != b[i]) return a[i] - b[i];
    }
    return 0;
}

void *memcpy(void *destination_, const void *source_, usize byte_count) {
    u8 *destination = destination_;
    const u8 *source = source_;
    if (byte_count != 0) {
        assert(destination != 0);
        assert(source != 0);
    }
    for (usize i = 0; i < byte_count; i += 1) destination[i] = source[i];
    return destination;
}

extern void *memset(void *destination_, int byte_, usize byte_count) {
    assert(byte_ < 256);
    u8 byte = (u8)byte_;
    u8 *destination = destination_;
    assert(destination != 0);
    for (usize i = 0; i < byte_count; i += 1) destination[i] = byte;
    return destination;
}

// TODO(felix): replace with something that feels less hacky
#if OS_LINUX || OS_MACOS
    static int argument_count_;
    static char **arguments_;

    static void entrypoint(void);
    int main(int argument_count, char **arguments) {
        argument_count_ = argument_count;
        arguments_ = arguments;
        entrypoint();
        return 0;
    }
#endif

static Slice_String os_get_arguments(Arena *arena) {
    Array_String arguments = { .arena = arena };

    #if OS_WINDOWS
        int argument_count = 0;
        u16 **arguments_utf16 = CommandLineToArgvW(GetCommandLineW(), &argument_count);
        if (arguments_utf16 == 0) {
            log_error("unable to get command line arguments");
            return (Slice_String){0};
        }

        array_ensure_capacity(&arguments, (usize)argument_count);

        for (usize i = 0; i < (usize)argument_count; i += 1) {
            u16 *argument_utf16 = arguments_utf16[i];

            usize length = 0;
            while (argument_utf16[length] != 0) length += 1;

            // TODO(felix): convert to UTF-8, not ascii
            Array_u8 argument = { .arena = arena };
            array_ensure_capacity(&argument, length);
            for (usize j = 0; j < length; j += 1) {
                u16 wide_character = argument_utf16[j];
                assert(wide_character < 128);
                u8 character = (u8)wide_character;
                array_push_assume_capacity(&argument, &character);
            }
            array_push_assume_capacity(&arguments, (String *)&argument);
        }
    #elif OS_LINUX || OS_MACOS
        array_ensure_capacity(&arguments, argument_count__);
        for (usize i = 0; i < (usize)argument_count__; i += 1) {
            String argument = string_from_cstring(arguments__[i]);
            array_push_assume_capacity(&arguments, &argument);
        }
    #endif

    return bit_cast(Slice_String) arguments;
}
