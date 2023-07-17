# CHANGELOG

## `[0.1.0]` - 2023-07-14

Main focus of this release is to create a simple running program with blocking
I/O. It includes the implementation and the project structure.

- Initialize the project with stuff:
  - `README.md`
  - `CHANGELOG.md`
  - `LICENSE`
  - `.gitignore`
  - `CMakeLists.txt`
  - `.editorconfig`

### `[0.1.1]`

- Add `src` directory for implementation files
- Add `include` directory for header files
- Configure `CMakeLists.txt` to compile the project
- Add `defs.h` as the header file for the project
- Add `main.c` as the entry point of the program
- Add `reactor.h` as the header file for the reactor server
- Add `reactor.c` as the implementation file for the reactor server

### `[0.1.2]`

- Add `struct reactor` to represent the reactor server
- Add `reactor_init` to initialize a server instance
- Add `reactor_destroy` to destroy a server instance
