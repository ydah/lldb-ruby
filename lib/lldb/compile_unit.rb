# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class CompileUnit
    prepend NativeLifecycle

    # @rbs target: Target?
    # @rbs context: Context?
    # @rbs return: void
    def initialize(ptr, target: nil, context: nil)
      @target = target
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_compile_unit_destroy(released) },
        context: context || target&.context
      )
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_compile_unit_is_valid(@ptr) != 0
    end

    # @rbs return: FileSpec?
    def file_spec
      return nil unless valid?

      ptr = FFIBindings.lldb_compile_unit_get_file_spec(@ptr)
      return nil if ptr.nil? || ptr.null?

      FileSpec.from_ptr(ptr, context: context)
    end

    # @rbs return: Integer
    def num_line_entries
      return 0 unless valid?

      FFIBindings.lldb_compile_unit_get_num_line_entries(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: LineEntry?
    def line_entry_at_index(index)
      return nil unless valid?

      ptr = FFIBindings.lldb_compile_unit_get_line_entry_at_index(@ptr, index)
      return nil if ptr.nil? || ptr.null?

      LineEntry.new(ptr, target: @target, context: context)
    end

    # @rbs return: Array[LineEntry]
    def line_entries
      (0...num_line_entries).map { |index| line_entry_at_index(index) }.compact
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
