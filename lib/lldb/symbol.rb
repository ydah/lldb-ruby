# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Symbol
    prepend NativeLifecycle

    # @rbs target: Target?
    # @rbs context: Context?
    # @rbs return: void
    def initialize(ptr, target: nil, context: nil)
      @target = target
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_symbol_destroy(released) },
        context: context || target&.context
      )
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_symbol_is_valid(@ptr) != 0
    end

    # @rbs return: String?
    def name
      return nil unless valid?

      FFIBindings.lldb_symbol_get_name(@ptr)
    end

    # @rbs return: String?
    def display_name
      return nil unless valid?

      FFIBindings.lldb_symbol_get_display_name(@ptr)
    end

    # @rbs return: String?
    def mangled_name
      return nil unless valid?

      FFIBindings.lldb_symbol_get_mangled_name(@ptr)
    end

    # @rbs return: String?
    def base_name
      return nil unless valid?

      FFIBindings.lldb_symbol_get_base_name(@ptr)
    end

    # @rbs return: Address?
    def start_address
      address_at(:lldb_symbol_get_start_address)
    end

    # @rbs return: Address?
    def end_address
      address_at(:lldb_symbol_get_end_address)
    end

    # @rbs return: Integer
    def value
      return INVALID_ADDRESS unless valid?

      FFIBindings.lldb_symbol_get_value(@ptr)
    end

    # @rbs return: Integer
    def size
      return 0 unless valid?

      FFIBindings.lldb_symbol_get_size(@ptr)
    end

    # @rbs return: Integer
    def prologue_byte_size
      return 0 unless valid?

      FFIBindings.lldb_symbol_get_prologue_byte_size(@ptr)
    end

    # @rbs return: Integer
    def type
      return 0 unless valid?

      FFIBindings.lldb_symbol_get_type(@ptr)
    end

    # @rbs return: Integer
    def id
      return 0 unless valid?

      FFIBindings.lldb_symbol_get_id(@ptr)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end

    private

    # @rbs method_name: ::Symbol
    # @rbs return: Address?
    def address_at(method_name)
      return nil unless valid?

      ptr = FFIBindings.public_send(method_name, @ptr)
      return nil if ptr.nil? || ptr.null?

      Address.from_ptr(ptr, target: @target, context: context)
    end
  end
end
