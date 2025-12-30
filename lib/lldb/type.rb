# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Type
    # @rbs ptr: FFI::Pointer
    # @rbs return: void
    def initialize(ptr)
      @ptr = ptr # : FFI::Pointer
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_type_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_type_is_valid(@ptr) != 0
    end

    # @rbs return: String?
    def name
      return nil unless valid?

      FFIBindings.lldb_type_get_name(@ptr)
    end

    # @rbs return: String?
    def display_type_name
      return nil unless valid?

      FFIBindings.lldb_type_get_display_type_name(@ptr)
    end

    # @rbs return: Integer
    def byte_size
      return 0 unless valid?

      FFIBindings.lldb_type_get_byte_size(@ptr)
    end

    # @rbs return: bool
    def pointer_type?
      return false unless valid?

      FFIBindings.lldb_type_is_pointer_type(@ptr) != 0
    end

    # @rbs return: bool
    def reference_type?
      return false unless valid?

      FFIBindings.lldb_type_is_reference_type(@ptr) != 0
    end

    # @rbs return: bool
    def array_type?
      return false unless valid?

      FFIBindings.lldb_type_is_array_type(@ptr) != 0
    end

    # @rbs return: bool
    def vector_type?
      return false unless valid?

      FFIBindings.lldb_type_is_vector_type(@ptr) != 0
    end

    # @rbs return: bool
    def typedef_type?
      return false unless valid?

      FFIBindings.lldb_type_is_typedef_type(@ptr) != 0
    end

    # @rbs return: bool
    def function_type?
      return false unless valid?

      FFIBindings.lldb_type_is_function_type(@ptr) != 0
    end

    # @rbs return: bool
    def polymorphic_class?
      return false unless valid?

      FFIBindings.lldb_type_is_polymorphic_class(@ptr) != 0
    end

    # @rbs return: Type?
    def pointer_type
      raise InvalidObjectError, 'Type is not valid' unless valid?

      ptr = FFIBindings.lldb_type_get_pointer_type(@ptr)
      return nil if ptr.nil? || ptr.null?

      Type.new(ptr)
    end

    # @rbs return: Type?
    def pointee_type
      raise InvalidObjectError, 'Type is not valid' unless valid?

      ptr = FFIBindings.lldb_type_get_pointee_type(@ptr)
      return nil if ptr.nil? || ptr.null?

      Type.new(ptr)
    end

    # @rbs return: Type?
    def reference_type
      raise InvalidObjectError, 'Type is not valid' unless valid?

      ptr = FFIBindings.lldb_type_get_reference_type(@ptr)
      return nil if ptr.nil? || ptr.null?

      Type.new(ptr)
    end

    # @rbs return: Type?
    def dereferenced_type
      raise InvalidObjectError, 'Type is not valid' unless valid?

      ptr = FFIBindings.lldb_type_get_dereferenced_type(@ptr)
      return nil if ptr.nil? || ptr.null?

      Type.new(ptr)
    end

    # @rbs return: Type?
    def unqualified_type
      raise InvalidObjectError, 'Type is not valid' unless valid?

      ptr = FFIBindings.lldb_type_get_unqualified_type(@ptr)
      return nil if ptr.nil? || ptr.null?

      Type.new(ptr)
    end

    # @rbs return: Type?
    def canonical_type
      raise InvalidObjectError, 'Type is not valid' unless valid?

      ptr = FFIBindings.lldb_type_get_canonical_type(@ptr)
      return nil if ptr.nil? || ptr.null?

      Type.new(ptr)
    end

    # @rbs return: Type?
    def array_element_type
      raise InvalidObjectError, 'Type is not valid' unless valid?

      ptr = FFIBindings.lldb_type_get_array_element_type(@ptr)
      return nil if ptr.nil? || ptr.null?

      Type.new(ptr)
    end

    # @rbs return: Integer
    def array_size
      return 0 unless valid?

      FFIBindings.lldb_type_get_array_size(@ptr)
    end

    # @rbs return: Integer
    def num_fields
      return 0 unless valid?

      FFIBindings.lldb_type_get_num_fields(@ptr)
    end

    # @rbs return: Integer
    def num_direct_base_classes
      return 0 unless valid?

      FFIBindings.lldb_type_get_num_direct_base_classes(@ptr)
    end

    # @rbs return: Integer
    def num_virtual_base_classes
      return 0 unless valid?

      FFIBindings.lldb_type_get_num_virtual_base_classes(@ptr)
    end

    # @rbs return: Integer
    def basic_type
      return BasicType::INVALID unless valid?

      FFIBindings.lldb_type_get_basic_type(@ptr)
    end

    # @rbs return: String
    def to_s
      display_type_name || name || '(unknown type)'
    end

    # @rbs return: String
    def inspect
      "#<LLDB::Type name=#{name.inspect} byte_size=#{byte_size}>"
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
