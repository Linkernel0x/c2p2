# c2p2

This is a little software written in C++ while learning. I always really liked **CyberChef**, the online tool for encoding, decoding, and encrypting data. I wanted to create a similar software but offline and for terminal (that's where the name comes from!) :D

* **CLI Mode**: Quick, single-line commands for fast operations.
* **TUI Mode**: An interactive terminal UI (powered by FTXUI) to build and visualize data pipelines step-by-step.

## Modules

Here are the modules currently available in `c2p2`:

* **`rot13`** (`encode`) – Simple ROT13 cipher transformation
* **`base64`** (`encode`, `decode`) – Standard Base64 encoding and decoding
* **`hex`** (`encode`, `decode`) – Hexadecimal converter with optional parameters (`uppercase=true`, `delimiter=" "`)
* **`caesar`** (`encrypt`, `decrypt`) – Classic Caesar cipher with configurable `shift` parameter (default: 3)

*More modules (like Enigma!) are coming soon :D*

## CLI Usage

You can use the command line interface to quickly pass data through a module:

# Basic usage
`./c2p2 <module> <action> [--params...] ["text" | --input-file <path>] [--output-file <path>]`

```bash
# Examples
./c2p2 rot13 encode "Hello World"
./c2p2 caesar encrypt --shift=5 "Secret Message"
./c2p2 hex encode "Data to Hex"
./c2p2 base64 decode "SGVsbG8="

```

## TUI Usage

Launch the binary without arguments to enter the Interactive TUI:

```bash
./c2p2
```

In TUI mode, you can create a pipeline of multiple modules working together. You can test inputs live in the preview box or process files directly:

* `add <id> <module> <action> [--params...]` – Adds a step to your pipeline.
* `remove <id>` – Removes a step from the pipeline.
* `list` – Lists all available modules and actions.
* `run [--input-file <path>] [--output-file <path>] ["text"]` – Runs the active pipeline.
* `clear` – Clears the current pipeline.
* `help` – Displays help information.

*More coming soon!*

## Building the Project

Requirements:

* C++20 compatible compiler (GCC, Clang, or MSVC)
* CMake (3.20+)

```bash
mkdir build && cd build
cmake ..
cmake --build .
```