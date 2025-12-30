#!/usr/bin/env ruby
# frozen_string_literal: true

# Expression evaluation example
#
# This example demonstrates:
# - Finding and inspecting variables
# - Evaluating expressions
# - Working with Value objects

require_relative '../lib/lldb'

PROGRAM_PATH = File.expand_path('test_program', __dir__)

unless File.exist?(PROGRAM_PATH)
  puts 'Please compile test_program.c first'
  exit 1
end

LLDB.initialize

begin
  debugger = LLDB::Debugger.create
  debugger.async = false
  target = debugger.create_target(PROGRAM_PATH)

  # Set breakpoint inside add function
  target.breakpoint_create_by_name('add')

  puts 'Launching process...'
  process = target.launch

  if process.stopped?
    thread = process.selected_thread
    frame = thread.selected_frame

    puts "Stopped at: #{frame.function_name}"
    puts "Location: #{frame.location}"

    puts "\n=== Variables ==="

    # Find individual variables
    a = frame.find_variable('a')
    b = frame.find_variable('b')

    if a&.valid?
      puts "Variable 'a':"
      puts "  Name: #{a.name}"
      puts "  Type: #{a.type_name}"
      puts "  Value: #{a.value}"
      puts "  As integer: #{a.to_i}"
      puts "  Byte size: #{a.byte_size}"
    end

    if b&.valid?
      puts "\nVariable 'b':"
      puts "  #{b.inspect}"
    end

    puts "\n=== Expression Evaluation ==="

    # Evaluate expressions
    expressions = [
      'a + b',
      'a * b',
      'a - b',
      'a / 2',
      '(a + b) * 2'
    ]

    expressions.each do |expr|
      result = frame.evaluate_expression(expr)
      if result&.valid?
        puts "#{expr} = #{result.value}"
      else
        puts "#{expr} = (error)"
      end
    end

    process.kill
  end
ensure
  LLDB.terminate
end
