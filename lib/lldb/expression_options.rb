# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  module DynamicValue
    NO_DYNAMIC = 0
    CAN_RUN_TARGET = 1
    DONT_RUN_TARGET = 2
  end

  class ExpressionOptions
    prepend NativeLifecycle

    # @rbs context: Context?
    # @rbs return: void
    def initialize(context: nil)
      ptr = FFIBindings.lldb_expression_options_create
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_expression_options_destroy(released) },
        context: context
      )
      @language = nil
    end

    # @rbs return: bool
    def valid?
      !@ptr.null?
    end

    # @rbs return: Integer
    def timeout
      FFIBindings.lldb_expression_options_get_timeout(@ptr)
    end

    # @rbs value: Integer
    # @rbs return: void
    def timeout=(value)
      FFIBindings.lldb_expression_options_set_timeout(@ptr, value)
    end

    # @rbs return: bool
    def unwind_on_error?
      FFIBindings.lldb_expression_options_get_unwind_on_error(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def unwind_on_error=(value)
      FFIBindings.lldb_expression_options_set_unwind_on_error(@ptr, value ? 1 : 0)
    end

    # @rbs return: bool
    def ignore_breakpoints?
      FFIBindings.lldb_expression_options_get_ignore_breakpoints(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def ignore_breakpoints=(value)
      FFIBindings.lldb_expression_options_set_ignore_breakpoints(@ptr, value ? 1 : 0)
    end

    # @rbs return: Integer
    def fetch_dynamic_value
      FFIBindings.lldb_expression_options_get_fetch_dynamic_value(@ptr)
    end

    # @rbs value: Integer
    # @rbs return: void
    def fetch_dynamic_value=(value)
      FFIBindings.lldb_expression_options_set_fetch_dynamic_value(@ptr, value)
    end

    # @rbs return: bool
    def try_all_threads?
      FFIBindings.lldb_expression_options_get_try_all_threads(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def try_all_threads=(value)
      FFIBindings.lldb_expression_options_set_try_all_threads(@ptr, value ? 1 : 0)
    end

    # @rbs return: bool
    def stop_others?
      FFIBindings.lldb_expression_options_get_stop_others(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def stop_others=(value)
      FFIBindings.lldb_expression_options_set_stop_others(@ptr, value ? 1 : 0)
    end

    # @rbs return: Integer?
    attr_reader :language

    # @rbs value: Integer
    # @rbs return: void
    def language=(value)
      @language = value
      FFIBindings.lldb_expression_options_set_language(@ptr, value)
    end

    # @rbs return: bool
    def suppress_persistent_result?
      FFIBindings.lldb_expression_options_get_suppress_persistent_result(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def suppress_persistent_result=(value)
      FFIBindings.lldb_expression_options_set_suppress_persistent_result(@ptr, value ? 1 : 0)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
