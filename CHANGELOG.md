# Changelog

## 1.2.0

- Changed scan API to support variable-length Verus solution vectors from pool jobs.
- Fixed the solution nonce offset calculation to match the original Verus miner.
- Removed the incorrect fixed 1344-byte solution requirement.

## 1.1.0

- Added VerusHash 2.2 mining scan API.
- Added solution nonce patching at the same offset used by the original Verus miner.
- Kept existing Linux, macOS, Windows, x86, x64, and ARM64 GitHub Actions release matrix.

## 1.0.0

- Initial standalone VerusHash shared library.
