# TODO Fancy Terminal Task Manager

A single-file C todo app with a coloured, windowed terminal UI, CSV export, and persistent storage. Runs on Windows with MinGW/GCC.

## Requirements

- Windows 10 or later
- [MinGW-w64](https://www.mingw-w64.org/) with `gcc` on your `PATH`
- `make` (ships with MinGW or install via [Chocolatey](https://chocolatey.org/): `choco install make`)

## Building

### With Make (recommended)

```bash
make
```

This compiles `todo.c` and produces `todo.exe`.

### Manually with GCC

```bash
gcc -Wall -Wextra -O2 -std=c99 -o todo.exe todo.c -lkernel32 -luser32
```

### Other Make targets

| Command | What it does |
|---|---|
| `make run` | Build and immediately launch the app |
| `make clean` | Delete `todo.exe` |

## How to Use

### Launching

```bash
todo.exe
# or via make:
make run
```

The app opens in a resized console window (100×35). Your tasks are loaded automatically from `todos.dat` if it exists.

### Step-by-Step

**1. Navigate the list**

Use the `↑` / `↓` arrow keys to move the highlight up and down. You can also use `J` (down) and `K` (up) like Vim.

**2. Add a task**

Press `A`. A form appears with four fields:

- **Title** the name of your task (required)
- **Tag** a short label like `work`, `personal`, `urgent`
- **Date** in `YYYY-MM-DD` format; today's date is pre-filled
- **Priority** press `Tab` or `Space` while on this field to cycle through `LOW`, `MED`, `HIGH`

Move between fields with `Tab` or the arrow keys. Press `Enter` to confirm and save. Press `Esc` to cancel.

**3. Edit a task**

Select a task and press `E`. The same form opens pre-filled with the task's current values. Edit any field, then press `Enter` to save.

**4. Toggle done / todo**

Select a task and press `Space`. The status flips between `TODO` and `DONE`. Done tasks appear dimmed in the list.

**5. Filter the view**

Press `Tab` to cycle through three view tabs at the top of the screen:

- `ALL` every task
- `ACTIVE` only tasks still marked TODO
- `DONE` only completed tasks

**6. Delete a task**

Select a task and press `D` (or the `Delete` key). A confirmation dialog appears use `←` / `→` to choose Cancel or OK, then press `Enter`.

**7. Export to CSV**

Press `X` at any time. A file called `todos.csv` is written to the same directory as `todo.exe`. It contains all tasks across all views with columns: `Title`, `Tag`, `Date`, `Priority`, `Status`.

**8. Quit**

Press `Q` or `Esc`. A confirmation dialog appears. Confirm to exit your tasks are saved automatically to `todos.dat` on every change, so nothing is lost.

### Keyboard Reference

| Key | Action |
|---|---|
| `↑` / `K` | Move selection up |
| `↓` / `J` | Move selection down |
| `A` | Add new task |
| `E` | Edit selected task |
| `Space` | Toggle done / todo |
| `D` / `Delete` | Delete selected task |
| `X` | Export all tasks to `todos.csv` |
| `Tab` | Cycle view (All → Active → Done) |
| `Q` / `Esc` | Quit |

### Output Files

| File | Description |
|---|---|
| `todos.dat` | Binary save file, created automatically |
| `todos.csv` | CSV export, created when you press `X` |

## Resources

These are the references used to build this project. They are useful if you want to understand or replicate the techniques used.

### Windows Console API

- [Console Screen Buffers Microsoft Docs](https://learn.microsoft.com/en-us/windows/console/console-screen-buffers)
  `SetConsoleCursorPosition`, `SetConsoleTextAttribute`, `GetConsoleScreenBufferInfo`
- [Console Virtual Terminal Sequences](https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences)
  Alternative to Win32 API; ANSI escape codes for colour and cursor control
- [COORD structure](https://learn.microsoft.com/en-us/windows/console/coord-str)
  Used for cursor positioning

### C Standard Library

- [cppreference C Standard Library](https://en.cppreference.com/w/c)
  Complete reference for `stdio.h`, `string.h`, `time.h`, etc.
- [cppreference File I/O](https://en.cppreference.com/w/c/io)
  `fopen`, `fread`, `fwrite`, `fclose` used for persistence and CSV export

### Keyboard Input (conio.h)

- [Virtual-Key Codes Microsoft Docs](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)
  Key scan codes for arrow keys, function keys, Delete, etc.

### CP437 Box-Drawing Characters

- [Code Page 437 Wikipedia](https://en.wikipedia.org/wiki/Code_page_437)
  The original IBM PC character set source of all the `╔═╗`, `║`, `╚╝`, `┌─┐` characters used in the UI
- [Box-drawing characters Wikipedia](https://en.wikipedia.org/wiki/Box-drawing_character)
  Overview of single vs double line box characters and their hex codes

### MinGW / Toolchain

- [MinGW-w64 Downloads](https://www.mingw-w64.org/downloads/)
  Compiler toolchain for building Windows executables with GCC
- [GNU Make Manual](https://www.gnu.org/software/make/manual/make.html)
  Full reference for writing and using Makefiles

## Author

**John Paul Valenzuela**
Copyright © 2026 John Paul Valenzuela. All rights reserved.