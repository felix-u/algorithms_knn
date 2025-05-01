// Based on information from https://sourceforge.net/p/predef/wiki/Home/

#ifndef BASE_CONTEXT_CUSTOM


#if defined(__EMSCRIPTEN__)
    #define OS_EMSCRIPTEN 1
#elif defined(__linux__) || defined(__gnu_linux__)
    #define OS_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
    #define OS_MACOS 1
#elif defined(_WIN32)
    #define OS_WINDOWS 1
#else
    #error unsupported OS
#endif // OS_...

#ifndef OS_EMSCRIPTEN
    #define OS_EMSCRIPTEN 0
#endif
#ifndef OS_LINUX
    #define OS_LINUX 0
#endif
#ifndef OS_MACOS
    #define OS_MACOS 0
#endif
#ifndef OS_WINDOWS
    #define OS_WINDOWS 0
#endif


#if defined(__clang__)
    #define COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define COMPILER_GCC 1
#elif defined(_MSC_VER)
    #define COMPILER_MSVC 1
#else
    #define COMPILER_STANDARD 1
#endif // COMPILER_...

#ifndef COMPILER_CLANG
    #define COMPILER_CLANG 0
#endif
#ifndef COMPILER_GCC
    #define COMPILER_GCC 0
#endif
#ifndef COMPILER_MSVC
    #define COMPILER_MSVC 0
#endif


#if COMPILER_CLANG
    #if __has_feature(address_sanitizer)
        #define BUILD_ASAN 1
    #endif
#elif COMPILER_GCC || COMPILER_MSVC
    #ifdef __SANITIZE_ADDRESS__
        #define BUILD_ASAN 1
    #endif
#endif // COMPILER_...

#ifndef BUILD_ASAN
    #define BUILD_ASAN 0
#endif
#ifndef BUILD_DEBUG
    #define BUILD_DEBUG 0
#endif


#if !defined(BASE_GRAPHICS)
    #define BASE_GRAPHICS 1
#endif


#endif // !BASE_CONTEXT_CUSTOM
