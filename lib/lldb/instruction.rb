# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Instruction
    prepend NativeLifecycle

    # @rbs target: Target?
    # @rbs context: Context?
    # @rbs return: void
    def initialize(ptr, target: nil, context: nil)
      @target = target
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_instruction_destroy(released) },
        context: context || target&.context
      )
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_instruction_is_valid(@ptr) != 0
    end

    # @rbs return: Address?
    def address
      return nil unless valid?

      ptr = FFIBindings.lldb_instruction_get_address(@ptr)
      return nil if ptr.nil? || ptr.null?

      Address.from_ptr(ptr, target: @target, context: context)
    end

    # @rbs return: String?
    def mnemonic
      return nil unless valid? && @target

      FFIBindings.lldb_instruction_get_mnemonic(@ptr, @target.to_ptr)
    end

    # @rbs return: String?
    def operands
      return nil unless valid? && @target

      FFIBindings.lldb_instruction_get_operands(@ptr, @target.to_ptr)
    end

    # @rbs return: String?
    def comment
      return nil unless valid? && @target

      FFIBindings.lldb_instruction_get_comment(@ptr, @target.to_ptr)
    end

    # @rbs return: Integer
    def byte_size
      return 0 unless valid?

      FFIBindings.lldb_instruction_get_byte_size(@ptr)
    end

    # @rbs return: String
    def bytes
      return ''.b unless valid? && @target

      length = byte_size
      return ''.b if length.zero?

      buffer = FFI::MemoryPointer.new(:uint8, length)
      written = FFIBindings.lldb_instruction_get_bytes(
        @ptr, @target.to_ptr, buffer, length
      )
      buffer.get_bytes(0, [written, length].min).b
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
