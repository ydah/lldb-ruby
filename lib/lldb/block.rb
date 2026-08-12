# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Block
    prepend NativeLifecycle

    # @rbs target: Target?
    # @rbs context: Context?
    # @rbs return: void
    def initialize(ptr, target: nil, context: nil)
      @target = target
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_block_destroy(released) },
        context: context || target&.context
      )
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_block_is_valid(@ptr) != 0
    end

    # @rbs return: String?
    def inlined_name
      return nil unless valid?

      FFIBindings.lldb_block_get_inlined_name(@ptr)
    end

    # @rbs return: FileSpec?
    def inlined_call_site_file
      return nil unless valid?

      ptr = FFIBindings.lldb_block_get_inlined_call_site_file(@ptr)
      return nil if ptr.nil? || ptr.null?

      FileSpec.from_ptr(ptr, context: context)
    end

    # @rbs return: Integer
    def inlined_call_site_line
      return INVALID_LINE_NUMBER unless valid?

      FFIBindings.lldb_block_get_inlined_call_site_line(@ptr)
    end

    # @rbs return: Integer
    def inlined_call_site_column
      return 0 unless valid?

      FFIBindings.lldb_block_get_inlined_call_site_column(@ptr)
    end

    # @rbs return: Block?
    def parent
      child_block(:lldb_block_get_parent)
    end

    # @rbs return: Block?
    def sibling
      child_block(:lldb_block_get_sibling)
    end

    # @rbs return: Block?
    def first_child
      child_block(:lldb_block_get_first_child)
    end

    # @rbs return: Integer
    def num_ranges
      return 0 unless valid?

      FFIBindings.lldb_block_get_num_ranges(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: Address?
    def range_start_address(index)
      range_address(:lldb_block_get_range_start_address, index)
    end

    # @rbs index: Integer
    # @rbs return: Address?
    def range_end_address(index)
      range_address(:lldb_block_get_range_end_address, index)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end

    private

    # @rbs method_name: ::Symbol
    # @rbs return: Block?
    def child_block(method_name)
      return nil unless valid?

      ptr = FFIBindings.public_send(method_name, @ptr)
      return nil if ptr.nil? || ptr.null?

      Block.new(ptr, target: @target, context: context)
    end

    # @rbs method_name: ::Symbol
    # @rbs index: Integer
    # @rbs return: Address?
    def range_address(method_name, index)
      return nil unless valid?

      ptr = FFIBindings.public_send(method_name, @ptr, index)
      return nil if ptr.nil? || ptr.null?

      Address.from_ptr(ptr, target: @target, context: context)
    end
  end
end
