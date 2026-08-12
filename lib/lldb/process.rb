# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Process
    prepend NativeLifecycle

    module BroadcastBit
      STATE_CHANGED = 1 << 0
      INTERRUPT = 1 << 1
      STDOUT = 1 << 2
      STDERR = 1 << 3
      PROFILE_DATA = 1 << 4
      STRUCTURED_DATA = 1 << 5
    end

    # @rbs return: Target?
    attr_reader :target

    # @rbs ptr: FFI::Pointer
    # @rbs target: Target?
    # @rbs return: void
    def initialize(ptr, target: nil, context: target&.context)
      @target = target
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_process_destroy(released) },
        context: context
      )
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_process_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_process_is_valid(@ptr) != 0
    end

    # @rbs return: true
    def continue
      raise InvalidObjectError, 'Process is not valid' unless valid?

      native_operation('process.continue') do |error|
        FFIBindings.lldb_process_continue(@ptr, error.to_ptr)
      end
    end

    # @rbs return: true
    def stop
      raise InvalidObjectError, 'Process is not valid' unless valid?

      native_operation('process.stop') { |error| FFIBindings.lldb_process_stop(@ptr, error.to_ptr) }
    end

    # @rbs return: true
    def kill
      raise InvalidObjectError, 'Process is not valid' unless valid?

      native_operation('process.kill') { |error| FFIBindings.lldb_process_kill(@ptr, error.to_ptr) }
    end

    # @rbs return: true
    def detach
      raise InvalidObjectError, 'Process is not valid' unless valid?

      native_operation('process.detach') { |error| FFIBindings.lldb_process_detach(@ptr, error.to_ptr) }
    end

    # @rbs return: Integer
    def state
      return State::INVALID unless valid?

      FFIBindings.lldb_process_get_state(@ptr)
    end

    # @rbs return: String
    def state_name
      State.name(state)
    end

    # @rbs return: bool
    def stopped?
      state == State::STOPPED
    end

    # @rbs return: bool
    def running?
      state == State::RUNNING
    end

    # @rbs return: bool
    def exited?
      state == State::EXITED
    end

    # @rbs return: bool
    def crashed?
      state == State::CRASHED
    end

    # @rbs return: Integer
    def num_threads
      return 0 unless valid?

      FFIBindings.lldb_process_get_num_threads(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: Thread?
    def thread_at_index(index)
      raise InvalidObjectError, 'Process is not valid' unless valid?

      thread_ptr = FFIBindings.lldb_process_get_thread_at_index(@ptr, index)
      return nil if thread_ptr.nil? || thread_ptr.null?

      Thread.new(thread_ptr, process: self, context: context)
    end

    # @rbs return: Thread?
    def selected_thread
      raise InvalidObjectError, 'Process is not valid' unless valid?

      thread_ptr = FFIBindings.lldb_process_get_selected_thread(@ptr)
      return nil if thread_ptr.nil? || thread_ptr.null?

      Thread.new(thread_ptr, process: self, context: context)
    end

    # @rbs thread_id: Integer
    # @rbs return: bool
    def select_thread_by_id(thread_id)
      raise InvalidObjectError, 'Process is not valid' unless valid?

      FFIBindings.lldb_process_set_selected_thread_by_id(@ptr, thread_id) != 0
    end

    # @rbs return: Array[Thread]
    def threads
      (0...num_threads).map { |i| thread_at_index(i) }.compact
    end

    # @rbs return: Integer
    def process_id
      return 0 unless valid?

      FFIBindings.lldb_process_get_process_id(@ptr)
    end

    alias pid process_id

    # @rbs return: Integer
    def exit_status
      raise InvalidObjectError, 'Process is not valid' unless valid?

      FFIBindings.lldb_process_get_exit_status(@ptr)
    end

    # @rbs return: String?
    def exit_description
      raise InvalidObjectError, 'Process is not valid' unless valid?

      FFIBindings.lldb_process_get_exit_description(@ptr)
    end

    # @rbs return: true
    def destroy
      raise InvalidObjectError, 'Process is not valid' unless valid?

      native_operation('process.destroy') do |error|
        FFIBindings.lldb_process_destroy_process(@ptr, error.to_ptr)
      end
    end

    # @rbs signal: Integer
    # @rbs return: true
    def signal(signal)
      raise InvalidObjectError, 'Process is not valid' unless valid?

      native_operation('process.signal') do |error|
        FFIBindings.lldb_process_signal(@ptr, signal, error.to_ptr)
      end
    end

    # @rbs thread_id: Integer
    # @rbs return: Thread?
    def thread_by_id(thread_id)
      raise InvalidObjectError, 'Process is not valid' unless valid?
      APISupport.require_method!(:lldb_process_get_thread_by_id, 'thread_by_id')

      thread_ptr = FFIBindings.lldb_process_get_thread_by_id(@ptr, thread_id)
      return nil if thread_ptr.nil? || thread_ptr.null?

      Thread.new(thread_ptr, process: self, context: context)
    end

    # @rbs index_id: Integer
    # @rbs return: Thread?
    def thread_by_index_id(index_id)
      raise InvalidObjectError, 'Process is not valid' unless valid?

      thread_ptr = FFIBindings.lldb_process_get_thread_by_index_id(@ptr, index_id)
      return nil if thread_ptr.nil? || thread_ptr.null?

      Thread.new(thread_ptr, process: self, context: context)
    end

    # @rbs index_id: Integer
    # @rbs return: bool
    def select_thread_by_index_id(index_id)
      raise InvalidObjectError, 'Process is not valid' unless valid?

      FFIBindings.lldb_process_set_selected_thread_by_index_id(@ptr, index_id) != 0
    end

    # @rbs address: Integer
    # @rbs size: Integer
    # @rbs return: String
    def read_memory(address, size)
      raise InvalidObjectError, 'Process is not valid' unless valid?
      APISupport.require_method!(:lldb_process_read_memory, 'read_memory')

      buffer = FFI::MemoryPointer.new(:uint8, size)
      error = Error.new
      bytes_read = FFIBindings.lldb_process_read_memory(@ptr, address, buffer, size, error.to_ptr)

      error.raise_if_error!('process.read_memory')
      buffer.get_bytes(0, bytes_read)
    end

    # @rbs address: Integer
    # @rbs data: String
    # @rbs return: Integer
    def write_memory(address, data)
      raise InvalidObjectError, 'Process is not valid' unless valid?
      APISupport.require_method!(:lldb_process_write_memory, 'write_memory')

      buffer = FFI::MemoryPointer.new(:uint8, [data.bytesize, 1].max)
      buffer.put_bytes(0, data)
      error = Error.new
      bytes_written = FFIBindings.lldb_process_write_memory(@ptr, address, buffer, data.bytesize, error.to_ptr)

      error.raise_if_error!('process.write_memory')
      bytes_written
    end

    # @rbs size: Integer
    # @rbs permissions: Integer
    # @rbs return: Integer
    def allocate_memory(size, permissions)
      raise InvalidObjectError, 'Process is not valid' unless valid?

      error = Error.new
      address = FFIBindings.lldb_process_allocate_memory(@ptr, size, permissions, error.to_ptr)

      error.raise_if_error!('process.allocate_memory')
      address
    end

    # @rbs address: Integer
    # @rbs return: true
    def deallocate_memory(address)
      raise InvalidObjectError, 'Process is not valid' unless valid?

      native_operation('process.deallocate_memory') do |error|
        FFIBindings.lldb_process_deallocate_memory(@ptr, address, error.to_ptr)
      end
    end

    # @rbs address: Integer
    # @rbs max_size: Integer
    # @rbs return: String
    def read_cstring_from_memory(address, max_size = 1024)
      raise InvalidObjectError, 'Process is not valid' unless valid?
      raise ArgumentError, 'max_size must be non-negative' if max_size.negative?

      buffer = FFI::MemoryPointer.new(:uint8, [max_size, 1].max)
      error = Error.new
      bytes_read = FFIBindings.lldb_process_read_cstring_from_memory(
        @ptr,
        address,
        buffer,
        max_size,
        error.to_ptr
      )

      error.raise_if_error!('process.read_cstring_from_memory')
      bytes = buffer.get_bytes(0, [bytes_read, max_size].min)
      bytes.delete_suffix("\0")
    end

    # @rbs max_size: Integer
    # @rbs return: String
    def get_stdout(max_size = 4096)
      raise InvalidObjectError, 'Process is not valid' unless valid?

      buffer = FFI::MemoryPointer.new(:uint8, max_size)
      bytes_read = FFIBindings.lldb_process_get_stdout(@ptr, buffer, max_size)
      buffer.get_bytes(0, bytes_read)
    end

    # @rbs max_size: Integer
    # @rbs return: String
    def get_stderr(max_size = 4096)
      raise InvalidObjectError, 'Process is not valid' unless valid?

      buffer = FFI::MemoryPointer.new(:uint8, max_size)
      bytes_read = FFIBindings.lldb_process_get_stderr(@ptr, buffer, max_size)
      buffer.get_bytes(0, bytes_read)
    end

    # @rbs data: String
    # @rbs return: Integer
    def put_stdin(data)
      raise InvalidObjectError, 'Process is not valid' unless valid?

      buffer = FFI::MemoryPointer.new(:uint8, [data.bytesize, 1].max)
      buffer.put_bytes(0, data)
      FFIBindings.lldb_process_put_stdin(@ptr, buffer, data.bytesize)
    end

    # @rbs return: nil
    def send_async_interrupt
      raise InvalidObjectError, 'Process is not valid' unless valid?

      FFIBindings.lldb_process_send_async_interrupt(@ptr)
      nil
    end

    # @rbs return: Broadcaster
    def broadcaster
      raise InvalidObjectError, 'Process is not valid' unless valid?

      broadcaster_ptr = FFIBindings.lldb_process_get_broadcaster(@ptr)
      raise LLDBError, 'Failed to get process broadcaster' if broadcaster_ptr.nil? || broadcaster_ptr.null?

      Broadcaster.from_ptr(broadcaster_ptr, context: context)
    end

    # @rbs return: Integer
    def num_supported_hardware_watchpoints
      return 0 unless valid?

      result = FFI::MemoryPointer.new(:uint32)
      error = Error.new
      status = FFIBindings.lldb_process_get_num_supported_hardware_watchpoints(
        @ptr,
        result,
        error.to_ptr
      )
      Native.check_status!(status, 'process.num_supported_hardware_watchpoints', error)
      result.read_uint32
    end

    # @rbs return: Integer
    def unique_id
      return 0 unless valid?

      FFIBindings.lldb_process_get_unique_id(@ptr)
    end

    # Get information about the memory region at the specified address
    #
    # @rbs address: Integer
    # @rbs return: MemoryRegionInfo?
    def get_memory_region_info(address)
      raise InvalidObjectError, 'Process is not valid' unless valid?
      APISupport.require_method!(:lldb_process_get_memory_region_info,
                                 'get_memory_region_info')

      error = Error.new
      info_ptr = FFIBindings.lldb_process_get_memory_region_info(@ptr, address, error.to_ptr)

      error.raise_if_error!('process.get_memory_region_info')
      return nil if info_ptr.nil? || info_ptr.null?

      MemoryRegionInfo.new(info_ptr, context: context)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end

    private

    # @rbs operation: String
    # @rbs &block: (Error) -> Integer
    # @rbs return: true
    def native_operation(operation, &block)
      error = Error.new
      Native.check_status!(yield(error), operation, error)
      true
    end
  end
end
