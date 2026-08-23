# c2p2

This is a little software written in C++ while learning. I always really liked **CyberChef**, the online tool for encoding, decoding, and encrypting data. I wanted to create a similar software but offline and for terminal (that's where the name comes from!) :D

* **CLI Mode**: Quick, single-line commands for fast operations.
* **TUI Mode**: An interactive terminal UI (powered by FTXUI) to build and visualize data pipelines step-by-step.

## Modules

Here are the modules currently available in `c2p2`:
#### Encoding/Decoding
* **`base64`** (`encode`, `decode`) – Standard Base64 encoding and decoding
* **`hex`** (`encode`, `decode`) – Hexadecimal converter with optional parameters (`--uppercase=true`, `--delimiter=" "`)
* **`url`** (`encode`, `decode`) – URL encoding and decoding
* **`html`** (`encode`, `decode`) – HTML entity encoding and decoding
* **`binary`** (`encode`, `decode`) – Binary converter with optional parameter `--delimiter`
#### Encryption/Decryption
* **`rot13`** (`encrypt`, `decrypt`) – Simple ROT13 cipher transformation with configurable `--shift` parameter (default: 13)
* **`rot47`** (`encrypt`, `decrypt`) – Simple ROT47 cipher transformation with configurable `--shift` parameter (default: 47)
* **`rot8000`** (`encrypt`, `decrypt`) – Simple ROT8000 cipher transformation
* **`caesar_box`** (`encrypt`, `decrypt`) – Caesar Box transposition cipher with configurable `--length` parameter (default: 3)
* **`vigenere`** (`encrypt`, `decrypt`) – Vigenère cipher with configurable `--key` parameter
* **`enigma`** (`process`) – Enigma machine simulation with `--config` parameter for machine settings
#### Files
* **`archive`** (`compress`, `decompress`) – Compress (in a specific `--format`, with configurable `--filename`) and decompress files. Supported formats: `zip|xar|ar|ar.svr4|shar|shar.dump|gz|gzip|bz2|bzip2|xz|zstd|lz4|lzip|lzma|lzo|z|uu|uuencode|grz|tar.gz|tgz|tar.bz2|tbz2|tar.xz|txz|tar.zst|tzst|tar.lz4|tlz4|tar.lz|tar.lzma|tlz|tar.lzo|tar.Z|cpio.gz|cpio.xz|cpio.zst`
#### Hashing
* **`md`** (`hash`, `check`) – Hashing with MD4 and MD5 (set in `--md`, default: 5) and configurable `--format` parameter
* **`sha`** (`hash`, `check`) – Hashing with different SHA variants (set in `--sha`, default: 256) and configurable `--variant` parameter

*More modules are coming soon :D*

## CLI Usage

You can use the command line interface to quickly pass data through a module:

# Basic usage
`./c2p2 <module> <action> [--params...] ["text" | --input-file <path>] [--output-file <path>]`

```bash
# Examples
./c2p2 binary encode "Hello World"
./c2p2 rot13 encrypt --shift=5 "Secret Message"
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
