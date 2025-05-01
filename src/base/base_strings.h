structdef(Format) {
    #if BUILD_DEBUG
        u32 magic_value_for_debug;
    #endif
    enum {
        format_type_bool,
        format_type_char,
        format_type_u8, format_type_u16, format_type_u32, format_type_u64, format_type_usize,
        format_type_i8, format_type_i16, format_type_i32, format_type_i64, format_type_isize,
        format_type_f32,
        format_type_f64,
        format_type_V2,
        format_type_cstring,
        format_type_String,
    } type;
    union {
        bool value_bool;
        u8 value_char;
        u8 value_u8; u16 value_u16; u32 value_u32; u64 value_u64; usize value_usize;
        i8 value_i8; i16 value_i16; i32 value_i32; i64 value_i64; isize value_isize;
        f32 value_f32;
        f64 value_f64;
        V2 value_V2;
        char *value_cstring;
        String value_String;
    };
    u8 base; // TODO(felix)
    bool prefix; // TODO(felix)
    bool uppercase; // TODO(felix)
};

#define format_magic_value_for_debug 0xfeffec // why not? this is the background colour of the acme editor from plan9

#if BUILD_DEBUG
    #define fmt(type_, ...) (Format){ .magic_value_for_debug = format_magic_value_for_debug,\
                                      .type = format_type_##type_, .value_##type_ = __VA_ARGS__ }
#else
    #define fmt(type_, ...) (Format){ .type = format_type_##type_, .value_##type_ = __VA_ARGS__ }
#endif

#define cstring_print(arena_ptr, fmt, ...)\
    (char *)(string_print(arena_ptr, fmt "\0", __VA_ARGS__).data)

#define _for_valid_hex_digit(action)\
    action('0', 0x0) action('1', 0x1) action('2', 0x2) action('3', 0x3)\
    action('4', 0x4) action('5', 0x5) action('6', 0x6) action('7', 0x7)\
    action('8', 0x8) action('9', 0x9) action('A', 0xa) action('B', 0xb)\
    action('C', 0xc) action('D', 0xd) action('E', 0xe) action('F', 0xf)\
    action('a', 0xa) action('b', 0xb) action('c', 0xc) action('d', 0xd)\
    action('e', 0xe) action('f', 0xf)

const u8 decimal_from_hex_digit_table[256] = {
    #define _make_hex_digit_value_table(c, val) [c] = val,
    _for_valid_hex_digit(_make_hex_digit_value_table)
};

const bool is_hex_digit[256] = {
    #define _make_hex_digit_truth_table(c, val) [c] = true,
    _for_valid_hex_digit(_make_hex_digit_truth_table)
};

static usize int_from_string_base(String s, usize base);

static char *cstring_from_string(Arena *arena, String s);

static bool   string_equal(String s1, String s2);
static String string_from_cstring(char *s);
static String string_from_int_base(Arena *arena, usize _num, u8 base);
static String string_print(Arena *arena, char *fmt, ...);
static String string_range(String string, usize start, usize end);
static String string_vprint(Arena *arena, char *fmt, va_list args);

static String16 string16_from_string(Arena *arena, String s);

static void string_builder_print(String_Builder *builder, char *fmt, ...);
static void string_builder_print_var_args(String_Builder *builder, char *fmt, va_list args);

#define string_builder_push(builder, type_, ...) string_builder_push_format(builder, fmt(type_, __VA_ARGS__))
static void string_builder_push_format(String_Builder *builder, Format format);
static inline void string_builder_null_terminate(String_Builder *builder);
