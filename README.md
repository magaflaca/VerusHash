# verushash

Standalone C ABI shared library for VerusHash.

Maintainer: isawicca <isa@cenizalunar.com>

## What it is

This repo contains only the VerusHash hashing code and a small C wrapper.
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

Available entry points:

- `verushash_hash`
- `verushash_hash_v2_2`
- `verushash_hash_v2_1`
- `verushash_hash_v2b`
- `verushash_hash_v2`
- `verushash_hash_v1`
- `verushash_hash_v2b_version`

## Build locally

Linux:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --prefix dist
```

Windows:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
cmake --install build --config Release --prefix dist
```

## GitHub Actions

Push to `main` to build test artifacts.
Create a tag like `v1.0.0` to build release artifacts.

The workflow builds:

- Linux x64 on Ubuntu 22.04
- Linux x64 on Ubuntu 24.04
- Linux ARM64 cross-build
- Linux ARMv7 cross-build
- Windows x64
- Windows x86
- Windows ARM64

Release artifacts are attached automatically when the workflow runs from a `v*` tag.

## Files

- `verushash_lib.h`: public C ABI header
- `verushash_lib.cpp`: wrapper implementation
- `crypto/`: extracted VerusHash core
- `CMakeLists.txt`: shared library build
- `.github/workflows/build.yml`: CI and release builds
