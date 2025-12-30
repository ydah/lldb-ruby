# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class BreakpointLocation
    # @rbs return: Breakpoint
    attr_reader :breakpoint

    # @rbs ptr: FFI::Pointer
    # @rbs breakpoint: Breakpoint
    # @rbs return: void
    def initialize(ptr, breakpoint:)
      @ptr = ptr # : FFI::Pointer
      @breakpoint = breakpoint
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_breakpoint_location_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_breakpoint_location_is_valid(@ptr) != 0
    end

    # @rbs return: Integer
    def id
      return -1 unless valid?

      FFIBindings.lldb_breakpoint_location_get_id(@ptr)
    end

    # @rbs return: Integer
    def load_address
      return 0 unless valid?

      FFIBindings.lldb_breakpoint_location_get_load_address(@ptr)
    end

    # @rbs return: bool
    def enabled?
      return false unless valid?

      FFIBindings.lldb_breakpoint_location_is_enabled(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def enabled=(value)
      raise InvalidObjectError, 'BreakpointLocation is not valid' unless valid?

      FFIBindings.lldb_breakpoint_location_set_enabled(@ptr, value ? 1 : 0)
    end

    # @rbs return: void
    def enable
      self.enabled = true
    end

    # @rbs return: void
    def disable
      self.enabled = false
    end

    # @rbs return: Integer
    def hit_count
      return 0 unless valid?

      FFIBindings.lldb_breakpoint_location_get_hit_count(@ptr)
    end

    # @rbs return: Integer
    def ignore_count
      return 0 unless valid?

      FFIBindings.lldb_breakpoint_location_get_ignore_count(@ptr)
    end

    # @rbs count: Integer
    # @rbs return: void
    def ignore_count=(count)
      raise InvalidObjectError, 'BreakpointLocation is not valid' unless valid?

      FFIBindings.lldb_breakpoint_location_set_ignore_count(@ptr, count)
    end

    # @rbs return: String?
    def condition
      return nil unless valid?

      cond = FFIBindings.lldb_breakpoint_location_get_condition(@ptr)
      cond.nil? || cond.empty? ? nil : cond
    end

    # @rbs expr: String?
    # @rbs return: void
    def condition=(expr)
      raise InvalidObjectError, 'BreakpointLocation is not valid' unless valid?

      FFIBindings.lldb_breakpoint_location_set_condition(@ptr, expr)
    end

    # @rbs return: String
    def to_s
      "BreakpointLocation #{id}: address=0x#{load_address.to_s(16)}, enabled=#{enabled?}"
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
