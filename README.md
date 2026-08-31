# Bengine

Bengine is a small, intentionally minimal reusable game engine in modern
C++, built on [raylib](https://www.raylib.com/). Not a general-purpose
engine — a clean architecture (static library, no owned `main()`) meant
to support my own games while I learn engine design.

## Requirements

- CMake 3.24+
- A C++20 compiler (this repo is developed against MSVC / Visual Studio 2022 on Windows)
- Internet access on first configure, so CMake's `FetchContent` can pull raylib

## Building and running an example

From the repo root:

```powershell
cmake -B build -S .
cmake --build build --config Debug
.\build\examples\00_hello_engine\Debug\00_hello_engine.exe
```

- The first command configures the project and fetches/builds raylib via
  `FetchContent`. This step is slow the first time (raylib is compiled from
  source) but is safe to re-run any time `CMakeLists.txt` changes.
- The second command builds `raylib`, the `bengine` static library, and every
  example and prototype.
- The third command runs the example directly from the build output. Each
  example is under `build/examples/<example_name>/Debug/<example_name>.exe`;
  prototypes are under `build/prototypes/<prototype_name>/Debug/<prototype_name>.exe`.

Swap `Debug` for `Release` in both the build and run commands for an
optimized build.

## Project layout

- `engine/` — Bengine itself, built as a static library (`bengine`) and
  exposed to consumers through the `engine::` C++ namespace. No `main()`,
  no game-specific logic.
- `examples/` — small standalone executables that link against `bengine`.
  Each one owns its own `main()` and demonstrates/exercises one engine
  capability at a time.
- `prototypes/` — small game prototypes that combine existing engine
  capabilities into something resembling a real game, to reveal what the
  engine needs next. Each one is its own executable linked against
  `bengine`, just like a real game would be.
- `docs/` — architecture notes and design decisions, added as they accumulate.
