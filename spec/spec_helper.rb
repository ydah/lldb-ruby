# frozen_string_literal: true

require 'lldb'

RSpec.configure do |config|
  # Enable flags like --only-failures and --next-failure
  config.example_status_persistence_file_path = '.rspec_status'

  # Disable RSpec exposing methods globally on `Module` and `main`
  config.disable_monkey_patching!

  config.expect_with :rspec do |c|
    c.syntax = :expect
  end

  # Initialize LLDB before running tests
  config.before(:suite) do
    LLDB.initialize
  end

  # Terminate LLDB after running tests
  config.after(:suite) do
    LLDB.terminate
  end
end

# Helper to get fixture path
def fixture_path(name)
  File.expand_path("fixtures/#{name}", __dir__)
end

# Helper to compile a C fixture
def compile_fixture(name)
  source = fixture_path("#{name}.c")
  output = fixture_path(name)

  unless File.exist?(output) && File.mtime(output) > File.mtime(source)
    system('gcc', '-g', '-O0', '-o', output, source, exception: true)
  end

  output
end
