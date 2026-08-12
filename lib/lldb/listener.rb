# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Listener
    prepend NativeLifecycle

    # @rbs name: String?
    # @rbs context: Context?
    # @rbs pointer: FFI::Pointer?
    # @rbs return: void
    def initialize(name = nil, context: nil, pointer: nil)
      ptr = pointer || FFIBindings.lldb_listener_create(name)
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_listener_destroy(released) },
        context: context
      )
    end

    # @rbs ptr: FFI::Pointer
    # @rbs context: Context?
    # @rbs return: Listener
    def self.from_ptr(ptr, context: nil)
      new(context: context, pointer: ptr)
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_listener_is_valid(@ptr) != 0
    end

    # @rbs broadcaster: Broadcaster
    # @rbs event_mask: Integer
    # @rbs return: Integer
    def start_listening_for_events(broadcaster, event_mask)
      ensure_open!
      FFIBindings.lldb_listener_start_listening_for_events(@ptr, broadcaster.to_ptr, event_mask)
    end

    # @rbs broadcaster: Broadcaster
    # @rbs event_mask: Integer
    # @rbs return: bool
    def stop_listening_for_events(broadcaster, event_mask)
      ensure_open!
      FFIBindings.lldb_listener_stop_listening_for_events(@ptr, broadcaster.to_ptr, event_mask) != 0
    end

    # @rbs timeout_seconds: Integer
    # @rbs return: Event?
    def wait_for_event(timeout_seconds: 0)
      ensure_open!
      timeout = normalize_timeout(timeout_seconds)
      event_ptr = FFIBindings.lldb_listener_wait_for_event(@ptr, timeout)
      event_from_ptr(event_ptr)
    end

    # @rbs return: Event?
    def peek_event
      ensure_open!
      event_from_ptr(FFIBindings.lldb_listener_peek_event(@ptr))
    end

    # @rbs return: Event?
    def next_event
      ensure_open!
      event_from_ptr(FFIBindings.lldb_listener_next_event(@ptr))
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end

    private

    # @rbs timeout: Integer
    # @rbs return: Integer
    def normalize_timeout(timeout)
      unless timeout.is_a?(Integer) && timeout >= 0 && timeout <= 0xFFFFFFFF
        raise ArgumentError, 'timeout_seconds must be an Integer between 0 and UINT32_MAX'
      end

      timeout
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: Event?
    def event_from_ptr(ptr)
      return nil if ptr.nil? || ptr.null?

      Event.from_ptr(ptr, context: context)
    end
  end
end
