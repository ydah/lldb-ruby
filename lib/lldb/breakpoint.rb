# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Breakpoint
    # @rbs return: Target
    attr_reader :target

    # @rbs ptr: FFI::Pointer
    # @rbs target: Target
    # @rbs return: void
    def initialize(ptr, target:)
      @ptr = ptr # : FFI::Pointer
      @target = target
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_breakpoint_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_breakpoint_is_valid(@ptr) != 0
    end

    # @rbs return: Integer
    def id
      return -1 unless valid?

      FFIBindings.lldb_breakpoint_get_id(@ptr)
    end

    # @rbs return: bool
    def enabled?
      return false unless valid?

      FFIBindings.lldb_breakpoint_is_enabled(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def enabled=(value)
      raise InvalidObjectError, 'Breakpoint is not valid' unless valid?

      FFIBindings.lldb_breakpoint_set_enabled(@ptr, value ? 1 : 0)
    end

    # @rbs return: void
    def enable
      self.enabled = true
    end

    # @rbs return: void
    def disable
      self.enabled = false
    end

    # @rbs return: bool
    def one_shot?
      return false unless valid?

      FFIBindings.lldb_breakpoint_is_one_shot(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def one_shot=(value)
      raise InvalidObjectError, 'Breakpoint is not valid' unless valid?

      FFIBindings.lldb_breakpoint_set_one_shot(@ptr, value ? 1 : 0)
    end

    # @rbs return: Integer
    def hit_count
      return 0 unless valid?

      FFIBindings.lldb_breakpoint_get_hit_count(@ptr)
    end

    # @rbs return: Integer
    def ignore_count
      return 0 unless valid?

      FFIBindings.lldb_breakpoint_get_ignore_count(@ptr)
    end

    # @rbs count: Integer
    # @rbs return: void
    def ignore_count=(count)
      raise InvalidObjectError, 'Breakpoint is not valid' unless valid?

      FFIBindings.lldb_breakpoint_set_ignore_count(@ptr, count)
    end

    # @rbs return: String?
    def condition
      return nil unless valid?

      cond = FFIBindings.lldb_breakpoint_get_condition(@ptr)
      cond.nil? || cond.empty? ? nil : cond
    end

    # @rbs expr: String?
    # @rbs return: void
    def condition=(expr)
      raise InvalidObjectError, 'Breakpoint is not valid' unless valid?

      FFIBindings.lldb_breakpoint_set_condition(@ptr, expr)
    end

    # @rbs return: Integer
    def num_locations
      return 0 unless valid?

      FFIBindings.lldb_breakpoint_get_num_locations(@ptr)
    end

    # @rbs return: bool
    def delete
      raise InvalidObjectError, 'Breakpoint is not valid' unless valid?

      target.delete_breakpoint(id)
    end

    # @rbs return: String
    def to_s
      "Breakpoint #{id}: enabled=#{enabled?}, hit_count=#{hit_count}, locations=#{num_locations}"
    end

    # @rbs index: Integer
    # @rbs return: BreakpointLocation?
    def location_at_index(index)
      raise InvalidObjectError, 'Breakpoint is not valid' unless valid?

      loc_ptr = FFIBindings.lldb_breakpoint_get_location_at_index(@ptr, index)
      return nil if loc_ptr.nil? || loc_ptr.null?

      BreakpointLocation.new(loc_ptr, breakpoint: self)
    end

    # @rbs location_id: Integer
    # @rbs return: BreakpointLocation?
    def find_location_by_id(location_id)
      raise InvalidObjectError, 'Breakpoint is not valid' unless valid?

      loc_ptr = FFIBindings.lldb_breakpoint_find_location_by_id(@ptr, location_id)
      return nil if loc_ptr.nil? || loc_ptr.null?

      BreakpointLocation.new(loc_ptr, breakpoint: self)
    end

    # @rbs return: Array[BreakpointLocation]
    def locations
      (0...num_locations).map { |i| location_at_index(i) }.compact
    end

    # @rbs return: bool
    def hardware?
      return false unless valid?

      FFIBindings.lldb_breakpoint_is_hardware(@ptr) != 0
    end

    # @rbs return: bool
    def auto_continue?
      return false unless valid?

      FFIBindings.lldb_breakpoint_get_auto_continue(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def auto_continue=(value)
      raise InvalidObjectError, 'Breakpoint is not valid' unless valid?

      FFIBindings.lldb_breakpoint_set_auto_continue(@ptr, value ? 1 : 0)
    end

    # @rbs return: Integer
    def thread_id
      return 0 unless valid?

      FFIBindings.lldb_breakpoint_get_thread_id(@ptr)
    end

    # @rbs value: Integer
    # @rbs return: void
    def thread_id=(value)
      raise InvalidObjectError, 'Breakpoint is not valid' unless valid?

      FFIBindings.lldb_breakpoint_set_thread_id(@ptr, value)
    end

    # @rbs return: String?
    def thread_name
      return nil unless valid?

      FFIBindings.lldb_breakpoint_get_thread_name(@ptr)
    end

    # @rbs value: String?
    # @rbs return: void
    def thread_name=(value)
      raise InvalidObjectError, 'Breakpoint is not valid' unless valid?

      FFIBindings.lldb_breakpoint_set_thread_name(@ptr, value)
    end

    # @rbs return: Integer
    def thread_index
      return 0 unless valid?

      FFIBindings.lldb_breakpoint_get_thread_index(@ptr)
    end

    # @rbs value: Integer
    # @rbs return: void
    def thread_index=(value)
      raise InvalidObjectError, 'Breakpoint is not valid' unless valid?

      FFIBindings.lldb_breakpoint_set_thread_index(@ptr, value)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
