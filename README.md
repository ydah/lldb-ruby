# LLDB Ruby [![Gem Version](https://badge.fury.io/rb/lldb.svg)](https://badge.fury.io/rb/lldb) [![CI](https://github.com/ydah/lldb-ruby/actions/workflows/ci.yml/badge.svg)](https://github.com/ydah/lldb-ruby/actions/workflows/ci.yml)

Ruby bindings for the LLDB debugger.

## Overview

This gem provides Ruby bindings for LLDB (Low Level Debugger), allowing you to access LLDB's debugging functionality from Ruby. It uses FFI (Foreign Function Interface) with a C wrapper around LLDB's C++ API.

## Requirements

- Ruby 3.0 or later
- LLDB 14 or later (Linux and macOS)
- C++17 compatible compiler (gcc 8+ / clang 10+)
- libffi-dev (for the FFI gem)

### Installing LLDB

**Ubuntu/Debian:**
```bash
sudo apt-get install lldb-14 liblldb-14-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install lldb-devel
```

**macOS:**
```bash
xcode-select --install
# or
brew install llvm
```

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'lldb'
```

And then execute:

```bash
bundle install
```

Or install it yourself as:

```bash
gem install lldb
```

## Usage

### Basic Example

```ruby
require 'lldb'

# Initialize LLDB
LLDB.initialize

# Create a debugger
debugger = LLDB::Debugger.create
debugger.async = false  # Run synchronously

# Create a target from an executable
target = debugger.create_target('./my_program')

# Set a breakpoint at main
bp = target.breakpoint_create_by_name('main')
puts "Breakpoint #{bp.id} created with #{bp.num_locations} locations"

# Launch the process
process = target.launch

# Check if stopped at breakpoint
if process.stopped?
  thread = process.selected_thread
  frame = thread.selected_frame

  puts "Stopped at: #{frame.function_name}"
  puts "Location: #{frame.location}"

  # Find a variable
  var = frame.find_variable('argc')
  puts "argc = #{var.value}" if var

  # Evaluate an expression
  result = frame.evaluate_expression('argc + 1')
  puts "argc + 1 = #{result.value}" if result

  # Continue execution
  process.continue
end

# Clean up
process.kill if process.valid?
LLDB.terminate
```

### Creating Breakpoints

```ruby
# By function name
bp = target.breakpoint_create_by_name('main')

# By source location
bp = target.breakpoint_create_by_location('main.c', 10)

# By address
bp = target.breakpoint_create_by_address(0x100001000)

# Configure breakpoint
bp.condition = 'x > 5'
bp.ignore_count = 2
bp.one_shot = true
bp.disable
bp.enable
```

### Stepping Through Code

```ruby
thread = process.selected_thread

# Step over (next line)
thread.step_over

# Step into (enter function)
thread.step_into

# Step out (return from function)
thread.step_out

# Step one instruction
thread.step_instruction
```

### Inspecting Variables

```ruby
frame = thread.selected_frame

# Find a variable by name
var = frame.find_variable('my_variable')

if var && var.valid?
  puts "Name: #{var.name}"
  puts "Type: #{var.type_name}"
  puts "Value: #{var.value}"
  puts "Size: #{var.byte_size} bytes"

  # For integer types
  puts "As integer: #{var.to_i}"

  # For complex types with children
  if var.might_have_children?
    var.each do |child|
      puts "  #{child.name} = #{child.value}"
    end
  end
end
```

`Target#launch` passes the launch request directly to LLDB. It does not add
`STOP_AT_ENTRY`, wait for a state transition, or auto-continue to a breakpoint.
Set `launch_flags: LLDB::LaunchFlags::STOP_AT_ENTRY` explicitly when that
behavior is required; callers are responsible for waiting on asynchronous
processes.

### Handling Events Explicitly

Use LLDB's listener queue when asynchronous state changes are needed:

