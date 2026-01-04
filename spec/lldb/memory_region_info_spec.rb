# frozen_string_literal: true

RSpec.describe LLDB::MemoryRegionInfo do
  # Note: Full integration tests require a running process
  # These tests verify the class structure and basic behavior

  describe 'class structure' do
    it 'is defined in LLDB module' do
      expect(defined?(LLDB::MemoryRegionInfo)).to eq('constant')
    end

    it 'has expected instance methods' do
      expect(LLDB::MemoryRegionInfo.instance_methods(false)).to include(
        :base_address,
        :end_address,
        :size,
        :readable?,
        :writable?,
        :executable?,
        :mapped?,
        :name,
        :permissions,
        :to_s,
        :to_ptr
      )
    end
  end
end
