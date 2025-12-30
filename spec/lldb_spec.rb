# frozen_string_literal: true

RSpec.describe LLDB do
  describe 'VERSION' do
    it 'has a version number' do
      expect(LLDB::VERSION).not_to be_nil
    end
  end

  describe '.initialize' do
    it 'initializes the LLDB library' do
      # Already initialized in spec_helper
      expect(LLDB.initialized?).to be true
    end
  end

  describe '.create_debugger' do
    it 'creates a valid debugger' do
      debugger = LLDB.create_debugger
      expect(debugger).to be_a(LLDB::Debugger)
      expect(debugger).to be_valid
    end
  end
end
