# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Debugger
    # @rbs return: Debugger
    def self.create
      LLDB.ensure_initialized!
      ptr = FFIBindings.lldb_debugger_create
      raise LLDBError, 'Failed to create debugger' if ptr.null?

      new(ptr)
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: void
    def initialize(ptr)
      @ptr = ptr # : FFI::Pointer
      @targets = [] # : Array[Target]
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_debugger_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_debugger_is_valid(@ptr) != 0
    end

    # @rbs filename: String
    # @rbs arch: String?
    # @rbs platform: String?
    # @rbs add_dependent_modules: bool
    # @rbs return: Target
    def create_target(filename, arch: nil, platform: nil, add_dependent_modules: true)
      raise InvalidObjectError, 'Debugger is not valid' unless valid?

      error = Error.new
      target_ptr = FFIBindings.lldb_debugger_create_target(
        @ptr,
        filename.to_s,
        arch&.to_s,
        platform&.to_s,
        add_dependent_modules ? 1 : 0,
        error.to_ptr
      )

      error.raise_if_error!
      raise LLDBError, "Failed to create target for '#{filename}'" if target_ptr.nil? || target_ptr.null?

      target = Target.new(target_ptr, debugger: self)
      @targets << target
      target
    end

    # @rbs filename: String
    # @rbs return: Target
    def create_target_simple(filename)
      raise InvalidObjectError, 'Debugger is not valid' unless valid?

      target_ptr = FFIBindings.lldb_debugger_create_target_simple(@ptr, filename.to_s)
      raise LLDBError, "Failed to create target for '#{filename}'" if target_ptr.nil? || target_ptr.null?

      target = Target.new(target_ptr, debugger: self)
      @targets << target
      target
    end

    # @rbs return: Integer
    def num_targets
      return 0 unless valid?

      FFIBindings.lldb_debugger_get_num_targets(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: Target?
    def target_at_index(index)
      raise InvalidObjectError, 'Debugger is not valid' unless valid?

      target_ptr = FFIBindings.lldb_debugger_get_target_at_index(@ptr, index)
      return nil if target_ptr.nil? || target_ptr.null?

      Target.new(target_ptr, debugger: self)
    end

    # @rbs return: Target?
    def selected_target
      raise InvalidObjectError, 'Debugger is not valid' unless valid?

      target_ptr = FFIBindings.lldb_debugger_get_selected_target(@ptr)
      return nil if target_ptr.nil? || target_ptr.null?

      Target.new(target_ptr, debugger: self)
    end

    # @rbs return: Array[Target]
    def targets
      (0...num_targets).map { |i| target_at_index(i) }.compact
    end

    # @rbs return: bool
    def async?
      return false unless valid?

      FFIBindings.lldb_debugger_get_async(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def async=(value)
      raise InvalidObjectError, 'Debugger is not valid' unless valid?

      FFIBindings.lldb_debugger_set_async(@ptr, value ? 1 : 0)
    end

    # @rbs target: Target
    # @rbs return: void
    def selected_target=(target)
      raise InvalidObjectError, 'Debugger is not valid' unless valid?

      FFIBindings.lldb_debugger_set_selected_target(@ptr, target.to_ptr)
    end

    # @rbs target: Target
    # @rbs return: bool
    def delete_target(target)
      raise InvalidObjectError, 'Debugger is not valid' unless valid?

      result = FFIBindings.lldb_debugger_delete_target(@ptr, target.to_ptr) != 0
      @targets.delete(target) if result
      result
    end

    # @rbs pid: Integer
    # @rbs return: Target?
    def find_target_with_process_id(pid)
      raise InvalidObjectError, 'Debugger is not valid' unless valid?

      target_ptr = FFIBindings.lldb_debugger_find_target_with_process_id(@ptr, pid)
      return nil if target_ptr.nil? || target_ptr.null?

      Target.new(target_ptr, debugger: self)
    end

    # @rbs return: String?
    def self.version_string
      FFIBindings.lldb_debugger_get_version_string
    end

    # @rbs return: CommandInterpreter
    def command_interpreter
      raise InvalidObjectError, 'Debugger is not valid' unless valid?

      ci_ptr = FFIBindings.lldb_debugger_get_command_interpreter(@ptr)
      raise LLDBError, 'Failed to get command interpreter' if ci_ptr.nil? || ci_ptr.null?

      CommandInterpreter.new(ci_ptr, debugger: self)
    end

    # @rbs command: String
    # @rbs return: void
    def handle_command(command)
      raise InvalidObjectError, 'Debugger is not valid' unless valid?

      FFIBindings.lldb_debugger_handle_command(@ptr, command)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
