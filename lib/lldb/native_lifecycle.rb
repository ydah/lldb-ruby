# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  module NativeLifecycle
    # @rbs return: bool
    def closed?
      @native_handle ? @native_handle.closed? : false
    end

    # @rbs return: bool
    def close
      return false unless @native_handle
      return false if @native_handle.closed?

      send(:close_context) if respond_to?(:close_context, true)
      @native_handle.close
      @ptr = NativeHandle::NULL_POINTER
      true
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      return NativeHandle::NULL_POINTER if closed?

      super
    end

    # @rbs return: void
    def ensure_open!
      raise ClosedObjectError, "#{self.class} is closed" if closed?
      @context&.ensure_open!
    end

    # @rbs ptr: FFI::Pointer
    # @rbs release: ^(FFI::Pointer) -> void
    # @rbs context: Context?
    # @rbs return: void
    def initialize_native_object(ptr, release:, context: nil)
      @ptr = ptr
      @context = context
      @native_handle = NativeHandle.new(ptr, release: release)
      context&.register(@native_handle)
    end

    # @rbs return: Context?
    def context
      @context
    end

    # @rbs return: Context
    def context!
      context || raise(LifecycleError, 'LLDB object has no context')
    end

    # @rbs return: bool
    def valid?
      return false if closed? || @context&.closed?

      super
    end
  end
end
