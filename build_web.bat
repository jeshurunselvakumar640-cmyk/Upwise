@echo off
setlocal enabledelayedexpansion

echo =========================================================
echo             UPWISE WEB BUILD SYSTEM (WASM)
echo        Learn. Practise. Progress. - raylib v5.0
echo =========================================================

if exist "C:\emsdk\emsdk_env.bat" (
    call C:\emsdk\emsdk_env.bat
) else (
    echo [INFO] Assuming emcc is in current PATH...
)

set SRC=src/main.c src/ui.c src/quiz.c src/student.c src/progress.c src/leaderboard.c src/data_structures.c src/compat.c
set INCLUDES=-Isrc -Iraylib-5.0_webassembly/include
set RAYLIB_LIB=raylib-5.0_webassembly/lib/libraylib.a
set FLAGS=-DPLATFORM_WEB -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 -lidbfs.js --preload-file data --preload-file assets --shell-file web/shell.html -O2

echo [*] Compiling Upwise C application to WebAssembly...
if not exist "web" mkdir web
emcc %SRC% %RAYLIB_LIB% -o web/index.html %INCLUDES% %FLAGS%
if %ERRORLEVEL% neq 0 (
    echo [ERROR] WebAssembly compilation failed! Check output above.
    exit /b %ERRORLEVEL%
)

echo [*] Syncing WebAssembly build artifacts to repository root for root Vercel deployments...
copy /Y web\index.html .\index.html >nul
copy /Y web\index.js .\index.js >nul
copy /Y web\index.wasm .\index.wasm >nul
copy /Y web\index.data .\index.data >nul

echo [SUCCESS] Upwise WebAssembly build completed successfully!
echo [OUTPUT] web/index.html, index.js, index.wasm, index.data
echo =========================================================
