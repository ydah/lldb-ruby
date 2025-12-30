# frozen_string_literal: true

RSpec.describe LLDB::Process do
  let(:debugger) { LLDB::Debugger.create }
  let(:executable) { compile_fixture('simple') }
  let(:target) { debugger.create_target(executable) }

  describe '#launch' do
    before do
      # Set async to false to run synchronously
      debugger.async = false
      # Set a breakpoint at main to stop the process
      target.breakpoint_create_by_name('main')
    end

    it 'launches a process' do
      process = target.launch
      expect(process).to be_a(LLDB::Process)
      expect(process).to be_valid
      process.kill
    end

    it 'returns a stopped process when breakpoint is hit' do
      process = target.launch
      expect(process).to be_stopped
      process.kill
    end

    it 'has a valid process ID' do
      process = target.launch
      expect(process.pid).to be > 0
      process.kill
    end
  end

  describe '#state_name' do
    before do
      debugger.async = false
      target.breakpoint_create_by_name('main')
    end

    it "returns 'stopped' when stopped" do
      process = target.launch
      expect(process.state_name).to eq('stopped')
      process.kill
    end
  end

  describe '#num_threads' do
    before do
      debugger.async = false
      target.breakpoint_create_by_name('main')
    end

    it 'returns at least 1 thread' do
      process = target.launch
      expect(process.num_threads).to be >= 1
      process.kill
    end
  end

  describe '#threads' do
    before do
      debugger.async = false
      target.breakpoint_create_by_name('main')
    end

    it 'returns an array of threads' do
      process = target.launch
      threads = process.threads
      expect(threads).not_to be_empty
      expect(threads.first).to be_a(LLDB::Thread)
      expect(threads.first).to be_valid
      process.kill
    end
  end

  describe '#selected_thread' do
    before do
      debugger.async = false
      target.breakpoint_create_by_name('main')
    end

    it 'returns the selected thread' do
      process = target.launch
      thread = process.selected_thread
      expect(thread).to be_a(LLDB::Thread)
      expect(thread).to be_valid
      process.kill
    end
  end

  describe '#thread_by_id' do
    before do
      debugger.async = false
      target.breakpoint_create_by_name('main')
    end

    it 'returns a thread by ID' do
      process = target.launch
      thread = process.selected_thread
      found = process.thread_by_id(thread.id)

      expect(found).to be_a(LLDB::Thread)
      expect(found).to be_valid
      process.kill
    end

    it 'returns nil for non-existent ID' do
      process = target.launch
      found = process.thread_by_id(0xFFFFFFFF)
      expect(found).to be_nil
      process.kill
    end
  end

  describe '#unique_id' do
    before do
      debugger.async = false
      target.breakpoint_create_by_name('main')
    end

    it 'returns a unique ID' do
      process = target.launch
      expect(process.unique_id).to be > 0
      process.kill
    end
  end

  describe '#get_stdout' do
    before do
      debugger.async = false
      target.breakpoint_create_by_name('main')
    end

    it 'returns stdout output as string' do
      process = target.launch
      stdout = process.get_stdout
      expect(stdout).to be_a(String)
      process.kill
    end
  end

  describe '#get_stderr' do
    before do
      debugger.async = false
      target.breakpoint_create_by_name('main')
    end

    it 'returns stderr output as string' do
      process = target.launch
      stderr = process.get_stderr
      expect(stderr).to be_a(String)
      process.kill
    end
  end

  describe '#send_async_interrupt' do
    before do
      debugger.async = true
    end

    it 'returns without error' do
      process = target.launch
      sleep(0.1)

      if process.running?
        result = process.send_async_interrupt
        expect(result).to be true
      end

      process.kill
    end
  end
end
