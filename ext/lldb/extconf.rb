#!/usr/bin/env ruby
# frozen_string_literal: true

require 'mkmf'
require 'open3'
require 'rbconfig'
require 'shellwords'
require 'tmpdir'

require_relative 'discovery'

unless RbConfig::CONFIG['host_os'] =~ /darwin|linux/
  abort "Unsupported platform: #{RbConfig::CONFIG['host_os']} (supported: Linux, macOS)"
end

$CXXFLAGS ||= ''
$CXXFLAGS << ' -std=c++17'

def run_command(*command)
  stdout, stderr, status = Open3.capture3(*command)
  [stdout, stderr, status]
end

def native_probe(compiler, include_dir, lib_dir, source, link:)
  Dir.mktmpdir('lldb-ruby-probe') do |directory|
    source_path = File.join(directory, 'probe.cpp')
    output_path = File.join(directory, 'probe')
    File.write(source_path, source)

    command = [*compiler, '-std=c++17', '-I', include_dir]
    if link
      command.concat([source_path, '-L', lib_dir, '-llldb', '-o', output_path])
      command << "-Wl,-rpath,#{lib_dir}"
      command << (RbConfig::CONFIG['host_os'] =~ /darwin/ ? '-lc++' : '-lstdc++')
    else
      command.concat(['-fsyntax-only', source_path])
    end

    run_command(*command)
  end
end

def lldb_library_present?(lib_dir)
  return false unless lib_dir && File.directory?(lib_dir)

  Dir.glob(File.join(lib_dir, 'liblldb.{so,dylib}*')).any?
end

def lldb_probe(compiler, candidate)
  return [false, 'include directory does not exist'] unless File.directory?(candidate.include_dir)
  return [false, 'library directory does not contain liblldb'] unless lldb_library_present?(candidate.lib_dir)

  source = <<~CPP
    #include <lldb/API/LLDB.h>

    int main() {
      lldb::SBDebugger debugger;
      return debugger.IsValid() ? 0 : 0;
    }
  CPP

  stdout, stderr, status = native_probe(
    compiler,
    candidate.include_dir,
    candidate.lib_dir,
    source,
    link: true
  )
  return [true, nil] if status.success?

  [false, [stdout, stderr].reject(&:empty?).join("\n")]
end

def watchpoint_capability_probe(compiler, candidate)
  source = <<~CPP
    #include <lldb/API/SBWatchpoint.h>

    int main() {
      auto reads = static_cast<bool (lldb::SBWatchpoint::*)()>(
        &lldb::SBWatchpoint::IsWatchingReads);
      auto writes = static_cast<bool (lldb::SBWatchpoint::*)()>(
        &lldb::SBWatchpoint::IsWatchingWrites);
      (void)reads;
      (void)writes;
      return 0;
    }
  CPP

  _stdout, _stderr, status = native_probe(
    compiler,
    candidate.include_dir,
    candidate.lib_dir,
    source,
    link: false
  )
  status.success?
end

def api_capability_probe(compiler, candidate, header, expression)
  source = <<~CPP
    #include <lldb/API/#{header}>

    int main() {
      auto method = #{expression};
      (void)method;
      return 0;
    }
  CPP

  _stdout, _stderr, status = native_probe(
    compiler,
    candidate.include_dir,
    candidate.lib_dir,
    source,
    link: false
  )
  status.success?
end

def llvm_config_version(candidate)
  llvm_config = candidate.llvm_config
  if !llvm_config
    possible = File.join(candidate.lib_dir.to_s, '..', 'bin', 'llvm-config')
    llvm_config = possible if File.executable?(possible)
  end
  return 'unknown' unless llvm_config

  stdout, _stderr, status = run_command(llvm_config, '--version')
  status.success? && !stdout.strip.empty? ? stdout.strip : 'unknown'
end

explicit_options = {
  dir: with_config('lldb-dir'),
  include: with_config('lldb-include'),
  lib: with_config('lldb-lib')
}.compact

compiler = Shellwords.split(ENV.fetch('CXX', RbConfig::CONFIG['CXX'] || 'c++'))
abort 'C++ compiler command is empty' if compiler.empty?
attempts = []
selected = nil

LLDB::BuildDiscovery.candidates(explicit_options).each do |candidate|
  success, reason = lldb_probe(compiler, candidate)
  unless success
    attempts << "#{candidate.inspect}: #{reason}"
    next
  end

  selected = candidate
  break
end

unless selected
  abort <<~MSG

    *** ERROR: Could not find a usable LLDB installation ***

    A C++17 program including lldb/API/LLDB.h and linking against liblldb
    must compile successfully. Tried:
    #{attempts.join("\n")}

    Supported inputs, in priority order:
      --with-lldb-include=/path/to/include --with-lldb-lib=/path/to/lib
      --with-lldb-dir=/path/to/llvm
      LLDB_DIR=/path/to/llvm
      llvm-config or llvm-config-N on PATH

    The compiler used was: #{compiler}
  MSG
end

