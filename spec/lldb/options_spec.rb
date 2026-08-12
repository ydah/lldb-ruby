# frozen_string_literal: true

require 'tmpdir'
require 'open3'

RSpec.describe LLDB::Debugger do
  it 'passes source_init_files directly to LLDB' do
    Dir.mktmpdir('lldb-ruby-home') do |home|
      File.write(File.join(home, '.lldbinit'), "settings set target.max-children-count 7\n")
      source = <<~RUBY
        require 'lldb'
        LLDB.initialize
        [false, true].each do |source_init_files|
          debugger = LLDB::Debugger.create(source_init_files: source_init_files)
          result = debugger.command_interpreter.handle_command('settings show target.max-children-count')
          puts result.output
          debugger.close
        end
        LLDB.terminate
      RUBY
      stdout, stderr, status = Open3.capture3(
        { 'HOME' => home }, RbConfig.ruby, '-I', File.expand_path('../../lib', __dir__), '-e', source,
        chdir: home
      )

      expect(status).to be_success, stderr
      expect(stdout.lines[0]).not_to include('= 7')
      expect(stdout.lines[1]).to include('= 7')
    end
  end
end

RSpec.describe LLDB::LaunchInfo do
  let(:executable) { compile_fixture('simple') }

  it 'exposes launch choices without imposing Ruby defaults' do
    info = described_class.new
    file = LLDB::FileSpec.new(executable, resolve: true)
    listener = LLDB::Listener.new('launch listener')

    info.arguments = [executable, 'argument']
    info.working_directory = File.dirname(executable)
    info.set_environment({ 'LLDB_RUBY_OPTION' => 'enabled' }, append: false)
    info.executable_file = file
    info.listener = listener
    info.process_plugin_name = 'posix'
    info.shell = '/bin/sh'

    expect(info.executable_file.path).to eq(executable)
    expect(info.arguments).to eq([executable, 'argument'])
    expect(info.working_directory).to eq(File.dirname(executable))
    expect(info.environment_entries).to include('LLDB_RUBY_OPTION=enabled')
    expect(info.listener).to be_a(LLDB::Listener)
    expect(info.process_plugin_name).to eq('posix')
    expect(info.shell).to eq('/bin/sh')
    expect(info.add_close_file_action(0)).to be true
    expect(info.add_duplicate_file_action(1, 2)).to be true
    expect(info.add_suppress_file_action(2, read: true)).to be true
  ensure
    listener&.close
    file&.close
    info&.close
  end
end

RSpec.describe LLDB::AttachInfo do
  it 'round-trips attach choices' do
    info = described_class.new(123)
    listener = LLDB::Listener.new('attach listener')

    info.wait_for_launch = true
    info.ignore_existing = true
    info.resume_count = 2
    info.process_plugin_name = 'posix'
    info.listener = listener

    expect(info.pid).to eq(123)
    expect(info.wait_for_launch?).to be true
    expect(info.ignore_existing?).to be true
    expect(info.resume_count).to eq(2)
    expect(info.process_plugin_name).to eq('posix')
    expect(info.listener).to be_a(LLDB::Listener)
  ensure
    listener&.close
    info&.close
  end
end

RSpec.describe LLDB::ExpressionOptions do
  it 'round-trips expression controls' do
    options = described_class.new

    options.timeout = 1234
    options.unwind_on_error = false
    options.ignore_breakpoints = true
    options.fetch_dynamic_value = LLDB::DynamicValue::DONT_RUN_TARGET
    options.try_all_threads = false
    options.stop_others = true
    options.language = 0
    options.suppress_persistent_result = true

    expect(options.timeout).to eq(1234)
    expect(options.unwind_on_error?).to be false
    expect(options.ignore_breakpoints?).to be true
    expect(options.fetch_dynamic_value).to eq(LLDB::DynamicValue::DONT_RUN_TARGET)
    expect(options.try_all_threads?).to be false
    expect(options.stop_others?).to be true
    expect(options.language).to eq(0)
    expect(options.suppress_persistent_result?).to be true
  ensure
    options&.close
  end
end
