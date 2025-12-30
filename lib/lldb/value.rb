# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Value
    include Enumerable #[Value]

    # @rbs return: Frame | Value | Target
    attr_reader :parent

    # @rbs ptr: FFI::Pointer
    # @rbs parent: Frame | Value | Target
    # @rbs return: void
    def initialize(ptr, parent:)
      @ptr = ptr # : FFI::Pointer
      @parent = parent
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_value_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_value_is_valid(@ptr) != 0
    end

    # @rbs return: String?
    def name
      return nil unless valid?

      FFIBindings.lldb_value_get_name(@ptr)
    end

    # @rbs return: String?
    def value
      return nil unless valid?

      FFIBindings.lldb_value_get_value(@ptr)
    end

    # @rbs return: String?
    def summary
      return nil unless valid?

      FFIBindings.lldb_value_get_summary(@ptr)
    end

    # @rbs return: String?
    def type_name
      return nil unless valid?

      FFIBindings.lldb_value_get_type_name(@ptr)
    end

    # @rbs return: Integer
    def num_children
      return 0 unless valid?

      FFIBindings.lldb_value_get_num_children(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: Value?
    def child_at_index(index)
      raise InvalidObjectError, 'Value is not valid' unless valid?

      child_ptr = FFIBindings.lldb_value_get_child_at_index(@ptr, index)
      return nil if child_ptr.nil? || child_ptr.null?

      Value.new(child_ptr, parent: self)
    end

    # @rbs name: String
    # @rbs return: Value?
    def child_member(name)
      raise InvalidObjectError, 'Value is not valid' unless valid?

      child_ptr = FFIBindings.lldb_value_get_child_member_with_name(@ptr, name)
      return nil if child_ptr.nil? || child_ptr.null?

      Value.new(child_ptr, parent: self)
    end

    alias [] child_member

    # @rbs return: Array[Value]
    def children
      (0...num_children).map { |i| child_at_index(i) }.compact
    end

    # @rbs &block: (Value) -> void
    # @rbs return: Enumerator[Value, void] | void
    def each(&block)
      return enum_for(:each) unless block_given?

      children.each(&block)
    end

    # @rbs return: Integer
    def value_as_signed
      return 0 unless valid?

      FFIBindings.lldb_value_get_value_as_signed(@ptr)
    end

    alias to_i value_as_signed

    # @rbs return: Integer
    def value_as_unsigned
      return 0 unless valid?

      FFIBindings.lldb_value_get_value_as_unsigned(@ptr)
    end

    # @rbs return: Integer
    def byte_size
      return 0 unless valid?

      FFIBindings.lldb_value_get_byte_size(@ptr)
    end

    # @rbs return: bool
    def might_have_children?
      return false unless valid?

      FFIBindings.lldb_value_might_have_children(@ptr) != 0
    end

    # @rbs return: Error
    def error
      raise InvalidObjectError, 'Value is not valid' unless valid?

      error = Error.new
      FFIBindings.lldb_value_get_error(@ptr, error.to_ptr)
      error
    end

    # @rbs return: bool
    def has_error?
      err = error
      err.fail?
    end

    # @rbs return: Value?
    def dereference
      raise InvalidObjectError, 'Value is not valid' unless valid?

      deref_ptr = FFIBindings.lldb_value_dereference(@ptr)
      return nil if deref_ptr.nil? || deref_ptr.null?

      Value.new(deref_ptr, parent: self)
    end

    # @rbs return: Value?
    def address_of
      raise InvalidObjectError, 'Value is not valid' unless valid?

      addr_ptr = FFIBindings.lldb_value_address_of(@ptr)
      return nil if addr_ptr.nil? || addr_ptr.null?

      Value.new(addr_ptr, parent: self)
    end

    # @rbs return: String
    def to_s
      if summary
        "#{name} = #{summary}"
      elsif value
        "#{name} = #{value}"
      else
        "#{name} (#{type_name})"
      end
    end

    # @rbs return: String
    def inspect
      "#<LLDB::Value name=#{name.inspect} type=#{type_name.inspect} value=#{value.inspect}>"
    end

    # @rbs return: Type?
    def type
      raise InvalidObjectError, 'Value is not valid' unless valid?

      type_ptr = FFIBindings.lldb_value_get_type(@ptr)
      return nil if type_ptr.nil? || type_ptr.null?

      Type.new(type_ptr)
    end

    # @rbs target_type: Type
    # @rbs return: Value?
    def cast(target_type)
      raise InvalidObjectError, 'Value is not valid' unless valid?

      cast_ptr = FFIBindings.lldb_value_cast(@ptr, target_type.to_ptr)
      return nil if cast_ptr.nil? || cast_ptr.null?

      Value.new(cast_ptr, parent: self)
    end

    # @rbs return: Integer
    def load_address
      return 0 unless valid?

      FFIBindings.lldb_value_get_load_address(@ptr)
    end

    # @rbs return: Integer
    def value_type
      return ValueType::INVALID unless valid?

      FFIBindings.lldb_value_get_value_type(@ptr)
    end

    # @rbs return: String
    def value_type_name
      ValueType.name(value_type)
    end

    # @rbs str: String
    # @rbs return: bool
    def set_value_from_cstring(str)
      raise InvalidObjectError, 'Value is not valid' unless valid?

      error = Error.new
      result = FFIBindings.lldb_value_set_value_from_cstring(@ptr, str, error.to_ptr)
      error.raise_if_error!
      result != 0
    end

    # @rbs name: String
    # @rbs value_type: Type
    # @rbs offset: Integer
    # @rbs return: Value?
    def create_child_at_offset(name, value_type, offset)
      raise InvalidObjectError, 'Value is not valid' unless valid?

      child_ptr = FFIBindings.lldb_value_create_child_at_offset(@ptr, name, value_type.to_ptr, offset)
      return nil if child_ptr.nil? || child_ptr.null?

      Value.new(child_ptr, parent: self)
    end

    # @rbs name: String
    # @rbs address: Integer
    # @rbs value_type: Type
    # @rbs return: Value?
    def create_value_from_address(name, address, value_type)
      raise InvalidObjectError, 'Value is not valid' unless valid?

      value_ptr = FFIBindings.lldb_value_create_value_from_address(@ptr, name, address, value_type.to_ptr)
      return nil if value_ptr.nil? || value_ptr.null?

      Value.new(value_ptr, parent: self)
    end

    # @rbs name: String
    # @rbs expression: String
    # @rbs return: Value?
    def create_value_from_expression(name, expression)
      raise InvalidObjectError, 'Value is not valid' unless valid?

      value_ptr = FFIBindings.lldb_value_create_value_from_expression(@ptr, name, expression)
      return nil if value_ptr.nil? || value_ptr.null?

      Value.new(value_ptr, parent: self)
    end

    # @rbs resolve: bool
    # @rbs read: bool
    # @rbs write: bool
    # @rbs return: Watchpoint?
    def watch(resolve: true, read: false, write: true)
      raise InvalidObjectError, 'Value is not valid' unless valid?

      error = Error.new
      wp_ptr = FFIBindings.lldb_value_watch(@ptr, resolve ? 1 : 0, read ? 1 : 0, write ? 1 : 0, error.to_ptr)

      error.raise_if_error!
      return nil if wp_ptr.nil? || wp_ptr.null?

      target = get_target_from_parent
      Watchpoint.new(wp_ptr, target: target)
    end

    # @rbs return: String?
    def expression_path
      return nil unless valid?

      FFIBindings.lldb_value_get_expression_path(@ptr)
    end

    # @rbs return: bool
    def pointer_type?
      return false unless valid?

      FFIBindings.lldb_value_is_pointer_type(@ptr) != 0
    end

    # @rbs return: Value?
    def non_synthetic_value
      raise InvalidObjectError, 'Value is not valid' unless valid?

      value_ptr = FFIBindings.lldb_value_get_non_synthetic_value(@ptr)
      return nil if value_ptr.nil? || value_ptr.null?

      Value.new(value_ptr, parent: self)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end

    private

    # @rbs return: Target
    def get_target_from_parent
      current = @parent
      while current
        return current.target if current.respond_to?(:target)
        return current if current.is_a?(Target)

        current = current.respond_to?(:parent) ? current.parent : nil
      end
      raise LLDBError, 'Could not find target from value hierarchy'
    end
  end
end
