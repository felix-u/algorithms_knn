static usize int_from_string_base(String s, usize base) {
    usize result = 0, magnitude = s.count;
    for (usize i = 0; i < s.count; i += 1, magnitude -= 1) {
        result *= base;
        usize digit = decimal_from_hex_digit_table[s.data[i]];
        if (digit >= base) {
            log_error("digit '%' is invalid in base %", fmt(char, s.data[i]), fmt(usize, base));
            return 0;
        }
        result += digit;
    }
    return result;
}

static bool string_equal(String s1, String s2) {
    if (s1.count != s2.count) return false;
    return memcmp(s1.data, s2.data, s1.count) == 0;
}

static String string_from_cstring(char *s) {
    if (s == NULL) return (String){ 0 };
    usize count = 0;
    while (s[count] != '\0') count += 1;
    return (String){ .data = (u8 *)s, .count = count };
}

static char *cstring_from_string(Arena *arena, String s) {
    char *cstring = arena_make(arena, s.count + 1, sizeof(char));
    for (usize i = 0; i < s.count; i += 1) cstring[i] = (char)s.data[i];
    cstring[s.count] = '\0';
    return cstring;
}

// Only bases <= 10
static String string_from_int_base(Arena *arena, usize _num, u8 base) {
    String str = {0};
    usize num = _num;

    do {
        num /= base;
        str.count += 1;
    } while (num > 0);

    str.data = arena_make(arena, str.count, sizeof(u8));

    num = _num;
    for (isize i = (isize)str.count - 1; i >= 0; i -= 1) {
        str.data[i] = (u8)((num % base) + '0');
        num /= base;
    }

    return str;
}

static String string_print(Arena *arena, char *format, ...) {
    va_list args;
    va_start(args, format);
    String result = string_vprint(arena, format, args);
    va_end(args);
    return result;
}

static String string_range(String string, usize start, usize end) {
    assert(end <= string.count);
    assert(start <= end);
    String result = { .data = string.data + start, .count = end - start };
    return result;
}

static String string_vprint(Arena *arena, char *fmt, va_list args) {
    String_Builder builder = { .arena = arena };
    string_builder_print_var_args(&builder, fmt, args);
    return bit_cast(String) builder;
}

static String16 string16_from_string(Arena *arena, String s) {
    #if OS_WINDOWS
        String16 wstr = {
            .data = arena_make(arena, s.count, sizeof(u16)),
            .count = (usize)MultiByteToWideChar(CP_UTF8, 0, (char *)s.data, (int)s.count, wstr.data, (int)s.count),
        };
        win32_ensure_not_0(wstr.count);
        return wstr;
    #else
        discard(arena);
        discard(s);
        panic("unimplemented for this OS");
    #endif
}

static void string_builder_print(String_Builder *builder, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    string_builder_print_var_args(builder, fmt, args);
    va_end(args);
}

static void string_builder_print_var_args(String_Builder *builder, char *fmt_c, va_list args) {
    String fmt_str = string_from_cstring(fmt_c);
    assert(fmt_str.count > 0);

    for (usize i = 0; i < fmt_str.count; i += 1) {
        usize beg_i = i;
        while (fmt_str.data[i] != '%' && i < fmt_str.count) i += 1;

        if (i == fmt_str.count) {
            string_builder_push(builder, String, string_range(fmt_str, beg_i, i));
            return;
        }

        assert(fmt_str.data[i] == '%');

        if (i > beg_i) string_builder_push(builder, String, string_range(fmt_str, beg_i, i));

        Format format = va_arg(args, Format);
        string_builder_push_format(builder, format);
    }
}

