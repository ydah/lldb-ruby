# frozen_string_literal: true

RSpec.describe LLDB::Error do
  describe '#initialize' do
    it 'creates a new error object' do
      error = LLDB::Error.new
      expect(error).not_to be_nil
    end
  end

  describe '#success?' do
    it 'returns true for a new error' do
      error = LLDB::Error.new
      expect(error).to be_success
    end
  end

  describe '#fail?' do
    it 'returns false for a new error' do
      error = LLDB::Error.new
      expect(error).not_to be_fail
    end
  end

  describe '#message' do
    it 'returns an empty string for a new error' do
      error = LLDB::Error.new
      expect(error.message).to eq('')
    end
  end

  describe '#set_error' do
    it 'sets an error message' do
      error = LLDB::Error.new
      error.set_error('Test error')
      expect(error).to be_fail
      expect(error.message).to eq('Test error')
    end
  end

  describe '#clear' do
    it 'clears the error' do
      error = LLDB::Error.new
      error.set_error('Test error')
      expect(error).to be_fail

      error.clear
      expect(error).to be_success
    end
  end

  describe '#raise_if_error!' do
    it 'does nothing for success' do
      error = LLDB::Error.new
      expect { error.raise_if_error! }.not_to raise_error
    end

    it 'raises LLDBError for failure' do
      error = LLDB::Error.new
      error.set_error('Test error')
      expect { error.raise_if_error! }.to raise_error(LLDB::LLDBError, 'Test error')
    end
  end
end
