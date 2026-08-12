# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class Watchpoint
    prepend NativeLifecycle

    # @rbs return: Target
    attr_reader :target

    # @rbs ptr: FFI::Pointer
    # @rbs target: Target
    # @rbs return: void
    def initialize(ptr, target:, context: target.context)
      @target = target
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_watchpoint_destroy(released) },
        context: context
      )
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_watchpoint_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_watchpoint_is_valid(@ptr) != 0
    end

    # @rbs return: Integer
    def id
      return -1 unless valid?

      FFIBindings.lldb_watchpoint_get_id(@ptr)
    end

    # @rbs return: bool
    def enabled?
      return false unless valid?

      FFIBindings.lldb_watchpoint_is_enabled(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def enabled=(value)
      raise InvalidObjectError, 'Watchpoint is not valid' unless valid?

      FFIBindings.lldb_watchpoint_set_enabled(@ptr, value ? 1 : 0)
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

      FFIBindings.lldb_watchpoint_get_hit_count(@ptr)
    end

    # @rbs return: Integer
    def ignore_count
      return 0 unless valid?

      FFIBindings.lldb_watchpoint_get_ignore_count(@ptr)
    end

    # @rbs count: Integer
    # @rbs return: void
    def ignore_count=(count)
      raise InvalidObjectError, 'Watchpoint is not valid' unless valid?

      FFIBindings.lldb_watchpoint_set_ignore_count(@ptr, count)
    end

    # @rbs return: String?
    def condition
      return nil unless valid?

      cond = FFIBindings.lldb_watchpoint_get_condition(@ptr)
      cond.nil? || cond.empty? ? nil : cond
    end

    # @rbs expr: String?
    # @rbs return: void
    def condition=(expr)
      raise InvalidObjectError, 'Watchpoint is not valid' unless valid?

      FFIBindings.lldb_watchpoint_set_condition(@ptr, expr)
    end

    # @rbs return: Integer
    def watch_address
      return 0 unless valid?

      FFIBindings.lldb_watchpoint_get_watch_address(@ptr)
    end

    # @rbs return: Integer
    def watch_size
      return 0 unless valid?

      FFIBindings.lldb_watchpoint_get_watch_size(@ptr)
    end

    # @rbs return: bool
    def watching_reads?
      raise InvalidObjectError, 'Watchpoint is not valid' unless valid?
      APISupport.require_feature!(:watchpoint_access_kind)

      result = FFI::MemoryPointer.new(:int)
      status = FFIBindings.lldb_watchpoint_is_watching_reads(@ptr, result)
      raise UnsupportedAPIError, 'Watchpoint read access is unsupported' if status == NativeStatus::UNSUPPORTED
      raise LLDBError, "Failed to query watchpoint read access (status=#{status})" unless status == NativeStatus::OK

      result.read_int != 0
    end

    # @rbs return: bool
    def watching_writes?
      raise InvalidObjectError, 'Watchpoint is not valid' unless valid?
      APISupport.require_feature!(:watchpoint_access_kind)

      result = FFI::MemoryPointer.new(:int)
      status = FFIBindings.lldb_watchpoint_is_watching_writes(@ptr, result)
      raise UnsupportedAPIError, 'Watchpoint write access is unsupported' if status == NativeStatus::UNSUPPORTED
      raise LLDBError, "Failed to query watchpoint write access (status=#{status})" unless status == NativeStatus::OK

      result.read_int != 0
    end

    # @rbs return: bool
    def delete
      raise InvalidObjectError, 'Watchpoint is not valid' unless valid?

      target.delete_watchpoint(id)
    end

    # @rbs return: String
    def to_s
      "Watchpoint #{id}: address=0x#{watch_address.to_s(16)}, size=#{watch_size}, enabled=#{enabled?}"
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
