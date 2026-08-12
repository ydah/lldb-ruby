# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Address
    prepend NativeLifecycle

    # @rbs target: Target?
    # @rbs context: Context?
    # @rbs pointer: FFI::Pointer?
    # @rbs return: void
    def initialize(target: nil, context: nil, pointer: nil, load_address: nil)
      if load_address
        raise ArgumentError, 'target is required for a load address' unless target.is_a?(Target)

        ptr = FFIBindings.lldb_address_create_from_load_address(load_address, target.to_ptr)
      else
        ptr = pointer || FFIBindings.lldb_address_create
      end
      @target = target
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_address_destroy(released) },
        context: context || target&.context
      )
    end

    # @rbs ptr: FFI::Pointer
    # @rbs target: Target?
    # @rbs context: Context?
    # @rbs return: Address
    def self.from_ptr(ptr, target: nil, context: nil)
      new(pointer: ptr, target: target, context: context)
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_address_is_valid(@ptr) != 0
    end

    # @rbs return: Integer
    def file_address
      return INVALID_ADDRESS unless valid?

      FFIBindings.lldb_address_get_file_address(@ptr)
    end

    # @rbs target: Target?
    # @rbs return: Integer
    def load_address(target: @target)
      return INVALID_ADDRESS unless valid?
      raise ArgumentError, 'target must be a Target' if target && !target.is_a?(Target)

      target_ptr = target ? target.to_ptr : NativeHandle::NULL_POINTER
      FFIBindings.lldb_address_get_load_address(@ptr, target_ptr)
    end

    # @rbs return: Integer
    def offset
      return 0 unless valid?

      FFIBindings.lldb_address_get_offset(@ptr)
    end

    # @rbs return: LineEntry?
    def line_entry
      return nil unless valid?

      ptr = FFIBindings.lldb_address_get_line_entry(@ptr)
      return nil if ptr.nil? || ptr.null?

      LineEntry.new(ptr, target: @target, context: context)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
