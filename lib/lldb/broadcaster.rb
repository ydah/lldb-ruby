# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Broadcaster
    prepend NativeLifecycle

    # @rbs name: String?
    # @rbs context: Context?
    # @rbs pointer: FFI::Pointer?
    # @rbs return: void
    def initialize(name = nil, context: nil, pointer: nil)
      ptr = pointer || FFIBindings.lldb_broadcaster_create(name)
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_broadcaster_destroy(released) },
        context: context
      )
    end

    # @rbs ptr: FFI::Pointer
    # @rbs context: Context?
    # @rbs return: Broadcaster
    def self.from_ptr(ptr, context: nil)
      new(context: context, pointer: ptr)
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_broadcaster_is_valid(@ptr) != 0
    end

    # @rbs return: String?
    def name
      return nil unless valid?

      FFIBindings.lldb_broadcaster_get_name(@ptr)
    end

    # @rbs listener: Listener
    # @rbs event_mask: Integer
    # @rbs return: Integer
    def add_listener(listener, event_mask)
      ensure_open!
      FFIBindings.lldb_broadcaster_add_listener(@ptr, listener.to_ptr, event_mask)
    end

    # @rbs listener: Listener
    # @rbs event_mask: Integer
    # @rbs return: bool
    def remove_listener(listener, event_mask = (1 << 32) - 1)
      ensure_open!
      FFIBindings.lldb_broadcaster_remove_listener(@ptr, listener.to_ptr, event_mask) != 0
    end

    # @rbs event_type: Integer
    # @rbs return: bool
    def event_type_has_listeners?(event_type)
      return false unless valid?

      FFIBindings.lldb_broadcaster_event_type_has_listeners(@ptr, event_type) != 0
    end

    # @rbs event_type: Integer
    # @rbs unique: bool
    # @rbs return: void
    def broadcast_event_by_type(event_type, unique: false)
      ensure_open!
      FFIBindings.lldb_broadcaster_broadcast_event_by_type(@ptr, event_type, unique ? 1 : 0)
      nil
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
