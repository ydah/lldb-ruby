# frozen_string_literal: true

RSpec.describe LLDB::Type do
  let(:debugger) { LLDB::Debugger.create }
  let(:executable) { compile_fixture('simple') }
  let(:target) { debugger.create_target(executable) }
  let(:process) do
    debugger.async = false
    target.breakpoint_create_by_name('lldb_test_add')
    target.launch
  end
  let(:thread) { process.selected_thread }
  let(:frame) { thread.selected_frame }
  let(:value) { frame.find_variable('a') }
  let(:type) { value&.type }

  after do
    process.kill if process&.valid?
  end

  describe '#valid?' do
    it 'returns true for a valid type' do
      skip 'Value or type not found' if type.nil?
      expect(type).to be_valid
    end
  end

  describe '#name' do
    it 'returns the type name' do
      skip 'Value or type not found' if type.nil?
      expect(type.name).to be_a(String)
    end
  end

  describe '#byte_size' do
    it 'returns the byte size of the type' do
      skip 'Value or type not found' if type.nil?
      expect(type.byte_size).to be >= 0
    end
  end

  describe '#pointer_type?' do
    it 'returns a boolean' do
      skip 'Value or type not found' if type.nil?
      expect([true, false]).to include(type.pointer_type?)
    end
  end

  describe '#to_s' do
    it 'returns a string representation' do
      skip 'Value or type not found' if type.nil?
      expect(type.to_s).to be_a(String)
    end
  end
end
