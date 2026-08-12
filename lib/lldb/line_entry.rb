# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class LineEntry
    prepend NativeLifecycle

    # @rbs target: Target?
    # @rbs context: Context?
    # @rbs return: void
    def initialize(ptr, target: nil, context: nil)
      @target = target
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_line_entry_destroy(released) },
        context: context || target&.context
      )
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_line_entry_is_valid(@ptr) != 0
    end

    # @rbs return: Address?
    def start_address
      return nil unless valid?

      ptr = FFIBindings.lldb_line_entry_get_start_address(@ptr)
      return nil if ptr.nil? || ptr.null?

      Address.from_ptr(ptr, target: @target, context: context)
    end

    # @rbs return: Address?
    def end_address
      return nil unless valid?

      ptr = FFIBindings.lldb_line_entry_get_end_address(@ptr)
      return nil if ptr.nil? || ptr.null?

      Address.from_ptr(ptr, target: @target, context: context)
    end

    # @rbs return: FileSpec?
    def file_spec
      return nil unless valid?

      ptr = FFIBindings.lldb_line_entry_get_file_spec(@ptr)
      return nil if ptr.nil? || ptr.null?

      FileSpec.from_ptr(ptr, context: context)
    end

    # @rbs return: Integer
    def line
      return INVALID_LINE_NUMBER unless valid?

      FFIBindings.lldb_line_entry_get_line(@ptr)
    end

    # @rbs return: Integer
    def column
      return 0 unless valid?

      FFIBindings.lldb_line_entry_get_column(@ptr)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
