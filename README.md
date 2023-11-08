# dubdog

An ultimate CLI duplicate file finder and remover!

Watch the video tutorial on YouTube:
https://www.youtube.com/watch?v=qdoLNcz_zYg

## Installation of needed libraries
Boost and OpenSSL libraries are used for reading the files and computing the hashes.

On Ubuntu/Debian install these libraries with:

```bash
sudo apt install libboost-iostreams-dev libssl-dev
```

## Building

### Build with CMake

```bash
mkdir build
cd build/
cmake ..
cmake --build .
```

### Build with G++ directly:

```bash
g++ main.cpp -o dubdog -O3 -s -llibboost_iostreams-mt -lssl -lcrypto
```

## Usage

```
dubdog path extensions [options]

Options:

-a [md5]|crc32|fso    hash algorithm
-t count              threads count
-v                    view only (don't ask for file removal)
```

`extensions` is a comma separated list of file's extensions.

For example when running on Windows you may run:

```bash
dubdog 'C:\Books' pdf,djvu,epub,fb2
```

Current version compares files by file's MD5 hash by default. Optionally it's possible to use:

- CRC32 hash by specifying `-a crc32` in command line.
- `-a fso` parameter means comparison by "file size only".
