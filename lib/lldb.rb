# frozen_string_literal: true

# rbs_inline: enabled

require_relative 'lldb/version'
require_relative 'lldb/ffi_bindings'
require_relative 'lldb/types'
require_relative 'lldb/error'
require_relative 'lldb/api_support'
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
      return if @initialized

      FFIBindings.lldb_initialize
      @initialized = true

      at_exit { terminate }
    end

    # @rbs return: void
    def terminate
      return unless @initialized

      FFIBindings.lldb_terminate
      @initialized = false
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
  end
end
