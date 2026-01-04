# frozen_string_literal: true

RSpec.describe LLDB::APISupport do
  describe '.method_available?' do
    it 'returns true for existing FFI methods' do
      expect(LLDB::APISupport.method_available?(:lldb_debugger_create)).to be true
    end

    it 'returns false for non-existing methods' do
      expect(LLDB::APISupport.method_available?(:non_existing_method_12345)).to be false
    end
  end

  describe '.require_method!' do
    it 'does not raise for existing methods' do
      expect { LLDB::APISupport.require_method!(:lldb_debugger_create) }.not_to raise_error
    end

    it 'raises UnsupportedAPIError for non-existing methods' do
      expect {
        LLDB::APISupport.require_method!(:non_existing_method, 'test_feature')
      }.to raise_error(LLDB::UnsupportedAPIError, /test_feature/)
    end

    it 'uses method name as feature name when not specified' do
      expect {
        LLDB::APISupport.require_method!(:non_existing_method)
      }.to raise_error(LLDB::UnsupportedAPIError, /non_existing_method/)
    end
  end

  describe '.feature_supported?' do
    it 'returns true for supported features' do
      # These features should be available in our FFI bindings
      expect(LLDB::APISupport.feature_supported?(:step_into)).to be true
      expect(LLDB::APISupport.feature_supported?(:step_over)).to be true
      expect(LLDB::APISupport.feature_supported?(:memory_read)).to be true
    end

    it 'returns false for unknown features' do
      expect(LLDB::APISupport.feature_supported?(:unknown_feature)).to be false
    end
  end

  describe '.supported_features' do
    it 'returns an array of supported feature symbols' do
      features = LLDB::APISupport.supported_features
      expect(features).to be_an(Array)
      expect(features).to include(:step_into)
      expect(features).to include(:memory_read)
    end
  end

  describe '.unsupported_features' do
    it 'returns an array of unsupported feature symbols' do
      features = LLDB::APISupport.unsupported_features
      expect(features).to be_an(Array)
    end
  end

  describe 'FEATURES constant' do
    it 'contains expected feature mappings' do
      expect(LLDB::APISupport::FEATURES).to include(
        breakpoint_by_address: :lldb_target_breakpoint_create_by_address,
        step_into: :lldb_thread_step_into,
        memory_read: :lldb_process_read_memory,
        evaluate_expression: :lldb_frame_evaluate_expression
      )
    end

    it 'is frozen' do
      expect(LLDB::APISupport::FEATURES).to be_frozen
    end
  end
end
