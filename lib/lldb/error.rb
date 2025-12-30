# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class LLDBError < StandardError; end
  class InvalidObjectError < LLDBError; end
  class LaunchError < LLDBError; end
  class AttachError < LLDBError; end
  class BreakpointError < LLDBError; end
  class EvaluationError < LLDBError; end

  class Error
    # @rbs ptr: FFI::Pointer?
    # @rbs return: void
    def initialize(ptr = nil)
      @ptr = ptr || FFIBindings.lldb_error_create
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_error_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def success?
      FFIBindings.lldb_error_success(@ptr) != 0
    end

    # @rbs return: bool
    def fail?
      FFIBindings.lldb_error_fail(@ptr) != 0
    end

    # @rbs return: String
    def message
      FFIBindings.lldb_error_get_cstring(@ptr) || ''
    end

    alias to_s message

    # @rbs return: Integer
    def error_code
      FFIBindings.lldb_error_get_error(@ptr)
    end

    # @rbs return: void
    def clear
      FFIBindings.lldb_error_clear(@ptr)
    end

    # @rbs message: String
    # @rbs return: void
    def set_error(message)
      FFIBindings.lldb_error_set_error_string(@ptr, message)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end

    # @rbs return: void
    def raise_if_error!
      raise LLDBError, message if fail?
    end
  end
end
