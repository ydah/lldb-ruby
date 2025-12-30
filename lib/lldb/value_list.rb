# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class ValueList
    include Enumerable #[Value]

    # @rbs return: Frame | Value
    attr_reader :parent

    # @rbs ptr: FFI::Pointer
    # @rbs parent: Frame | Value
    # @rbs return: void
    def initialize(ptr, parent:)
      @ptr = ptr # : FFI::Pointer
      @parent = parent
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_value_list_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_value_list_is_valid(@ptr) != 0
    end

    # @rbs return: Integer
    def size
      return 0 unless valid?

      FFIBindings.lldb_value_list_get_size(@ptr)
    end

    alias length size

    # @rbs index: Integer
    # @rbs return: Value?
    def value_at_index(index)
      raise InvalidObjectError, 'ValueList is not valid' unless valid?

      value_ptr = FFIBindings.lldb_value_list_get_value_at_index(@ptr, index)
      return nil if value_ptr.nil? || value_ptr.null?

      Value.new(value_ptr, parent: @parent)
    end

    alias [] value_at_index

    # @rbs name: String
    # @rbs return: Value?
    def first_value_by_name(name)
      raise InvalidObjectError, 'ValueList is not valid' unless valid?

      value_ptr = FFIBindings.lldb_value_list_get_first_value_by_name(@ptr, name)
      return nil if value_ptr.nil? || value_ptr.null?

      Value.new(value_ptr, parent: @parent)
    end

    # @rbs return: Array[Value]
    def to_a
      (0...size).map { |i| value_at_index(i) }.compact
    end

    # @rbs &block: (Value) -> void
    # @rbs return: Enumerator[Value, void] | void
    def each(&block)
      return enum_for(:each) unless block_given?

      to_a.each(&block)
    end

    # @rbs return: bool
    def empty?
      size.zero?
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
