# frozen_string_literal: true

require 'rbconfig'

module LLDB
  module BuildDiscovery
    Candidate = Struct.new(:include_dir, :lib_dir, :llvm_config, keyword_init: true)

    DEFAULT_PREFIXES = [
      '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr',
      '/Library/Developer/CommandLineTools/usr',
      '/opt/homebrew/opt/llvm',
      '/usr/local/opt/llvm',
      '/usr',
      '/usr/local'
    ].freeze

    module_function

    def candidates(options = {}, env: ENV, path: ENV.fetch('PATH', ''))
      explicit_include = options[:include] || options[:include_dir]
      explicit_lib = options[:lib] || options[:lib_dir]
      explicit_dir = options[:dir] || options[:lldb_dir] || env['LLDB_DIR']

      candidates = []
      candidates << Candidate.new(include_dir: explicit_include, lib_dir: explicit_lib) if explicit_include || explicit_lib

      if explicit_dir
        candidates << candidate_for_prefix(explicit_dir)
      end

      llvm_config_paths(path).each do |llvm_config|
        candidates << candidate_for_llvm_config(llvm_config)
      end

      prefix_paths(explicit_dir, env: env).each do |prefix|
        candidates << candidate_for_prefix(prefix)
      end

      candidates.compact.uniq { |candidate| [candidate.include_dir, candidate.lib_dir] }
    end

    def candidate_for_llvm_config(llvm_config)
      include_dir = command_output(llvm_config, '--includedir')
      lib_dir = command_output(llvm_config, '--libdir')
      return unless include_dir && lib_dir

      Candidate.new(include_dir: include_dir, lib_dir: lib_dir, llvm_config: llvm_config)
    end

    def candidate_for_prefix(prefix)
      return unless prefix

      include_dir = File.join(prefix, 'include')
      lib_dir = %w[lib lib64].map { |name| File.join(prefix, name) }.find do |path|
        File.directory?(path)
      end

      Candidate.new(include_dir: include_dir, lib_dir: lib_dir)
    end

    def prefix_paths(explicit_dir, env: ENV)
      paths = []
      paths << explicit_dir if explicit_dir
      paths << env['LLVM_PREFIX'] if env['LLVM_PREFIX']
      paths.concat(DEFAULT_PREFIXES)
      paths.concat(Dir.glob('/usr/lib/llvm-*').sort_by { |path| version_key(path) }.reverse)
      paths.compact.uniq
    end

    def llvm_config_paths(path)
      executables = path.split(File::PATH_SEPARATOR).flat_map do |directory|
        next [] unless File.directory?(directory)

        Dir.children(directory).filter_map do |name|
          next unless name.match?(/\Allvm-config(?:-\d+(?:\.\d+)*)?\z/)

          executable = File.join(directory, name)
          executable if File.executable?(executable)
        end
      end

      unique_executables = executables.uniq
      unversioned = unique_executables.select { |path| File.basename(path) == 'llvm-config' }
      versioned = unique_executables.reject { |path| File.basename(path) == 'llvm-config' }

      unversioned + versioned.sort_by { |path| version_key(path) }.reverse
    end

    def command_output(*command)
      output = IO.popen(command, &:read).strip
      output.empty? ? nil : output
    rescue SystemCallError
      nil
    end

    def version_key(value)
      value.to_s.scan(/\d+/).map(&:to_i)
    end
  end
end
