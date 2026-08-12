# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class CommandReturnObject
    prepend NativeLifecycle
    # @rbs return: CommandReturnObject
    def self.create(context: nil)
      ptr = FFIBindings.lldb_command_return_object_create
      raise LLDBError, 'Failed to create command return object' if ptr.nil? || ptr.null?

      new(ptr, context: context)
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: void
    def initialize(ptr, context: nil)
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_command_return_object_destroy(released) },
        context: context
      )
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_command_return_object_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_command_return_object_is_valid(@ptr) != 0
    end

    # @rbs return: String?
    def output
      return nil unless valid?

      FFIBindings.lldb_command_return_object_get_output(@ptr)
    end

    # @rbs return: String?
    def error
      return nil unless valid?

      FFIBindings.lldb_command_return_object_get_error(@ptr)
    end

    # @rbs return: Integer
    # @rbs return: Integer
    def status
      return ReturnStatus::INVALID unless valid?

      FFIBindings.lldb_command_return_object_get_status(@ptr)
    end

    # @rbs return: bool
    def succeeded?
      return false unless valid?

      FFIBindings.lldb_command_return_object_succeeded(@ptr) != 0
    end

    # @rbs return: bool
    def has_result?
      return false unless valid?

      FFIBindings.lldb_command_return_object_has_result(@ptr) != 0
    end

    # @rbs return: void
    def clear
      raise InvalidObjectError, 'CommandReturnObject is not valid' unless valid?

      FFIBindings.lldb_command_return_object_clear(@ptr)
    end

    # @rbs return: String
    def to_s
      succeeded? ? (output || '') : (error || '')
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
