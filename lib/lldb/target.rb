# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Target
    # @rbs return: Debugger
    attr_reader :debugger

    # @rbs ptr: FFI::Pointer
    # @rbs debugger: Debugger
    # @rbs return: void
    def initialize(ptr, debugger:)
      @ptr = ptr # : FFI::Pointer
      @debugger = debugger
      @breakpoints = [] # : Array[Breakpoint]
      @watchpoints = [] # : Array[Watchpoint]
      @process = nil # : Process?
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_target_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_target_is_valid(@ptr) != 0
    end

    # @rbs args: Array[String]?
    # @rbs env: Hash[String, String]?
    # @rbs working_dir: String?
    # @rbs return: Process
    def launch(args: nil, env: nil, working_dir: nil)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      # Use LaunchInfo with STOP_AT_ENTRY flag for reliable cross-platform behavior
      launch_info = LaunchInfo.new(args)
      launch_info.launch_flags = LaunchFlags::STOP_AT_ENTRY
      launch_info.working_directory = working_dir if working_dir
      launch_info.set_environment(env) if env

      error = Error.new
      process_ptr = FFIBindings.lldb_target_launch(@ptr, launch_info.to_ptr, error.to_ptr)

      error.raise_if_error!
      raise LaunchError, 'Failed to launch process' if process_ptr.nil? || process_ptr.null?

      @process = Process.new(process_ptr, target: self)

      # Wait for process to stop when in synchronous mode
      # On some platforms (Linux), the process may not be immediately stopped
      unless @debugger.async?
        wait_for_process_stop(@process)

        # STOP_AT_ENTRY stops at the program entry point, not at breakpoints.
        # If there are breakpoints set, continue to let the process run to the first breakpoint.
        if num_breakpoints > 0 && @process.state == State::STOPPED && @process.valid?
          @process.continue
          wait_for_process_stop(@process)
        end
      end

      @process
    end

    # @rbs launch_info: LaunchInfo
    # @rbs return: Process
    def launch_with_info(launch_info)
      raise InvalidObjectError, 'Target is not valid' unless valid?
      raise ArgumentError, 'launch_info is required' unless launch_info

      error = Error.new
      process_ptr = FFIBindings.lldb_target_launch(@ptr, launch_info.to_ptr, error.to_ptr)

      error.raise_if_error!
      raise LaunchError, 'Failed to launch process' if process_ptr.nil? || process_ptr.null?

      @process = Process.new(process_ptr, target: self)
    end

    # @rbs pid: Integer
    # @rbs return: Process
    def attach(pid:)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      error = Error.new
      process_ptr = FFIBindings.lldb_target_attach_to_process_with_id(@ptr, pid, error.to_ptr)

      error.raise_if_error!
      raise AttachError, "Failed to attach to process #{pid}" if process_ptr.nil? || process_ptr.null?

      @process = Process.new(process_ptr, target: self)
    end

    # @rbs symbol_name: String
    # @rbs module_name: String?
    # @rbs return: Breakpoint
    def breakpoint_create_by_name(symbol_name, module_name: nil)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      bp_ptr = FFIBindings.lldb_target_breakpoint_create_by_name(@ptr, symbol_name, module_name)
      raise BreakpointError, "Failed to create breakpoint for '#{symbol_name}'" if bp_ptr.nil? || bp_ptr.null?

      bp = Breakpoint.new(bp_ptr, target: self)
      @breakpoints << bp
      bp
    end

    # @rbs file: String
    # @rbs line: Integer
    # @rbs return: Breakpoint
    def breakpoint_create_by_location(file, line)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      bp_ptr = FFIBindings.lldb_target_breakpoint_create_by_location(@ptr, file, line)
      raise BreakpointError, "Failed to create breakpoint at #{file}:#{line}" if bp_ptr.nil? || bp_ptr.null?

      bp = Breakpoint.new(bp_ptr, target: self)
      @breakpoints << bp
      bp
    end

    # @rbs address: Integer
    # @rbs return: Breakpoint
    def breakpoint_create_by_address(address)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      bp_ptr = FFIBindings.lldb_target_breakpoint_create_by_address(@ptr, address)
      if bp_ptr.nil? || bp_ptr.null?
        raise BreakpointError,
              "Failed to create breakpoint at address 0x#{address.to_s(16)}"
      end

      bp = Breakpoint.new(bp_ptr, target: self)
      @breakpoints << bp
      bp
    end

    # @rbs breakpoint_id: Integer
    # @rbs return: bool
    def delete_breakpoint(breakpoint_id)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      result = FFIBindings.lldb_target_delete_breakpoint(@ptr, breakpoint_id)
      @breakpoints.reject! { |bp| bp.id == breakpoint_id } if result != 0
      result != 0
    end

    # @rbs id: Integer
    # @rbs return: Breakpoint?
    def find_breakpoint_by_id(id)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      bp_ptr = FFIBindings.lldb_target_find_breakpoint_by_id(@ptr, id)
      return nil if bp_ptr.nil? || bp_ptr.null?

      Breakpoint.new(bp_ptr, target: self)
    end

    # @rbs return: Integer
    def num_breakpoints
      return 0 unless valid?

      FFIBindings.lldb_target_get_num_breakpoints(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: Breakpoint?
    def breakpoint_at_index(index)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      bp_ptr = FFIBindings.lldb_target_get_breakpoint_at_index(@ptr, index)
      return nil if bp_ptr.nil? || bp_ptr.null?

      Breakpoint.new(bp_ptr, target: self)
    end

    # @rbs return: Array[Breakpoint]
    def breakpoints
      (0...num_breakpoints).map { |i| breakpoint_at_index(i) }.compact
    end

    # @rbs return: Process?
    def process
      raise InvalidObjectError, 'Target is not valid' unless valid?

      process_ptr = FFIBindings.lldb_target_get_process(@ptr)
      return nil if process_ptr.nil? || process_ptr.null?

      Process.new(process_ptr, target: self)
    end

    # @rbs return: String?
    def executable_path
      raise InvalidObjectError, 'Target is not valid' unless valid?

      FFIBindings.lldb_target_get_executable_path(@ptr)
    end

    # @rbs return: Integer
    def num_modules
      return 0 unless valid?

      FFIBindings.lldb_target_get_num_modules(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: Module?
    def module_at_index(index)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      mod_ptr = FFIBindings.lldb_target_get_module_at_index(@ptr, index)
      return nil if mod_ptr.nil? || mod_ptr.null?

      Module.new(mod_ptr, target: self)
    end

    # @rbs return: Array[Module]
    def modules
      (0...num_modules).map { |i| module_at_index(i) }.compact
    end

    # @rbs name: String
    # @rbs wait_for: bool
    # @rbs return: Process
    def attach_with_name(name, wait_for: false)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      error = Error.new
      process_ptr = FFIBindings.lldb_target_attach_to_process_with_name(@ptr, name, wait_for ? 1 : 0, error.to_ptr)

      error.raise_if_error!
      raise AttachError, "Failed to attach to process '#{name}'" if process_ptr.nil? || process_ptr.null?

      @process = Process.new(process_ptr, target: self)
    end

    # @rbs regex: String
    # @rbs module_name: String?
    # @rbs return: Breakpoint
    def breakpoint_create_by_regex(regex, module_name: nil)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      bp_ptr = FFIBindings.lldb_target_breakpoint_create_by_regex(@ptr, regex, module_name)
      raise BreakpointError, "Failed to create breakpoint for regex '#{regex}'" if bp_ptr.nil? || bp_ptr.null?

      bp = Breakpoint.new(bp_ptr, target: self)
      @breakpoints << bp
      bp
    end

    # @rbs regex: String
    # @rbs source_file: String?
    # @rbs return: Breakpoint
    def breakpoint_create_by_source_regex(regex, source_file: nil)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      bp_ptr = FFIBindings.lldb_target_breakpoint_create_by_source_regex(@ptr, regex, source_file)
      raise BreakpointError, "Failed to create breakpoint for source regex '#{regex}'" if bp_ptr.nil? || bp_ptr.null?

      bp = Breakpoint.new(bp_ptr, target: self)
      @breakpoints << bp
      bp
    end

    # @rbs return: bool
    def delete_all_breakpoints
      raise InvalidObjectError, 'Target is not valid' unless valid?

      result = FFIBindings.lldb_target_delete_all_breakpoints(@ptr) != 0
      @breakpoints.clear if result
      result
    end

    # @rbs return: bool
    def enable_all_breakpoints
      raise InvalidObjectError, 'Target is not valid' unless valid?

      FFIBindings.lldb_target_enable_all_breakpoints(@ptr) != 0
    end

    # @rbs return: bool
    def disable_all_breakpoints
      raise InvalidObjectError, 'Target is not valid' unless valid?

      FFIBindings.lldb_target_disable_all_breakpoints(@ptr) != 0
    end

    # @rbs expression: String
    # @rbs return: Value?
    def evaluate_expression(expression)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      value_ptr = FFIBindings.lldb_target_evaluate_expression(@ptr, expression)
      return nil if value_ptr.nil? || value_ptr.null?

      Value.new(value_ptr, parent: self)
    end

    # @rbs address: Integer
    # @rbs size: Integer
    # @rbs return: String
    def read_memory(address, size)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      buffer = FFI::MemoryPointer.new(:uint8, size)
      error = Error.new
      bytes_read = FFIBindings.lldb_target_read_memory(@ptr, address, buffer, size, error.to_ptr)

      error.raise_if_error!
      buffer.read_string(bytes_read)
    end

    # @rbs return: Integer
    def address_byte_size
      return 0 unless valid?

      FFIBindings.lldb_target_get_address_byte_size(@ptr)
    end

    # @rbs return: String?
    def triple
      return nil unless valid?

      FFIBindings.lldb_target_get_triple(@ptr)
    end

    # @rbs address: Integer
    # @rbs size: Integer
    # @rbs read: bool
    # @rbs write: bool
    # @rbs return: Watchpoint
    def watch_address(address, size, read: false, write: true)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      error = Error.new
      wp_ptr = FFIBindings.lldb_target_watch_address(@ptr, address, size, read ? 1 : 0, write ? 1 : 0, error.to_ptr)

      error.raise_if_error!
      raise LLDBError, "Failed to create watchpoint at address 0x#{address.to_s(16)}" if wp_ptr.nil? || wp_ptr.null?

      wp = Watchpoint.new(wp_ptr, target: self)
      @watchpoints << wp
      wp
    end

    # @rbs watchpoint_id: Integer
    # @rbs return: bool
    def delete_watchpoint(watchpoint_id)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      result = FFIBindings.lldb_target_delete_watchpoint(@ptr, watchpoint_id)
      @watchpoints.reject! { |wp| wp.id == watchpoint_id } if result != 0
      result != 0
    end

    # @rbs return: bool
    def delete_all_watchpoints
      raise InvalidObjectError, 'Target is not valid' unless valid?

      result = FFIBindings.lldb_target_delete_all_watchpoints(@ptr) != 0
      @watchpoints.clear if result
      result
    end

    # @rbs id: Integer
    # @rbs return: Watchpoint?
    def find_watchpoint_by_id(id)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      wp_ptr = FFIBindings.lldb_target_find_watchpoint_by_id(@ptr, id)
      return nil if wp_ptr.nil? || wp_ptr.null?

      Watchpoint.new(wp_ptr, target: self)
    end

    # @rbs return: Integer
    def num_watchpoints
      return 0 unless valid?

      FFIBindings.lldb_target_get_num_watchpoints(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: Watchpoint?
    def watchpoint_at_index(index)
      raise InvalidObjectError, 'Target is not valid' unless valid?

      wp_ptr = FFIBindings.lldb_target_get_watchpoint_at_index(@ptr, index)
      return nil if wp_ptr.nil? || wp_ptr.null?

      Watchpoint.new(wp_ptr, target: self)
    end

    # @rbs return: Array[Watchpoint]
    def watchpoints
      (0...num_watchpoints).map { |i| watchpoint_at_index(i) }.compact
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end

    private

    # @rbs process: Process
    # @rbs timeout: Float
    # @rbs return: void
    def wait_for_process_stop(process, timeout: 10.0)
      require 'timeout'

      Timeout.timeout(timeout, LaunchError, "Process failed to stop within #{timeout} seconds") do
        loop do
          state = process.state
          break if state == State::STOPPED || state == State::EXITED || state == State::CRASHED

          sleep(0.01)
        end
      end
    end
  end
end
