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

  def write_fixture(directory, rbs: 'def self.lldb_fixture: () -> void')
    File.write(File.join(directory, 'wrapper.h'), <<~HEADER)
      #ifndef WRAPPER_H
      #define WRAPPER_H
      void lldb_fixture(void);
      #endif
    HEADER
    File.write(File.join(directory, 'wrapper.cpp'), <<~CPP)
      void lldb_fixture(void) {}
    CPP
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
            kind: reviewed_no_throw
            reason: Fixture does not cross a native exception boundary.
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
end
