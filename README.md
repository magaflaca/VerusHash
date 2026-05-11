# verushash

Standalone C ABI shared library for VerusHash.

Maintainer: isawicca <isa@cenizalunar.com>

## About

This repository contains the VerusHash core and a small C wrapper.
It does not include wallet, RPC, network, CLI, mining, or blockchain code.

The default function uses VerusHash 2.2.

## API

```c
#include "verushash_lib.h"

unsigned char out[VERUSHASH_OUTPUT_SIZE];
int rc = verushash_hash(input, input_len, out);
```

`rc` is `VERUSHASH_SUCCESS` when the hash was written.
The output is always 32 bytes.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --config Release --prefix dist
```

Windows:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
cmake --install build --config Release --prefix dist
```

## Release artifacts

GitHub Actions builds a small release set:

- `linux-x86_64`
- `linux-aarch64`
- `macos-universal2`
- `windows-x64`
- `windows-arm64`

Create a tag like `v1.0.0` to publish the artifacts to a GitHub release.

## Portable mode

Portable mode is enabled by default.
It avoids CPU-specific binaries and is the right choice for public releases.

To enable runtime optimized x86 code in a local build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVERUSHASH_PORTABLE_ONLY=OFF
```
