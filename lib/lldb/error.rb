# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class LLDBError < StandardError; end
  class InvalidObjectError < LLDBError; end
  class LaunchError < LLDBError; end
  class AttachError < LLDBError; end
  class BreakpointError < LLDBError; end
  class EvaluationError < LLDBError; end
  class UnsupportedAPIError < LLDBError; end
  class UnsupportedPlatformError < LLDBError; end
  class IncompatibleWrapperError < LLDBError; end

  class OperationError < LLDBError
    attr_reader :operation, :error, :code, :error_type

    # @rbs operation: String
    # @rbs error: Error
    # @rbs return: void
    def initialize(operation, error)
      @operation = operation
      @error = error
      @code = error.code
      @error_type = error.type
      super("LLDB operation '#{operation}' failed (type=#{error.type_name}, code=#{error.code}): #{error.message}")
    end
  end

  class InternalBindingError < LLDBError
    attr_reader :operation, :native_message, :native_code

    # @rbs operation: String
    # @rbs native_message: String
    # @rbs native_code: Integer
    # @rbs return: void
    def initialize(operation, native_message, native_code)
      @operation = operation
      @native_message = native_message
      @native_code = native_code
      super("Native wrapper operation '#{operation}' failed (code=#{native_code}): #{native_message}")
    end
  end

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
    def code
      FFIBindings.lldb_error_get_error(@ptr)
    end

    alias error_code code

    # @rbs return: Integer
    def type
      FFIBindings.lldb_error_get_type(@ptr)
    end

    # @rbs return: String
    def type_name
      ErrorType::NAMES.fetch(type, 'unknown')
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

    # @rbs operation: String
    # @rbs return: void
    def raise_if_error!(operation = 'native operation')
      raise OperationError.new(operation, self) if fail?
    end
  end
end
