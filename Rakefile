# frozen_string_literal: true

require 'bundler/gem_tasks'
require 'rspec/core/rake_task'

RSpec::Core::RakeTask.new(:spec)

namespace :compile do
  desc 'Compile the LLDB wrapper shared library'
  task :lldb_wrapper do
    ext_dir = File.expand_path('ext/lldb', __dir__)
    lib_dir = File.expand_path('lib/lldb', __dir__)

    Dir.chdir(ext_dir) do
      abort 'extconf.rb failed' unless system('ruby extconf.rb')
      abort 'make failed' unless system('make clean && make')

      FileUtils.mkdir_p(lib_dir)

      lib_name = case RUBY_PLATFORM
                 when /darwin/
                   'liblldb_wrapper.dylib'
                 when /mingw|mswin/
                   'lldb_wrapper.dll'
                 else
                   'liblldb_wrapper.so'
                 end

      if File.exist?(lib_name)
        FileUtils.cp(lib_name, lib_dir)
        puts "Copied #{lib_name} to #{lib_dir}"
      else
        abort "Library #{lib_name} not found"
      end
    end
  end
end

desc 'Compile the native extension'
task compile: ['compile:lldb_wrapper']

desc 'Clean build artifacts'
task :clean do
  ext_dir = File.expand_path('ext/lldb', __dir__)
  lib_dir = File.expand_path('lib/lldb', __dir__)

  Dir.chdir(ext_dir) do
    system('make clean') if File.exist?('Makefile')
    FileUtils.rm_f('Makefile')
  end

  FileUtils.rm_f(File.join(lib_dir, 'liblldb_wrapper.dylib'))
  FileUtils.rm_f(File.join(lib_dir, 'liblldb_wrapper.so'))
  FileUtils.rm_f(File.join(lib_dir, 'lldb_wrapper.dll'))
  FileUtils.rm_rf('tmp')
end

namespace :rbs do
  desc 'Install RBS collection'
  task :install do
    sh 'bundle exec rbs collection install'
  end

  desc 'Generate RBS files from inline annotations'
  task :generate do
    sh 'bundle exec rbs-inline --output sig lib'
  end
end

namespace :steep do
  desc 'Run steep type check'
  task check: ['rbs:install', 'rbs:generate'] do
    sh 'bundle exec steep check'
  end
end

desc 'Run steep type check'
task steep: ['steep:check']

task default: %i[compile spec]
