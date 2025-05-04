@echo off

setlocal
set name=knn

for %%a in (%*) do set "%%a=1"
if not "%msvc%"=="1" if not "%clang%"=="1" set msvc=1
if not "%release%"=="1"                    set debug=1

set dir_deps=..\deps
set include_paths=-I%dir_deps%

set cl_common=cl -nologo -FC -diagnostics:column -std:c11 -MT %include_paths%
set clang_common=clang -pedantic -Wno-microsoft -std=c11 -MT %include_paths%
set cl_link=-link -entry:entrypoint -subsystem:console -incremental:no kernel32.lib shell32.lib ucrt.lib
set clang_link=-lkernel32 -lshell32 -lucrt -Xlinker -entry:entrypoint -Xlinker -subsystem:console
set cl_debug=%cl_common% -W4 -WX -Z7 -DBUILD_DEBUG=1
set clang_debug=%clang_common% ^
    -Wall -Werror -Wextra -Wshadow -Wconversion -Wdouble-promotion ^
    -Wno-unused-function -Wno-deprecated-declarations -Wno-unused-local-typedef -fno-strict-aliasing ^
    -g3 -fsanitize=undefined -fsanitize-trap -DBUILD_DEBUG=1
set cl_out=-out:
set clang_out=-o

set cl_release=%cl_common% /O2
set clang_release=%clang_common% -O3

if "%msvc%"=="1"  set compile_debug=%cl_debug%
if "%msvc%"=="1"  set compile_release=%cl_release%
if "%msvc%"=="1"  set compile_link=%cl_link%
if "%msvc%"=="1"  set compile_out=%cl_out%
if "%clang%"=="1" set compile_debug=%clang_debug%
if "%clang%"=="1" set compile_release=%clang_release%
if "%clang%"=="1" set compile_link=%clang_link%
if "%clang%"=="1" set compile_out=%clang_out%

if "%debug%"=="1" set compile=%compile_debug%
if "%release%"=="1" set compile=%compile_release%

if not exist build mkdir build
pushd build

if exist %name%.pdb del %name%.pdb
%compile% ..\src\main.c %compile_link% %compile_out%%name%.exe

popd
