# Changelog

## Unreleased

- Preserve LLDB operation status, error type, and operation context.
- Report native wrapper exceptions as `InternalBindingError`.
- Expose thread step modes and command return statuses.
- Return `nil` from `Process#send_async_interrupt`.
- Add idempotent native handles, shared debugger contexts, and deterministic debugger close.
- Make `Target#launch` preserve LLDB launch semantics without implicit flags or polling.
- Add `FileSpec` objects, dynamically sized path buffers, and LLDB sentinel constants.
- Add binding parity, surface-ledger, reproducible RBS, and sanitizer checks.
- Release the GVL around launch, attach, continue, stepping, and command execution calls.
- Expose explicit broadcaster, listener, and event handles for LLDB state events.
- Add `source_init_files`, `LaunchInfo`, `AttachInfo`, and `ExpressionOptions` controls.
- Expose structured `Address`, `LineEntry`, and `FileSpecList` objects for source and breakpoint locations.
- Expose `TypeMember` metadata for fields and direct or virtual base classes.
- Expose structured `Symbol`, `Function`, `CompileUnit`, and `Block` objects from modules and frames.
- Expose structured instruction lists and instruction bytes from frames.
- Add LLDB discovery and compile/link validation for Linux and macOS builds.
- Expose wrapper ABI and LLDB version metadata with capability-based watchpoint access checks.

## 0.2.0 - 2026-01-04

- Add MemoryRegionInfo bindings.
- Add APISupport module for feature detection.

## 0.1.0 - 2025-12-30

- Initial release.
