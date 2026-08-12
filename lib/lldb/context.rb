# frozen_string_literal: true

# rbs_inline: enabled

require 'weakref'

module LLDB
  class Context
    OPEN = :open
    CLOSING = :closing
    CLOSED = :closed

    # @rbs return: void
    def initialize
      @mutex = Mutex.new
      @state = OPEN
      @handles = [] # : Array[WeakRef]
    end

    # @rbs handle: NativeHandle
    # @rbs return: NativeHandle
    def register(handle)
      @mutex.synchronize do
        raise LifecycleError, 'LLDB context is closed' unless @state == OPEN

        @handles << WeakRef.new(handle)
      end
      handle
    end

    # @rbs return: ::Symbol
    def state
      @mutex.synchronize { @state }
    end

    # @rbs return: bool
    def open?
      state == OPEN
    end

    # @rbs return: bool
    def closing?
      state == CLOSING
    end

    # @rbs return: bool
    def closed?
      state == CLOSED
    end

    # @rbs return: void
    def ensure_open!
      raise ClosedObjectError, 'LLDB context is closed' unless open?
    end

    # @rbs except: NativeHandle?
    # @rbs return: void
    def close(except: nil)
      handles = @mutex.synchronize do
        next nil if @state == CLOSED || @state == CLOSING

        @state = CLOSING
        @handles.filter_map do |reference|
          begin
            handle = reference.__getobj__
            handle unless handle.equal?(except)
          rescue WeakRef::RefError
            nil
          end
        end
      end
      return unless handles

      begin
        handles.each(&:close)
      ensure
        @mutex.synchronize { @state = CLOSED }
      end
    end
  end
end
