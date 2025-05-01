static String file_read_bytes_relative_path(Arena *arena, char *path, usize max_bytes);
static bool file_write_bytes_to_relative_path(char *path, String bytes);

#define log_info(...) log_internal("info: " __VA_ARGS__)
#define log_internal(...) log_internal_with_location(__FILE__, __LINE__, (char *)__func__, __VA_ARGS__)
static void log_internal_with_location(char *file, usize line, char *func, char *format, ...);

static void os_write(String bytes);
static void print(char *format, ...);
static void print_var_args(char *format, va_list args);
