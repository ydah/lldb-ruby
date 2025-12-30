# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class CommandInterpreter
    # @rbs return: Debugger
    attr_reader :debugger

    # @rbs ptr: FFI::Pointer
    # @rbs debugger: Debugger
    # @rbs return: void
    def initialize(ptr, debugger:)
      @ptr = ptr # : FFI::Pointer
      @debugger = debugger
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_command_interpreter_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_command_interpreter_is_valid(@ptr) != 0
    end

    # @rbs command: String
    # @rbs add_to_history: bool
    # @rbs return: CommandReturnObject
    def handle_command(command, add_to_history: true)
      raise InvalidObjectError, 'CommandInterpreter is not valid' unless valid?

      result = CommandReturnObject.create
      FFIBindings.lldb_command_interpreter_handle_command(@ptr, command, result.to_ptr, add_to_history ? 1 : 0)
      result
    end

    # @rbs command: String
    # @rbs return: bool
    def command_exists?(command)
      return false unless valid?

      FFIBindings.lldb_command_interpreter_command_exists(@ptr, command) != 0
    end

    # @rbs alias_name: String
    # @rbs return: bool
    def alias_exists?(alias_name)
      return false unless valid?

      FFIBindings.lldb_command_interpreter_alias_exists(@ptr, alias_name) != 0
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
