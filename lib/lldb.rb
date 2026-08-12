# frozen_string_literal: true

# rbs_inline: enabled

require_relative 'lldb/version'
require_relative 'lldb/error'
require_relative 'lldb/ffi_bindings'
require_relative 'lldb/types'
require_relative 'lldb/native'
require_relative 'lldb/native_handle'
require_relative 'lldb/context'
require_relative 'lldb/native_lifecycle'
require_relative 'lldb/api_support'
require_relative 'lldb/native_string_array'
require_relative 'lldb/debugger'
require_relative 'lldb/target'
require_relative 'lldb/launch_info'
require_relative 'lldb/process'
require_relative 'lldb/memory_region_info'
require_relative 'lldb/thread'
require_relative 'lldb/frame'
require_relative 'lldb/breakpoint'
require_relative 'lldb/breakpoint_location'
require_relative 'lldb/value'
require_relative 'lldb/value_list'
require_relative 'lldb/type'
require_relative 'lldb/watchpoint'
require_relative 'lldb/module'
require_relative 'lldb/symbol_context'
require_relative 'lldb/command_return_object'
require_relative 'lldb/command_interpreter'

module LLDB
  class << self
    # @rbs return: void
    def initialize
      lifecycle_mutex.synchronize do
        return if @initialized

        error = Error.new
        status = FFIBindings.lldb_initialize(error.to_ptr)
        Native.check_status!(status, 'lldb.initialize', error)
        @initialized = true

        at_exit { terminate if open_debugger_count.zero? }
      end
    end

    # @rbs return: void
    def terminate
      lifecycle_mutex.synchronize do
        return unless @initialized
        if live_debugger_count.positive?
          raise LifecycleError, 'cannot terminate LLDB while debuggers are open'
        end

        FFIBindings.lldb_terminate
        @initialized = false
      end
    end

    # @rbs return: bool
    def initialized?
      @initialized == true
    end

    # @rbs return: void
    def ensure_initialized!
      return if initialized?

      raise LLDBError, 'LLDB has not been initialized. Call LLDB.initialize first.'
    end

    # @rbs return: Debugger
    def create_debugger
      LLDB.initialize
      Debugger.create
    end

    # @rbs return: Integer
    def open_debugger_count
      lifecycle_mutex.synchronize { live_debugger_count }
    end

    # @rbs debugger: Debugger
    # @rbs return: void
    def register_debugger(debugger)
      lifecycle_mutex.synchronize do
        debuggers = (@debuggers ||= []) # : Array[WeakRef]
        debuggers << WeakRef.new(debugger)
      end
    end

    private

    # @rbs return: Integer
    def live_debugger_count
      live = [] # : Array[WeakRef]
      count = (@debuggers || []).count do |reference|
        begin
          debugger = reference.__getobj__
          live << reference
          !debugger.closed?
        rescue WeakRef::RefError
          false
        end
      end
      @debuggers = live
      count
    end

    # @rbs return: Mutex
    def lifecycle_mutex
      @lifecycle_mutex ||= Mutex.new
    end
  end
end
