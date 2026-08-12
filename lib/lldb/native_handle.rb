# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class NativeHandle
    NULL_POINTER = FFI::Pointer.new(0)

    # @rbs ptr: FFI::Pointer
    # @rbs release: ^(FFI::Pointer) -> void
    # @rbs return: void
    def initialize(ptr, release:)
      # The resource is detached with Array#shift, which is a single Ruby VM
      # operation. Finalizers cannot safely acquire a Mutex on all supported
      # Rubies, so the detach itself must not depend on one.
      @state = [[ptr, release]]
      ObjectSpace.define_finalizer(self, self.class.finalizer(@state))
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      resource = @state.first
      resource ? resource.first : NULL_POINTER
    end

    # @rbs return: bool
    def closed?
      @state.empty?
    end

    # @rbs return: bool
    def close
      resource = @state.shift
      return false unless resource

      pointer, release = resource

      release.call(pointer) unless pointer.null?
      true
    end

    # @rbs state: Array[Array[untyped]]
    # @rbs return: ^(Integer) -> void
    def self.finalizer(state)
      ->(_object_id) do
        resource = state.shift
        next unless resource

        pointer, release = resource
        next unless pointer && !pointer.null?

        begin
          release.call(pointer)
        rescue StandardError
          # Finalizers must never raise into an unrelated Ruby thread.
        end
      end
    end
  end
end
