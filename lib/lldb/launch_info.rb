# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  # Launch flags for process launching
  module LaunchFlags
    NONE = 0
    EXEC = (1 << 0)
    DEBUG = (1 << 1)
    STOP_AT_ENTRY = (1 << 2)
    DISABLE_ASLR = (1 << 3)
    DISABLE_STDIO = (1 << 4)
    LAUNCH_IN_TTY = (1 << 5)
    LAUNCH_IN_SHELL = (1 << 6)
    LAUNCH_IN_SEPARATE_PROCESS_GROUP = (1 << 7)
    DONT_SET_EXIT_STATUS = (1 << 8)
    DETACH_ON_ERROR = (1 << 9)
    SHELL_EXPAND_ARGUMENTS = (1 << 10)
    CLOSE_TTY_ON_EXIT = (1 << 11)
    INHERIT_TCC_FROM_PARENT = (1 << 12)
  end

  class LaunchInfo
    # @rbs args: Array[String]?
    # @rbs return: void
    def initialize(args = nil)
      @arguments = if args && !args.empty?
                     NativeStringArray.new(args)
                   end

      @ptr = FFIBindings.lldb_launch_info_create(@arguments&.to_ptr) # : FFI::Pointer
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_launch_info_destroy(ptr) unless ptr.null? }
    end

    # @rbs dir: String
    # @rbs return: void
    def working_directory=(dir)
      NativeStringArray.validate!(dir)
      FFIBindings.lldb_launch_info_set_working_directory(@ptr, dir)
    end

    # @rbs env: Hash[String, String]?
    # @rbs append: bool
    # @rbs return: void
    def set_environment(env, append: true)
      return if env.nil?
      raise ArgumentError, 'environment must be a Hash' unless env.is_a?(Hash)

      entries = env.map do |key, value|
        key = NativeStringArray.validate!(key)
        value = NativeStringArray.validate!(value)
        raise ArgumentError, 'environment keys must not contain =' if key.include?('=')

        "#{key}=#{value}"
      end
      strings = NativeStringArray.new(entries)
      FFIBindings.lldb_launch_info_set_environment_entries(@ptr, strings.to_ptr, append ? 1 : 0)
    end

    # @rbs return: Integer
    def launch_flags
      FFIBindings.lldb_launch_info_get_launch_flags(@ptr)
    end

    # @rbs flags: Integer
    # @rbs return: void
    def launch_flags=(flags)
      FFIBindings.lldb_launch_info_set_launch_flags(@ptr, flags)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
