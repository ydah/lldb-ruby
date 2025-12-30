#!/usr/bin/env ruby
# frozen_string_literal: true

# Breakpoint management example
#
# This example demonstrates:
# - Creating different types of breakpoints
# - Managing breakpoint properties
# - Conditional breakpoints

require_relative '../lib/lldb'

PROGRAM_PATH = File.expand_path('test_program', __dir__)

unless File.exist?(PROGRAM_PATH)
  puts 'Please compile test_program.c first'
  exit 1
end

LLDB.initialize

begin
  debugger = LLDB::Debugger.create
  target = debugger.create_target(PROGRAM_PATH)

  puts '=== Breakpoint Types ==='

  # Breakpoint by function name
  bp1 = target.breakpoint_create_by_name('main')
  puts "Breakpoint by name 'main': ID=#{bp1.id}, locations=#{bp1.num_locations}"

  # Breakpoint by source location
  bp2 = target.breakpoint_create_by_location('test_program.c', 4)
  puts "Breakpoint by location: ID=#{bp2.id}"

  puts "\n=== Breakpoint Properties ==="

  # Disable a breakpoint
  bp1.disable
  puts "bp1 enabled? #{bp1.enabled?}"

  # Enable it again
  bp1.enable
  puts "bp1 enabled after enable? #{bp1.enabled?}"

  # Set condition
  bp2.condition = 'a > 5'
  puts "bp2 condition: #{bp2.condition}"

  # Set ignore count
  bp2.ignore_count = 2
  puts "bp2 ignore count: #{bp2.ignore_count}"

  # One-shot breakpoint
  bp2.one_shot = true
  puts "bp2 one-shot? #{bp2.one_shot?}"

  puts "\n=== All Breakpoints ==="
  target.breakpoints.each do |bp|
    puts bp
  end

  puts "\n=== Finding Breakpoints ==="
  found = target.find_breakpoint_by_id(bp1.id)
  puts "Found breakpoint #{found.id}" if found

  puts "\n=== Deleting Breakpoints ==="
  bp1.delete
  puts "Remaining breakpoints: #{target.num_breakpoints}"
ensure
  LLDB.terminate
end
