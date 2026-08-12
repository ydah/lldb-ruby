# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class NativeStringArray
    # @rbs strings: Array[String]
    # @rbs return: void
    def initialize(strings)
      @strings = strings.map { |string| NativeStringArray.validate!(string) }
      @string_pointers = @strings.map { |string| FFI::MemoryPointer.from_string(string) }
      @pointer_array = FFI::MemoryPointer.new(:pointer, @string_pointers.length + 1)

      @string_pointers.each_with_index do |pointer, index|
        @pointer_array[index].put_pointer(0, pointer)
      end
      @pointer_array[@string_pointers.length].put_pointer(0, nil)
    end

    # @rbs value: String
    # @rbs return: String
    def self.validate!(value)
      raise ArgumentError, 'native string values must be String' unless value.is_a?(String)
      raise ArgumentError, 'native strings must not contain NUL bytes' if value.include?("\0")

      value
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @pointer_array
    end
  end
end
