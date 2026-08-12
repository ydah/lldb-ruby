# frozen_string_literal: true

require 'open3'
require 'tmpdir'

RSpec.describe 'binding parity checker' do
  let(:script) { File.expand_path('../../script/check_bindings', __dir__) }

  def run_checker(directory)
    Open3.capture3(
      RbConfig.ruby,
      script,
      '--header', File.join(directory, 'wrapper.h'),
      '--source', File.join(directory, 'wrapper.cpp'),
      '--ffi', File.join(directory, 'ffi.rb'),
      '--rbs', File.join(directory, 'ffi.rbs'),
      '--surface', File.join(directory, 'surface.yml'),
      '--constants', File.join(directory, 'constants.yml'),
      '--skip-library',
      '--skip-constants'
    )
  end

  def write_fixture(directory, rbs: 'def self.lldb_fixture: () -> void', guarded: true)
    File.write(File.join(directory, 'wrapper.h'), <<~HEADER)
      #ifndef WRAPPER_H
      #define WRAPPER_H
      #ifdef __cplusplus
      #define LLDB_WRAPPER_NOEXCEPT noexcept
      #else
      #define LLDB_WRAPPER_NOEXCEPT
      #endif
      void lldb_fixture(void) LLDB_WRAPPER_NOEXCEPT;
      #endif
    HEADER
    source = if guarded
               <<~CPP
                 void lldb_fixture(void) LLDB_WRAPPER_NOEXCEPT {
                   try {
                   } catch (...) {
                   }
                 }
               CPP
             else
               "void lldb_fixture(void) {}\n"
             end
    File.write(File.join(directory, 'wrapper.cpp'), source)
    File.write(File.join(directory, 'ffi.rb'), "attach_function :lldb_fixture, [], :void\n")
    File.write(File.join(directory, 'ffi.rbs'), "#{rbs}\n")
    File.write(File.join(directory, 'constants.yml'), "version: 1\nconstants: []\n")
    File.write(File.join(directory, 'surface.yml'), <<~YAML)
      version: 1
      entries:
        lldb_fixture:
          classification: public
          reason: Fixture export for parity testing.
          exception_guard:
            kind: error_boundary
            reason: Fixture catches native exceptions before returning through C ABI.
      ruby_methods: []
    YAML
  end

  it 'accepts matching header, FFI, RBS, and ledger fixtures' do
    Dir.mktmpdir('lldb-ruby-bindings') do |directory|
      write_fixture(directory)
      _stdout, stderr, status = run_checker(directory)

      expect(status).to be_success, stderr
    end
  end

  it 'rejects a missing RBS declaration' do
    Dir.mktmpdir('lldb-ruby-bindings') do |directory|
      write_fixture(directory, rbs: '')
      _stdout, stderr, status = run_checker(directory)

      expect(status).not_to be_success
      expect(stderr).to include('FFI/RBS')
    end
  end

  it 'rejects an unguarded C ABI export' do
    Dir.mktmpdir('lldb-ruby-bindings') do |directory|
      write_fixture(directory, guarded: false)
      _stdout, stderr, status = run_checker(directory)

      expect(status).not_to be_success
      expect(stderr).to include('no try/catch body')
    end
  end
end
