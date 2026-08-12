# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Module
    prepend NativeLifecycle

    # @rbs return: Target?
    attr_reader :target

    # @rbs ptr: FFI::Pointer
    # @rbs target: Target?
    # @rbs return: void
    def initialize(ptr, target:, context: target&.context)
      @target = target
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_module_destroy(released) },
        context: context
      )
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_module_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_module_is_valid(@ptr) != 0
    end

    # @rbs return: String?
    def file_path
      return nil unless valid?

      file&.path
    end

    # @rbs return: String?
    def platform_file_path
      return nil unless valid?

      platform_file&.path
    end

    # @rbs return: FileSpec?
    def file
      return nil unless valid?

      file_ptr = FFIBindings.lldb_module_get_file(@ptr)
      return nil if file_ptr.nil? || file_ptr.null?

      FileSpec.from_ptr(file_ptr, context: context)
    end

    # @rbs return: FileSpec?
    def platform_file
      return nil unless valid?

      file_ptr = FFIBindings.lldb_module_get_platform_file(@ptr)
      return nil if file_ptr.nil? || file_ptr.null?

      FileSpec.from_ptr(file_ptr, context: context)
    end

    # @rbs return: Integer
    def num_symbols
      return 0 unless valid?

      FFIBindings.lldb_module_get_num_symbols(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: Symbol?
    def symbol_at_index(index)
      raise InvalidObjectError, 'Module is not valid' unless valid?

      symbol_ptr = FFIBindings.lldb_module_get_symbol_at_index(@ptr, index)
      return nil if symbol_ptr.nil? || symbol_ptr.null?

      Symbol.new(symbol_ptr, target: @target, context: context)
    end

    # @rbs return: Array[Symbol]
    def symbols
      (0...num_symbols).map { |index| symbol_at_index(index) }.compact
    end

    # @rbs return: String
    def to_s
      file_path || '(unknown module)'
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
