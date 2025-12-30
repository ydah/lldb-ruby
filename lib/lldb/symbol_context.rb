# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class SymbolContext
    # @rbs return: Frame
    attr_reader :parent

    # @rbs ptr: FFI::Pointer
    # @rbs parent: Frame
    # @rbs return: void
    def initialize(ptr, parent:)
      @ptr = ptr # : FFI::Pointer
      @parent = parent
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_symbol_context_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_symbol_context_is_valid(@ptr) != 0
    end

    # @rbs return: Module?
    def module
      return nil unless valid?

      mod_ptr = FFIBindings.lldb_symbol_context_get_module(@ptr)
      return nil if mod_ptr.nil? || mod_ptr.null?

      Module.new(mod_ptr, target: nil)
    end

    # @rbs return: String?
    def function_name
      return nil unless valid?

      FFIBindings.lldb_symbol_context_get_function_name(@ptr)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
