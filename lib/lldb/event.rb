# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Event
    prepend NativeLifecycle

    # @rbs context: Context?
    # @rbs pointer: FFI::Pointer
    # @rbs return: void
    def initialize(pointer, context: nil)
      initialize_native_object(
        pointer,
        release: ->(released) { FFIBindings.lldb_event_destroy(released) },
        context: context
      )
    end

    # @rbs ptr: FFI::Pointer
    # @rbs context: Context?
    # @rbs return: Event
    def self.from_ptr(ptr, context: nil)
      new(ptr, context: context)
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_event_is_valid(@ptr) != 0
    end

    # @rbs return: Integer
    def type
      return 0 unless valid?

      FFIBindings.lldb_event_get_type(@ptr)
    end

    # @rbs return: String?
    def data_flavor
      return nil unless valid?

      FFIBindings.lldb_event_get_data_flavor(@ptr)
    end

    # @rbs return: String?
    def broadcaster_class
      return nil unless valid?

      FFIBindings.lldb_event_get_broadcaster_class(@ptr)
    end

    # @rbs return: String
    def description
      return '' unless valid?

      NativeBuffer.read_c_string do |buffer, length|
        FFIBindings.lldb_event_get_description(@ptr, buffer, length)
      end
    end

    # @rbs return: Broadcaster?
    def broadcaster
      return nil unless valid?

      broadcaster_ptr = FFIBindings.lldb_event_get_broadcaster(@ptr)
      return nil if broadcaster_ptr.nil? || broadcaster_ptr.null?

      Broadcaster.from_ptr(broadcaster_ptr, context: context)
    end

    # @rbs return: bool
    def process_event?
      valid? && FFIBindings.lldb_event_is_process_event(@ptr) != 0
    end

    # @rbs return: Integer
    def process_state
      return State::INVALID unless valid?

      FFIBindings.lldb_event_get_process_state(@ptr)
    end

    # @rbs return: Process?
    def process
      return nil unless process_event?

      process_ptr = FFIBindings.lldb_event_get_process(@ptr)
      return nil if process_ptr.nil? || process_ptr.null?

      Process.new(process_ptr, target: nil, context: context)
    end

    # @rbs return: bool
    def restarted?
      valid? && FFIBindings.lldb_event_get_restarted(@ptr) != 0
    end

    # @rbs return: bool
    def interrupted?
      valid? && FFIBindings.lldb_event_get_interrupted(@ptr) != 0
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
