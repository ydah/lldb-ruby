# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

## [0.3.0] - 2026-08-12

### Added

- Expose thread step modes and command return statuses, and return `nil` from `Process#send_async_interrupt`.
- Add explicit broadcaster, listener, and event handles for LLDB state events.
- Add `source_init_files`, `LaunchInfo`, `AttachInfo`, and `ExpressionOptions` controls.
- Expose structured `FileSpec`, `Address`, `LineEntry`, and `FileSpecList` objects with dynamically sized path buffers and LLDB sentinel constants.
- Expose `TypeMember` metadata for fields and direct or virtual base classes.
- Expose structured `Symbol`, `Function`, `CompileUnit`, and `Block` objects from modules and frames.
- Expose structured instruction lists and instruction bytes from frames.
- Add LLDB discovery and compile/link validation for Linux and macOS builds.
- Expose wrapper ABI and LLDB version metadata with capability-based watchpoint access checks.
- Add binding parity, surface-ledger, reproducible RBS, and sanitizer checks.

### Changed

- Preserve LLDB operation status, error type, and operation context.
- Make `Target#launch` preserve LLDB launch semantics without implicit flags, polling, or auto-continue.
- Release Ruby's GVL around launch, attach, continue, stepping, and command execution calls.
- Normalize basic type values across LLDB versions.
- Pin the RBS collection revision and verify generated RBS deterministically.

### Fixed

- Report native wrapper exceptions as `InternalBindingError`.
- Add idempotent native handles, shared debugger contexts, and deterministic debugger close.
- Make native finalizer release safe under GC stress.
- Prevent native build artifacts from leaking into built gems.

## [0.2.0] - 2026-01-04

### Added

- Add MemoryRegionInfo bindings.
- Add APISupport module for feature detection.

## [0.1.0] - 2025-12-30

### Added

- Initial release.
