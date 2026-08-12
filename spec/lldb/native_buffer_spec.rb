# frozen_string_literal: true

RSpec.describe LLDB::NativeBuffer do
  describe '.read_c_string' do
    it 'grows when the native API reports a truncated buffer' do
      value = 'x' * 700

      result = described_class.read_c_string do |buffer, length|
        next 0 if buffer.nil?

        written = [value.bytesize, length - 1].min
        buffer.put_bytes(0, value, 0, written)
        buffer.put_uint8(written, 0)
        written
      end

      expect(result).to eq(value)
    end

    it 'honors a zero maximum size' do
      expect(described_class.read_c_string(max_size: 0) { raise 'not called' }).to eq('')
    end

    it 'rejects a negative maximum size' do
      expect do
        described_class.read_c_string(max_size: -1) { 0 }
      end.to raise_error(ArgumentError)
    end
  end
end
