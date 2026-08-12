# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  module NativeBuffer
    # @rbs max_size: Integer?
    # @rbs &reader: (FFI::Pointer?, Integer) -> Integer
    # @rbs return: String
    def self.read_c_string(max_size: nil, &reader)
      raise ArgumentError, 'max_size must be non-negative' if max_size&.negative?
      return '' if max_size == 0

      required = reader.call(nil, 0)
      capacity = if required.positive?
                   max_size ? [required + 1, max_size].min : required + 1
                 else
                   max_size || 256
                 end
      capacity = 1 if capacity.zero?
      loop do
        buffer = FFI::MemoryPointer.new(:uint8, capacity)
        written = reader.call(buffer, capacity)
        bytes = buffer.get_bytes(0, [written, capacity].min).delete_suffix("\0")
        return bytes if max_size && (capacity >= max_size || written < capacity - 1)
        return bytes if written < capacity - 1

        capacity = [capacity * 2, written + 2].max
      end
    end
  end
end
