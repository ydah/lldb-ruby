# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Module
    # @rbs return: Target?
    attr_reader :target

    # @rbs ptr: FFI::Pointer
    # @rbs target: Target?
    # @rbs return: void
    def initialize(ptr, target:)
      @ptr = ptr # : FFI::Pointer
      @target = target
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
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

      FFIBindings.lldb_module_get_file_path(@ptr)
    end

    # @rbs return: String?
    def platform_file_path
      return nil unless valid?

      FFIBindings.lldb_module_get_platform_file_path(@ptr)
    end

    # @rbs return: Integer
    def num_symbols
      return 0 unless valid?

      FFIBindings.lldb_module_get_num_symbols(@ptr)
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
