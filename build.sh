#!/usr/bin/env sh

name="knn"

for arg in "$@"; do
    eval "$arg=1"
done

[ "$gcc" != "1" ] && gcc=1
[ "$release" != "1" ] && debug=1

[ "$gcc" = "1" ] && compiler="gcc"
[ "$clang" = "1" ] && compiler="clang"

compile_common="$compiler -pedantic -std=c11"
compile_link=""
compile_debug="$compile_common -Wall -Werror -Wextra -Wshadow -Wconversion -Wdouble-promotion \
    -Wno-unused-function -fno-strict-aliasing -g3 \
    -fsanitize=address,undefined -fsanitize-trap -DBUILD_DEBUG=1 \
"
compile_release="$compile_common -O3 -s"

[ "$debug" = "1" ] && compile=$compile_debug
[ "$release" = "1" ] && compile=$compile_release

[ ! -d build ] && mkdir build

cd build || exit 1

$compile ../src/main.c $compile_link -o${name}

cd - > /dev/null