static void string_builder_push_format(String_Builder *builder, Format format) {
    #if BUILD_DEBUG
    {
        // This isn't *guaranteed* to trigger on a non-Format argument in print() because we may segfault when attempting to pop a Format value from variadic arguments.
        // The point is that we'll crash either way.
        bool format_argument_is_correctly_formed = format.magic_value_for_debug == format_magic_value_for_debug;
        assert(format_argument_is_correctly_formed);
    }
    #endif
    switch (format.type) {
        case format_type_bool: {
            // TODO(felix): support uppercase
            String string = format.value_bool ? string("true") : string("false");
            string_builder_push(builder, String, string);
        } break;
        case format_type_char: array_push(builder, &format.value_char); break;
        case format_type_i8:  format.type = format_type_isize; format.value_isize = (isize)format.value_i8; string_builder_push_format(builder, format); break;
        case format_type_i16: format.type = format_type_isize; format.value_isize = (isize)format.value_i16; string_builder_push_format(builder, format); break;
        case format_type_i32: format.type = format_type_isize; format.value_isize = (isize)format.value_i32; string_builder_push_format(builder, format); break;
        case format_type_i64: { static_assert(sizeof(i64) == sizeof(isize), "this case assumes i64 == isize"); } // fallthrough
        case format_type_isize: {
            isize value = format.value_isize;
            if (value < 0) {
                string_builder_push(builder, char, '-');
                value = -value;
            }
            format.type = format_type_usize;
            format.value_usize = (usize)value;
            string_builder_push_format(builder, format);
        } break;
        case format_type_u8:  format.type = format_type_usize; format.value_usize = (usize)format.value_u8; string_builder_push_format(builder, format); break;
        case format_type_u16: format.type = format_type_usize; format.value_usize = (usize)format.value_u16; string_builder_push_format(builder, format); break;
        case format_type_u32: format.type = format_type_usize; format.value_usize = (usize)format.value_u32; string_builder_push_format(builder, format); break;
        case format_type_u64: { static_assert(sizeof(u64) == sizeof(usize), "this case assumes u64 == usize"); } // fallthrough
        case format_type_usize: {
            usize value = format.value_usize;
            if (value == 0) {
                string_builder_push(builder, char, '0');
                break;
            }

            u8 buf_mem[sizeof(usize) * 8];
            usize buf_index = sizeof(buf_mem);

            u8 base = format.base == 0 ? 10 : format.base;
            assert(base <= 16);

            // TODO(felix): support capitalised
            char *character_from_digit = "0123456789abcdef";

            for (; value > 0; value /= base) {
                buf_index -= 1;
                u8 digit = (u8)(value % base);
                buf_mem[buf_index] = (u8)character_from_digit[digit];
            }

            String decimal = { .data = buf_mem + buf_index, .count = sizeof(buf_mem) - buf_index };
            string_builder_push(builder, String, decimal);
        } break;
        case format_type_f32: string_builder_push(builder, f64, (f64)format.value_f32); break;
        case format_type_f64: {
            // References:
            // https://github.com/nothings/stb/blob/master/stb_sprintf.h
            // https://en.wikipedia.org/wiki/Double-precision_floating-point_format
            // https://dl.acm.org/doi/pdf/10.1145/3360595
            //
            // Most valuable:
            // https://github.com/charlesnicholson/nanoprintf
            // http://0x80.pl/notesen/2015-12-29-float-to-string.html
            // TODO(felix): implement this properly! ^

            f64 value = format.value_f64;

            u64 bits = bit_cast(u64) value;
            i64 biased_exponent = (bits >> 52) & 0x7ff;
            u64 mantissa_mask = UINT64_MAX >> 12;
            u64 mantissa = bits & mantissa_mask;

            u8 is_negative = (u8)(bits >> 63);
            if (is_negative) string_builder_push(builder, char, '-');

            if (biased_exponent == 0x7ff) {
                String string = (mantissa == 0) ? string("Infinity") : string("NaN");
                string_builder_push(builder, String, string);
                break;
            }

            if (biased_exponent == 0 && mantissa == 0) {
                string_builder_push(builder, char, '0');
                break;
            }

            // NOTE(felix): this can only represent integer parts up to UINT64_MAX!
            f64 absolute_value = is_negative ? -value : value;
            if (absolute_value > (f64)UINT64_MAX) {
                string_builder_push(builder, String, string("(...)"));
                break;
            }

            string_builder_push(builder, usize, (usize)absolute_value);

            string_builder_push(builder, char, '.');

            f64 fraction = absolute_value;

            // TODO(felix): add ability to configure in format argument
            usize precision = 2;

            for (usize i = 0; i < precision; i += 1) {
                fraction -= (f64)(usize)fraction;
                fraction *= 10.0;
                u8 fraction_as_char = (u8)fraction + '0';
                string_builder_push(builder, char, fraction_as_char);
            }
        } break;
        case format_type_V2: {
            V2 value = format.value_V2;
            string_builder_push(builder, String, string("{ x = "));
            string_builder_push(builder, f32, value.x);
            string_builder_push(builder, String, string(", y = "));
            string_builder_push(builder, f32, value.y);
            string_builder_push(builder, String, string(" }"));
        } break;
        case format_type_cstring: string_builder_push(builder, String, string_from_cstring(format.value_cstring)); break;
        case format_type_String: array_push_slice(builder, &format.value_String); break;
        default: unreachable;
    }
}

static inline void string_builder_null_terminate(String_Builder *builder) {
    // Avoid push(0) because we don't want to increase the string length
    array_ensure_capacity(builder, builder->count + 1);
    builder->data[builder->count] = 0;
}