```ruby
listener = debugger.listener
broadcaster = process.broadcaster
listener.start_listening_for_events(broadcaster, LLDB::Process::BroadcastBit::STATE_CHANGED)

if (event = listener.wait_for_event(timeout_seconds: 1))
  puts "state = #{LLDB::State.name(event.process_state)}"
end
```

`timeout_seconds: 0` performs a non-blocking poll. The library does not start
background event threads.

Source locations remain structured when needed: `Frame#line_entry` exposes its
`LineEntry`, including `start_address`, `end_address`, file, line, and column;
`BreakpointLocation#address` exposes an `Address` with separate
`file_address` and `load_address` values. `FileSpecList` can collect and return
`FileSpec` objects without reducing them to path strings.

Debugger and launch choices are explicit option objects. For example,
`LLDB::Debugger.create(source_init_files: true)` opts into LLDB init files,
while `LLDB::ExpressionOptions` can be passed to expression evaluation without
changing the no-options call path.

### Working with Threads

```ruby
# Get all threads
process.threads.each do |thread|
  puts "Thread #{thread.id}: #{thread.name || 'unnamed'}"
  puts "  Stop reason: #{thread.stop_reason_name}"
end

# Get the call stack
thread.frames.each_with_index do |frame, i|
  puts "##{i}: #{frame.function_name} at #{frame.location}"
end
```

### Attaching to a Running Process

```ruby
process = target.attach(pid: 12345)
```

## API Reference

### Main Classes

- `LLDB::Debugger` - Entry point for debugging operations
- `LLDB::Target` - Represents a debug target (executable)
- `LLDB::Process` - Represents a running process
- `LLDB::Thread` - Represents an execution thread
- `LLDB::Frame` - Represents a stack frame
- `LLDB::Breakpoint` - Represents a breakpoint
- `LLDB::Value` - Represents a variable or expression result
- `LLDB::Module` - Represents a loaded module
- `LLDB::Error` - Represents an error from LLDB

### Constants

Process states are available in `LLDB::State`:
- `INVALID`, `UNLOADED`, `CONNECTED`, `ATTACHING`, `LAUNCHING`
- `STOPPED`, `RUNNING`, `STEPPING`, `CRASHED`, `DETACHED`, `EXITED`, `SUSPENDED`

Stop reasons are available in `LLDB::StopReason`:
- `INVALID`, `NONE`, `TRACE`, `BREAKPOINT`, `WATCHPOINT`
- `SIGNAL`, `EXCEPTION`, `EXEC`, `PLAN_COMPLETE`, `THREAD_EXITING`, `INSTRUMENTATION`

## Development

The native wrapper build discovers LLDB in this order:

1. `--with-lldb-include` and `--with-lldb-lib`
2. `--with-lldb-dir`
3. `LLDB_DIR`
4. `llvm-config` or a versioned `llvm-config-N` on `PATH`
5. Homebrew, Xcode, and standard LLVM prefixes

Discovery succeeds only after compiling and linking a C++17 probe against
`liblldb`. The selected paths and optional API capabilities are printed during
the build. Windows is not a supported platform yet.

After checking out the repo, run:

```bash
bundle install
cd ext/lldb && ruby extconf.rb && make && cd ../..
bundle exec rspec
```

Binding and type-surface checks can be run with:

```bash
bundle exec rake bindings:check
bundle exec rake rbs:verify
```

`bindings/surface.yml` records the classification and exception-safety review
for every native export. Update it together with any new header declaration.

Only calls that can wait for the inferior, debugger server, or command script
release Ruby's GVL. The wrapper does not invoke Ruby callbacks from those
calls; adding callback APIs requires revisiting this policy.

To run tests, you need to compile the test fixtures:

```bash
gcc -g -O0 -o spec/fixtures/simple spec/fixtures/simple.c
```

## Contributing

Bug reports and pull requests are welcome on GitHub.

## License

Dual licensed under the [MIT License](LICENSE-MIT) and [Apache License 2.0](LICENSE-APACHE).

## Acknowledgments

- [LLVM/LLDB Project](https://lldb.llvm.org/)
- [Ruby FFI](https://github.com/ffi/ffi)
