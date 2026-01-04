# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Thread
    # @rbs return: Process
    attr_reader :process

    # @rbs ptr: FFI::Pointer
    # @rbs process: Process
    # @rbs return: void
    def initialize(ptr, process:)
      @ptr = ptr # : FFI::Pointer
      @process = process
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_thread_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_thread_is_valid(@ptr) != 0
    end

    # @rbs return: bool
    def step_over
      raise InvalidObjectError, 'Thread is not valid' unless valid?
      APISupport.require_method!(:lldb_thread_step_over, 'step_over')

      FFIBindings.lldb_thread_step_over(@ptr) != 0
    end

    # @rbs return: bool
    def step_into
      raise InvalidObjectError, 'Thread is not valid' unless valid?
      APISupport.require_method!(:lldb_thread_step_into, 'step_into')

      FFIBindings.lldb_thread_step_into(@ptr) != 0
    end

    # @rbs return: bool
    def step_out
      raise InvalidObjectError, 'Thread is not valid' unless valid?
      APISupport.require_method!(:lldb_thread_step_out, 'step_out')

      FFIBindings.lldb_thread_step_out(@ptr) != 0
    end

    # @rbs step_over: bool
    # @rbs return: bool
    def step_instruction(step_over: false)
      raise InvalidObjectError, 'Thread is not valid' unless valid?
      APISupport.require_method!(:lldb_thread_step_instruction, 'step_instruction')

      FFIBindings.lldb_thread_step_instruction(@ptr, step_over ? 1 : 0) != 0
    end

    # @rbs return: Integer
    def num_frames
      return 0 unless valid?

      FFIBindings.lldb_thread_get_num_frames(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: Frame?
    def frame_at_index(index)
      raise InvalidObjectError, 'Thread is not valid' unless valid?

      frame_ptr = FFIBindings.lldb_thread_get_frame_at_index(@ptr, index)
      return nil if frame_ptr.nil? || frame_ptr.null?

      Frame.new(frame_ptr, thread: self)
    end

    # @rbs return: Frame?
    def selected_frame
      raise InvalidObjectError, 'Thread is not valid' unless valid?

      frame_ptr = FFIBindings.lldb_thread_get_selected_frame(@ptr)
      return nil if frame_ptr.nil? || frame_ptr.null?

      Frame.new(frame_ptr, thread: self)
    end

    # @rbs index: Integer
    # @rbs return: bool
    def select_frame(index)
      raise InvalidObjectError, 'Thread is not valid' unless valid?

      FFIBindings.lldb_thread_set_selected_frame(@ptr, index) != 0
    end

    # @rbs return: Array[Frame]
    def frames
      (0...num_frames).map { |i| frame_at_index(i) }.compact
    end

    # @rbs return: Integer
    def thread_id
      return 0 unless valid?

      FFIBindings.lldb_thread_get_thread_id(@ptr)
    end

    alias id thread_id

    # @rbs return: String?
    def name
      return nil unless valid?

      FFIBindings.lldb_thread_get_name(@ptr)
    end

    # @rbs return: Integer
    def stop_reason
      return StopReason::INVALID unless valid?

      FFIBindings.lldb_thread_get_stop_reason(@ptr)
    end

    # @rbs return: String
    def stop_reason_name
      StopReason.name(stop_reason)
    end

    # @rbs return: bool
    def stopped_at_breakpoint?
      stop_reason == StopReason::BREAKPOINT
    end

    # @rbs return: Integer
    def stop_reason_data_count
      return 0 unless valid?

      FFIBindings.lldb_thread_get_stop_reason_data_count(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: Integer
    def stop_reason_data_at_index(index)
      raise InvalidObjectError, 'Thread is not valid' unless valid?

      FFIBindings.lldb_thread_get_stop_reason_data_at_index(@ptr, index)
    end

    # @rbs return: Array[Integer]
    def stop_reason_data
      (0...stop_reason_data_count).map { |i| stop_reason_data_at_index(i) }
    end

    # @rbs address: Integer
    # @rbs return: bool
    def run_to_address(address)
      raise InvalidObjectError, 'Thread is not valid' unless valid?

      FFIBindings.lldb_thread_run_to_address(@ptr, address) != 0
    end

    # @rbs return: Integer
    def index_id
      return 0 unless valid?

      FFIBindings.lldb_thread_get_index_id(@ptr)
    end

    # @rbs return: String?
    def queue_name
      return nil unless valid?

      FFIBindings.lldb_thread_get_queue_name(@ptr)
    end

    # @rbs max_size: Integer
    # @rbs return: String?
    def stop_description(max_size = 256)
      return nil unless valid?

      FFIBindings.lldb_thread_get_stop_description(@ptr, max_size)
    end

    # @rbs return: bool
    def stopped?
      return false unless valid?

      FFIBindings.lldb_thread_is_stopped(@ptr) != 0
    end

    # @rbs return: bool
    def suspended?
      return false unless valid?

      FFIBindings.lldb_thread_is_suspended(@ptr) != 0
    end

    # @rbs return: bool
    def suspend
      raise InvalidObjectError, 'Thread is not valid' unless valid?

      FFIBindings.lldb_thread_suspend(@ptr) != 0
    end

    # @rbs return: bool
    def resume
      raise InvalidObjectError, 'Thread is not valid' unless valid?

      FFIBindings.lldb_thread_resume(@ptr) != 0
    end

    # @rbs return: Process?
    def get_process
      raise InvalidObjectError, 'Thread is not valid' unless valid?

      process_ptr = FFIBindings.lldb_thread_get_process(@ptr)
      return nil if process_ptr.nil? || process_ptr.null?

      Process.new(process_ptr, target: @process.target)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
