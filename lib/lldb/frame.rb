# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Frame
    # @rbs return: Thread
    attr_reader :thread

    # @rbs ptr: FFI::Pointer
    # @rbs thread: Thread
    # @rbs return: void
    def initialize(ptr, thread:)
      @ptr = ptr # : FFI::Pointer
      @thread = thread
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_frame_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_frame_is_valid(@ptr) != 0
    end

    # @rbs return: String?
    def function_name
      raise InvalidObjectError, 'Frame is not valid' unless valid?

      FFIBindings.lldb_frame_get_function_name(@ptr)
    end

    # @rbs return: String?
    def display_function_name
      raise InvalidObjectError, 'Frame is not valid' unless valid?

      FFIBindings.lldb_frame_get_display_function_name(@ptr)
    end

    # @rbs return: Integer
    def line
      return 0 unless valid?

      FFIBindings.lldb_frame_get_line(@ptr)
    end

    # @rbs return: String?
    def file_path
      raise InvalidObjectError, 'Frame is not valid' unless valid?

      FFIBindings.lldb_frame_get_file_path(@ptr)
    end

    # @rbs return: Integer
    def column
      return 0 unless valid?

      FFIBindings.lldb_frame_get_column(@ptr)
    end

    # @rbs return: Integer
    def pc
      return 0 unless valid?

      FFIBindings.lldb_frame_get_pc(@ptr)
    end

    # @rbs return: Integer
    def sp
      return 0 unless valid?

      FFIBindings.lldb_frame_get_sp(@ptr)
    end

    # @rbs return: Integer
    def fp
      return 0 unless valid?

      FFIBindings.lldb_frame_get_fp(@ptr)
    end

    # @rbs return: Integer
    def frame_id
      return 0 unless valid?

      FFIBindings.lldb_frame_get_frame_id(@ptr)
    end

    alias id frame_id

    # @rbs name: String
    # @rbs return: Value?
    def find_variable(name)
      raise InvalidObjectError, 'Frame is not valid' unless valid?
      APISupport.require_method!(:lldb_frame_find_variable, 'find_variable')

      value_ptr = FFIBindings.lldb_frame_find_variable(@ptr, name)
      return nil if value_ptr.nil? || value_ptr.null?

      Value.new(value_ptr, parent: self)
    end

    # @rbs expression: String
    # @rbs return: Value?
    def evaluate_expression(expression)
      raise InvalidObjectError, 'Frame is not valid' unless valid?
      APISupport.require_method!(:lldb_frame_evaluate_expression, 'evaluate_expression')

      value_ptr = FFIBindings.lldb_frame_evaluate_expression(@ptr, expression)
      return nil if value_ptr.nil? || value_ptr.null?

      Value.new(value_ptr, parent: self)
    end

    # @rbs path: String
    # @rbs return: Value?
    def get_value_for_variable_path(path)
      raise InvalidObjectError, 'Frame is not valid' unless valid?

      value_ptr = FFIBindings.lldb_frame_get_value_for_variable_path(@ptr, path)
      return nil if value_ptr.nil? || value_ptr.null?

      Value.new(value_ptr, parent: self)
    end

    # @rbs scope: Integer
    # @rbs return: SymbolContext?
    def symbol_context(scope = SymbolContextItem::EVERYTHING)
      raise InvalidObjectError, 'Frame is not valid' unless valid?

      ctx_ptr = FFIBindings.lldb_frame_get_symbol_context(@ptr, scope)
      return nil if ctx_ptr.nil? || ctx_ptr.null?

      SymbolContext.new(ctx_ptr, parent: self)
    end

    # @rbs return: String
    def location
      "#{file_path || '?'}:#{line}"
    end

    # @rbs return: String
    def to_s
      "#{display_function_name || function_name || '?'} at #{location}"
    end

    # @rbs value: Integer
    # @rbs return: bool
    def pc=(value)
      raise InvalidObjectError, 'Frame is not valid' unless valid?

      FFIBindings.lldb_frame_set_pc(@ptr, value) != 0
    end

    # @rbs arguments: bool
    # @rbs locals: bool
    # @rbs statics: bool
    # @rbs in_scope_only: bool
    # @rbs return: ValueList
    def get_variables(arguments: true, locals: true, statics: true, in_scope_only: true)
      raise InvalidObjectError, 'Frame is not valid' unless valid?

      list_ptr = FFIBindings.lldb_frame_get_variables(
        @ptr,
        arguments ? 1 : 0,
        locals ? 1 : 0,
        statics ? 1 : 0,
        in_scope_only ? 1 : 0
      )
      raise LLDBError, 'Failed to get variables' if list_ptr.nil? || list_ptr.null?

      ValueList.new(list_ptr, parent: self)
    end

    # @rbs return: ValueList
    def get_registers
      raise InvalidObjectError, 'Frame is not valid' unless valid?
      APISupport.require_method!(:lldb_frame_get_registers, 'get_registers')

      list_ptr = FFIBindings.lldb_frame_get_registers(@ptr)
      raise LLDBError, 'Failed to get registers' if list_ptr.nil? || list_ptr.null?

      ValueList.new(list_ptr, parent: self)
    end

    # @rbs return: bool
    def inlined?
      return false unless valid?

      FFIBindings.lldb_frame_is_inlined(@ptr) != 0
    end

    # @rbs return: String?
    def disassemble
      raise InvalidObjectError, 'Frame is not valid' unless valid?

      FFIBindings.lldb_frame_disassemble(@ptr)
    end

    # @rbs return: Module?
    def get_module
      raise InvalidObjectError, 'Frame is not valid' unless valid?

      mod_ptr = FFIBindings.lldb_frame_get_module(@ptr)
      return nil if mod_ptr.nil? || mod_ptr.null?

      Module.new(mod_ptr, target: @thread.process.target)
    end

    # @rbs return: Thread?
    def get_thread
      raise InvalidObjectError, 'Frame is not valid' unless valid?

      thread_ptr = FFIBindings.lldb_frame_get_thread(@ptr)
      return nil if thread_ptr.nil? || thread_ptr.null?

      Thread.new(thread_ptr, process: @thread.process)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
