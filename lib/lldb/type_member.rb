# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class TypeMember
    prepend NativeLifecycle

    # @rbs context: Context?
    # @rbs return: void
    def initialize(ptr, context: nil)
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_type_member_destroy(released) },
        context: context
      )
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_type_member_is_valid(@ptr) != 0
    end

    # @rbs return: String?
    def name
      return nil unless valid?

      FFIBindings.lldb_type_member_get_name(@ptr)
    end

    # @rbs return: Type?
    def type
      return nil unless valid?

      ptr = FFIBindings.lldb_type_member_get_type(@ptr)
      return nil if ptr.nil? || ptr.null?

      Type.new(ptr, context: context)
    end

    # @rbs return: Integer
    def offset_in_bytes
      return 0 unless valid?

      FFIBindings.lldb_type_member_get_offset_in_bytes(@ptr)
    end

    # @rbs return: Integer
    def offset_in_bits
      return 0 unless valid?

      FFIBindings.lldb_type_member_get_offset_in_bits(@ptr)
    end

    # @rbs return: Integer
    def bitfield_size_in_bits
      return 0 unless valid?

      FFIBindings.lldb_type_member_get_bitfield_size_in_bits(@ptr)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
