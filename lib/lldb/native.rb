# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  module Native
    # @rbs status: Integer
    # @rbs operation: String
    # @rbs error: Error?
    # @rbs return: bool
    def self.check_status!(status, operation, error = nil)
      return true if status == NativeStatus::OK

      if status == NativeStatus::INTERNAL_ERROR
        message = FFIBindings.lldb_wrapper_last_error_message || 'unknown native exception'
        code = FFIBindings.lldb_wrapper_last_error_code
        raise InternalBindingError.new(operation, message, code)
      end

      native_error = error || Error.new
      unless native_error.fail?
        native_error.set_error("native operation failed with status #{status}")
      end
      raise OperationError.new(operation, native_error)
    end

    # @rbs operation: String
    # @rbs error: Error
    # @rbs return: true
    def self.check_error!(operation, error)
      raise OperationError.new(operation, error) if error.fail?

      true
    end
  end
end
