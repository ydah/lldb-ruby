# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class InstructionList
    prepend NativeLifecycle
    include Enumerable #[Instruction]

    # @rbs target: Target?
    # @rbs context: Context?
    # @rbs return: void
    def initialize(ptr, target: nil, context: nil)
      @target = target
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_instruction_list_destroy(released) },
        context: context || target&.context
      )
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_instruction_list_is_valid(@ptr) != 0
    end

    # @rbs return: Integer
    def size
      return 0 unless valid?

      FFIBindings.lldb_instruction_list_get_size(@ptr)
    end

    alias length size

    # @rbs index: Integer
    # @rbs return: Instruction?
    def [](index)
      return nil unless valid?

      ptr = FFIBindings.lldb_instruction_list_get_instruction_at_index(@ptr, index)
      return nil if ptr.nil? || ptr.null?

      Instruction.new(ptr, target: @target, context: context)
    end

    # @rbs &block: (Instruction) -> void
    # @rbs return: Enumerator[Instruction, void] | void
    def each(&block)
      return enum_for(:each) unless block_given?

      (0...size).each do |index|
        instruction = self[index]
        block.call(instruction) if instruction
      end
    end

    # @rbs return: Array[Instruction]
    def to_a
      (0...size).map { |index| self[index] }.compact
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
