#!/usr/bin/env ruby
# frozen_string_literal: true

require 'mkmf'

# Enable C++17
$CXXFLAGS ||= ''
$CXXFLAGS << ' -std=c++17'

# Common LLDB search paths
lldb_search_paths = [
  # macOS Xcode
  '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr',
  '/Library/Developer/CommandLineTools/usr',
  # Homebrew LLVM on macOS (Apple Silicon)
  '/opt/homebrew/opt/llvm',
  # Homebrew LLVM on macOS (Intel)
  '/usr/local/opt/llvm',
  # Linux system paths
  '/usr',
  '/usr/local'
]

# Find LLDB version-specific paths on Linux
Dir.glob('/usr/lib/llvm-*').each do |path|
  lldb_search_paths << path
end

lldb_lib_dir = nil
lldb_include_dir = nil

# Try to find LLDB using lldb-config or llvm-config
llvm_config = find_executable('llvm-config') ||
              find_executable('llvm-config-18') ||
              find_executable('llvm-config-17') ||
              find_executable('llvm-config-16') ||
              find_executable('llvm-config-15') ||
              find_executable('llvm-config-14')

if llvm_config
  lldb_lib_dir = `#{llvm_config} --libdir`.strip
  lldb_include_dir = `#{llvm_config} --includedir`.strip
else
  # Search manually
  lldb_search_paths.each do |base_path|
    next unless File.directory?(base_path)

    lib_candidates = [
      File.join(base_path, 'lib'),
      File.join(base_path, 'lib64')
    ]

    lib_candidates.each do |lib_path|
      next unless File.directory?(lib_path)

      # Check for liblldb
      has_lldb = File.exist?(File.join(lib_path, 'liblldb.so')) ||
                 File.exist?(File.join(lib_path, 'liblldb.dylib')) ||
                 Dir.glob(File.join(lib_path, 'liblldb.so.*')).any?

      next unless has_lldb

      lldb_lib_dir = lib_path
      lldb_include_dir = File.join(base_path, 'include')
      break
    end

    break if lldb_lib_dir
  end
end


unless lldb_lib_dir
  abort <<~MSG

    *** ERROR: Could not find LLDB library ***

    Please install LLDB development files:

    On Ubuntu/Debian:
      sudo apt-get install lldb-14 liblldb-14-dev

    On Fedora/RHEL:
      sudo dnf install lldb-devel

    On macOS:
      xcode-select --install
      # or
      brew install llvm

    You can also set LLDB_DIR environment variable to point to your LLDB installation:
      LLDB_DIR=/path/to/llvm gem install lldb

  MSG
end

puts "Found LLDB library directory: #{lldb_lib_dir}"
puts "Found LLDB include directory: #{lldb_include_dir}" if lldb_include_dir

# Add include paths
$CXXFLAGS << " -I#{lldb_include_dir}" if lldb_include_dir && File.directory?(lldb_include_dir)

# Link against LLDB library
$LDFLAGS << " -L#{lldb_lib_dir} -llldb"
$LDFLAGS << " -Wl,-rpath,#{lldb_lib_dir}"

# Check for C++ standard library
$LDFLAGS << if RUBY_PLATFORM =~ /darwin/
              ' -lc++'
            else
              ' -lstdc++'
            end

# Create the extension as a shared library, not a Ruby native extension
# because we're using FFI to load it
$srcs = ['lldb_wrapper.cpp']
$objs = ['lldb_wrapper.o']

# Set the target library name
$DLDFLAGS ||= ''

if RUBY_PLATFORM =~ /darwin/
  target = 'liblldb_wrapper.dylib'
  $DLDFLAGS << ' -dynamiclib'
else
  target = 'liblldb_wrapper.so'
  $DLDFLAGS << ' -shared'
end

# Create a custom Makefile that builds a shared library
File.open('Makefile', 'w') do |f|
  f.puts <<~MAKEFILE
    CXX = #{RbConfig::CONFIG['CXX'] || 'c++'}
    CXXFLAGS = #{$CXXFLAGS} -fPIC
    LDFLAGS = #{$LDFLAGS} #{$DLDFLAGS}

    TARGET = #{target}
    SRCS = lldb_wrapper.cpp
    OBJS = lldb_wrapper.o

    all: $(TARGET)

    $(TARGET): $(OBJS)
    \t$(CXX) $(LDFLAGS) -o $@ $(OBJS)

    %.o: %.cpp
    \t$(CXX) $(CXXFLAGS) -c -o $@ $<

    install: $(TARGET)
    \tmkdir -p $(DESTDIR)#{RbConfig::CONFIG['sitelibdir']}/lldb
    \tcp $(TARGET) $(DESTDIR)#{RbConfig::CONFIG['sitelibdir']}/lldb/

    clean:
    \trm -f $(OBJS) $(TARGET)

    .PHONY: all install clean
  MAKEFILE
end

puts 'Makefile created successfully'
