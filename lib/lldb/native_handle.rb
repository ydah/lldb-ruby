# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class NativeHandle
    NULL_POINTER = FFI::Pointer.new(0)

    # @rbs ptr: FFI::Pointer
    # @rbs release: ^(FFI::Pointer) -> void
    # @rbs return: void
    def initialize(ptr, release:)
      @state = {
        mutex: Mutex.new,
        pointer: ptr,
        release: release,
        closed: false
      }
      ObjectSpace.define_finalizer(self, self.class.finalizer(@state))
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @state[:mutex].synchronize { @state[:pointer] }
    end

    # @rbs return: bool
    def closed?
      @state[:mutex].synchronize { @state[:closed] }
    end

    # @rbs return: bool
    def close
      pointer = @state[:mutex].synchronize do
        next nil if @state[:closed]

        @state[:closed] = true
        pointer = @state[:pointer]
        @state[:pointer] = NULL_POINTER
        pointer
      end
      return false unless pointer

      @state[:release].call(pointer) unless pointer.null?
      true
    end

    # @rbs state: Hash[Symbol, untyped]
    # @rbs return: ^(Integer) -> void
    def self.finalizer(state)
      ->(_object_id) do
        pointer = state[:mutex].synchronize do
          next nil if state[:closed]

          state[:closed] = true
          pointer = state[:pointer]
          state[:pointer] = NULL_POINTER
          pointer
        end
        next unless pointer && !pointer.null?

        begin
          state[:release].call(pointer)
        rescue StandardError
          # Finalizers must never raise into an unrelated Ruby thread.
        end
      end
    end
  end
end