puts "Found LLDB library directory: #{selected.lib_dir}"
puts "Found LLDB include directory: #{selected.include_dir}"
puts "Found LLDB via: #{selected.llvm_config || 'prefix discovery'}"
puts "Using C++ compiler: #{compiler.join(' ')}"

watchpoint_access_kind = watchpoint_capability_probe(compiler, selected)
symbol_get_base_name = api_capability_probe(
  compiler, selected, 'SBSymbol.h', '&lldb::SBSymbol::GetBaseName'
)
symbol_get_id = api_capability_probe(
  compiler, selected, 'SBSymbol.h', '&lldb::SBSymbol::GetID'
)
symbol_get_value = api_capability_probe(
  compiler, selected, 'SBSymbol.h', '&lldb::SBSymbol::GetValue'
)
symbol_get_size = api_capability_probe(
  compiler, selected, 'SBSymbol.h', '&lldb::SBSymbol::GetSize'
)
function_get_base_name = api_capability_probe(
  compiler, selected, 'SBFunction.h', '&lldb::SBFunction::GetBaseName'
)
build_version = llvm_config_version(selected)
config_path = File.expand_path('lldb_wrapper_config.h', __dir__)
File.write(config_path, <<~HEADER)
  #ifndef LLDB_WRAPPER_CONFIG_H
  #define LLDB_WRAPPER_CONFIG_H

  #define LLDB_RUBY_WRAPPER_ABI_VERSION 1
  #define LLDB_RUBY_BUILD_LLDB_VERSION #{build_version.dump}
  #define LLDB_RUBY_HAVE_WATCHPOINT_ACCESS_KIND #{watchpoint_access_kind ? 1 : 0}
  #define LLDB_RUBY_HAVE_SYMBOL_GET_BASE_NAME #{symbol_get_base_name ? 1 : 0}
  #define LLDB_RUBY_HAVE_SYMBOL_GET_ID #{symbol_get_id ? 1 : 0}
  #define LLDB_RUBY_HAVE_SYMBOL_GET_VALUE #{symbol_get_value ? 1 : 0}
  #define LLDB_RUBY_HAVE_SYMBOL_GET_SIZE #{symbol_get_size ? 1 : 0}
  #define LLDB_RUBY_HAVE_FUNCTION_GET_BASE_NAME #{function_get_base_name ? 1 : 0}

  #endif
HEADER
puts "Watchpoint access capability: #{watchpoint_access_kind ? 'supported' : 'unsupported'}"
puts "SBSymbol::GetBaseName capability: #{symbol_get_base_name ? 'supported' : 'unsupported'}"
puts "SBSymbol::GetID capability: #{symbol_get_id ? 'supported' : 'unsupported'}"
puts "SBSymbol::GetValue capability: #{symbol_get_value ? 'supported' : 'unsupported'}"
puts "SBSymbol::GetSize capability: #{symbol_get_size ? 'supported' : 'unsupported'}"
puts "SBFunction::GetBaseName capability: #{function_get_base_name ? 'supported' : 'unsupported'}"
puts "Build LLDB version: #{build_version}"

$CXXFLAGS << " -I#{selected.include_dir}"
$LDFLAGS << " -L#{selected.lib_dir} -llldb"
$LDFLAGS << " -Wl,-rpath,#{selected.lib_dir}"
$LDFLAGS << if RbConfig::CONFIG['host_os'] =~ /darwin/
              ' -lc++'
            else
              ' -lstdc++'
            end

# Create the extension as a shared library, not a Ruby native extension,
# because the Ruby layer loads it through FFI.
$srcs = ['lldb_wrapper.cpp']
$objs = ['lldb_wrapper.o']
$DLDFLAGS ||= ''

if RbConfig::CONFIG['host_os'] =~ /darwin/
  target = 'liblldb_wrapper.dylib'
  $DLDFLAGS << ' -dynamiclib'
else
  target = 'liblldb_wrapper.so'
  $DLDFLAGS << ' -shared'
end

File.open('Makefile', 'w') do |f|
  f.puts <<~MAKEFILE
    CXX = #{RbConfig::CONFIG['CXX'] || 'c++'}
    CXXFLAGS = #{$CXXFLAGS} -fPIC
    LDFLAGS = #{$LDFLAGS} #{$DLDFLAGS}

    TARGET = #{target}
    SRCS = lldb_wrapper.cpp
    OBJS = lldb_wrapper.o
    sitelibdir ?= #{RbConfig::CONFIG['sitelibdir']}
    sitearchdir ?= #{RbConfig::CONFIG['sitearchdir']}

    all: $(TARGET)

    $(TARGET): $(OBJS)
    \t$(CXX) $(LDFLAGS) -o $@ $(OBJS)

    %.o: %.cpp
    \t$(CXX) $(CXXFLAGS) -c -o $@ $<

    install: $(TARGET)
    \tmkdir -p $(DESTDIR)$(sitearchdir)/lldb
    \tcp $(TARGET) $(DESTDIR)$(sitearchdir)/lldb/

    clean:
    \trm -f $(OBJS) $(TARGET)

    .PHONY: all clean install
  MAKEFILE
end

puts 'Makefile created successfully'
