# frozen_string_literal: true

RSpec.describe LLDB::Watchpoint do
  let(:debugger) { LLDB::Debugger.create }
  let(:executable) { compile_fixture('simple') }
  let(:target) { debugger.create_target(executable) }
  let(:process) do
    debugger.async = false
    target.breakpoint_create_by_name('main')
    target.launch
  end

  after do
    process.kill if process&.valid?
  end

  describe 'watchpoint creation' do
    it 'can create a watchpoint at an address' do
      skip 'Watchpoints not supported on this platform' unless process.num_supported_hardware_watchpoints.positive?

      thread = process.selected_thread
      frame = thread.selected_frame
      var = frame.find_variable('x')
      skip 'Variable x not found' if var.nil?

      address = var.load_address
      skip 'Could not get load address' if address.zero?

      watchpoint = target.watch_address(address, 4, write: true)
      expect(watchpoint).to be_valid
      expect(watchpoint.id).to be > 0
    end
  end

  describe '#enabled?' do
    it 'returns true by default' do
      skip 'Watchpoints not supported on this platform' unless process.num_supported_hardware_watchpoints.positive?

      thread = process.selected_thread
      frame = thread.selected_frame
      var = frame.find_variable('x')
      skip 'Variable x not found' if var.nil?

      address = var.load_address
      skip 'Could not get load address' if address.zero?

      watchpoint = target.watch_address(address, 4, write: true)
      expect(watchpoint).to be_enabled
    end
  end

  describe '#enable and #disable' do
    it 'can enable and disable the watchpoint' do
      skip 'Watchpoints not supported on this platform' unless process.num_supported_hardware_watchpoints.positive?

      thread = process.selected_thread
      frame = thread.selected_frame
      var = frame.find_variable('x')
      skip 'Variable x not found' if var.nil?

      address = var.load_address
      skip 'Could not get load address' if address.zero?

      watchpoint = target.watch_address(address, 4, write: true)

      watchpoint.disable
      expect(watchpoint).not_to be_enabled

      watchpoint.enable
      expect(watchpoint).to be_enabled
    end
  end

  describe '#watch_address and #watch_size' do
    it 'returns the watch address and size' do
      skip 'Watchpoints not supported on this platform' unless process.num_supported_hardware_watchpoints.positive?

      thread = process.selected_thread
      frame = thread.selected_frame
      var = frame.find_variable('x')
      skip 'Variable x not found' if var.nil?

      address = var.load_address
      skip 'Could not get load address' if address.zero?

      watchpoint = target.watch_address(address, 4, write: true)

      expect(watchpoint.watch_address).to eq(address)
      expect(watchpoint.watch_size).to eq(4)
    end
  end

  describe '#delete' do
    it 'deletes the watchpoint from the target' do
      skip 'Watchpoints not supported on this platform' unless process.num_supported_hardware_watchpoints.positive?

      thread = process.selected_thread
      frame = thread.selected_frame
      var = frame.find_variable('x')
      skip 'Variable x not found' if var.nil?

      address = var.load_address
      skip 'Could not get load address' if address.zero?

      watchpoint = target.watch_address(address, 4, write: true)
      id = watchpoint.id
      watchpoint.delete

      expect(target.find_watchpoint_by_id(id)).to be_nil
    end
  end
end
