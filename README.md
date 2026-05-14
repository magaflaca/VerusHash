# verushash

Standalone C ABI shared library for VerusHash.

## What it provides

This repository builds `verushash` as a shared library.

It exposes two groups of functions:

- VerusHash digest functions.
- VerusHash 2.2 scan functions for pool mining work that includes a 140 byte Equihash-style header and a variable-length Verus solution vector.

The default hash version is VerusHash 2.2.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --config Release --prefix dist
```

Windows x64:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
cmake --install build --config Release --prefix dist
```

Windows x86:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release
cmake --install build --config Release --prefix dist
```

Windows ARM64:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A ARM64
cmake --build build --config Release
cmake --install build --config Release --prefix dist
```

## Hash API

```c
#include "verushash_lib.h"

unsigned char out[VERUSHASH_OUTPUT_SIZE];
int rc = verushash_hash(input, input_len, out);
```

`rc` is `VERUSHASH_SUCCESS` when the hash was written.
The output is always 32 bytes.

## Mining scan API

Use this API when the pool job already provides:

- 140 byte Equihash-style block header.
- Verus solution vector from the pool job.
- 32 byte share target in little-endian numeric form.

```c
unsigned char out_solution[VERUSHASH_MAX_SOLUTION_SIZE];
unsigned char out_hash[VERUSHASH_OUTPUT_SIZE];
uint64_t hashes_done = 0;
uint64_t found_nonce = 0;

int rc = verushash_scan_v2_2(
    header140,
    VERUSHASH_EQUIHASH_HEADER_SIZE,
    solution,
    solution_len,
    target32_le,
    start_nonce,
    nonce_count,
    &hashes_done,
    &found_nonce,
    out_solution,
    out_hash
);
```

Return values:

- `VERUSHASH_SCAN_FOUND`: `out_solution`, `out_hash`, and `found_nonce` are valid.
- `VERUSHASH_SCAN_NOT_FOUND`: no share was found in the requested nonce range.
- Any `VERUSHASH_ERROR_*`: invalid input or internal failure.

The returned solution has the same length as the input solution and has the mined nonce inserted at the Verus solution extra field used by the original Verus miner.

## Release artifacts

GitHub Actions builds:

- `linux-x86_64`
- `linux-aarch64`
- `macos-universal2`
- `windows-x86`
- `windows-x64`
- `windows-arm64`

Create a tag like `v1.2.0` to publish the artifacts to a GitHub release.

## Portable mode

Portable mode is enabled by default.
It avoids CPU-specific binaries and is the correct mode for public releases.

To enable runtime optimized x86 code in a local build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVERUSHASH_PORTABLE_ONLY=OFF
```

`windows-arm64` is Windows on ARM64, not 32-bit x86.
