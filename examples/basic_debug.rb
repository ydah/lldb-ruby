#!/usr/bin/env ruby
# frozen_string_literal: true

# Basic debugging example using LLDB Ruby bindings
#
# This example demonstrates:
# - Creating a debugger and target
# - Setting breakpoints
# - Launching a process
# - Inspecting variables at breakpoints
# - Stepping through code
#
# Usage:
#   # First, compile the test program:
#   gcc -g -O0 -o test_program examples/test_program.c
#
#   # Then run this script:
#   ruby examples/basic_debug.rb

require_relative '../lib/lldb'

# Path to the test program (compile with -g for debug info)
PROGRAM_PATH = File.expand_path('test_program', __dir__)

# Check if program exists
unless File.exist?(PROGRAM_PATH)
  puts "Test program not found at #{PROGRAM_PATH}"
  puts 'Please compile it first:'
  puts "  gcc -g -O0 -o #{PROGRAM_PATH} #{PROGRAM_PATH}.c"
  exit 1
end

# Initialize LLDB
LLDB.initialize

begin
  # Create a debugger instance
  debugger = LLDB::Debugger.create
  debugger.async = false # Run synchronously

  puts 'Created debugger'

  # Create a target from the executable
  target = debugger.create_target(PROGRAM_PATH)
  puts "Created target: #{target.executable_path}"

  # Set a breakpoint at main
  bp_main = target.breakpoint_create_by_name('main')
  puts "Set breakpoint at main: #{bp_main}"

  # Set a breakpoint at the add function
  bp_add = target.breakpoint_create_by_name('add')
  puts "Set breakpoint at add: #{bp_add}"

  # Launch the process
  puts "\nLaunching process..."
  process = target.launch

  puts "Process launched with PID: #{process.pid}"
  puts "Process state: #{process.state_name}"

  # Debug loop
  iteration = 0
  max_iterations = 10

  while process.stopped? && iteration < max_iterations
    iteration += 1
    puts "\n--- Iteration #{iteration} ---"

    thread = process.selected_thread
    frame = thread.selected_frame

    puts "Stopped at: #{frame.function_name}"
    puts "Location: #{frame.location}"
    puts "Stop reason: #{thread.stop_reason_name}"

    # Print local variables
    puts "\nLocal variables:"
    %w[a b result x y sum argc].each do |var_name|
      var = frame.find_variable(var_name)
      puts "  #{var.name} (#{var.type_name}) = #{var.value}" if var&.valid?
    end

    # Print the call stack
    puts "\nCall stack:"
    thread.frames.each_with_index do |f, i|
      puts "  ##{i}: #{f.function_name || '?'} at #{f.location}"
    end

    # Step over to the next line
    puts "\nStepping over..."
    thread.step_over

    # Wait a moment for the step to complete
    sleep 0.1
  end

  # Check final state
  if process.exited?
    puts "\nProcess exited with status: #{process.exit_status}"
  elsif process.stopped?
    puts "\nProcess still stopped"
    process.kill
  else
    puts "\nProcess state: #{process.state_name}"
    process.kill if process.valid?
  end
ensure
  LLDB.terminate
  puts "\nLLDB terminated"
end
