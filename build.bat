@echo off
setlocal enabledelayedexpansion

echo =========================================================
echo                  UPWISE BUILD SYSTEM
echo              Learn. Practise. Progress.
echo         Pure C Desktop Application with raylib
echo =========================================================

set CC=gcc
set CFLAGS=-Wall -Wextra -std=c99 -O2
set INCLUDES=-Iraylib/include
set LDFLAGS=-Lraylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm

set SRC=src/main.c src/ui.c src/quiz.c src/student.c src/progress.c src/leaderboard.c src/data_structures.c src/compat.c
set OUT=Upwise.exe

if "%1"=="clean" goto clean
if "%1"=="test" goto test
if "%1"=="run" goto run
goto build

:build
echo [*] Compiling Upwise desktop application (100%% pure C + raylib)...
if not exist "data" mkdir data
%CC% %CFLAGS% %SRC% %INCLUDES% %LDFLAGS% -o %OUT%
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed! Check compiler output above.
    exit /b %ERRORLEVEL%
)
echo [SUCCESS] Built %OUT% successfully!

echo [*] Compiling unit tests...
%CC% %CFLAGS% tests/test_ds.c src/data_structures.c src/student.c src/quiz.c src/progress.c src/leaderboard.c -o tests/test_ds.exe
if %ERRORLEVEL% equ 0 (
    echo [SUCCESS] Built tests/test_ds.exe successfully!
)
goto end

:test
echo [*] Building and executing automated data structures test suite...
%CC% %CFLAGS% tests/test_ds.c src/data_structures.c src/student.c src/quiz.c src/progress.c src/leaderboard.c -o tests/test_ds.exe
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Test compilation failed!
    exit /b %ERRORLEVEL%
)
tests\test_ds.exe

echo [*] Building and executing persistence integration tests...
%CC% %CFLAGS% tests/test_persistence.c src/data_structures.c src/student.c src/quiz.c src/progress.c src/leaderboard.c -o tests/test_persistence.exe
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Persistence test compilation failed!
    exit /b %ERRORLEVEL%
)
tests\test_persistence.exe
goto end

:run
call :build
echo [*] Launching Upwise desktop application...
start "" %OUT%
goto end

:clean
echo [*] Cleaning generated binaries...
if exist %OUT% del %OUT%
if exist tests\test_ds.exe del tests\test_ds.exe
if exist tests\test_persistence.exe del tests\test_persistence.exe
echo [SUCCESS] Clean completed.
goto end

:end
echo =========================================================
