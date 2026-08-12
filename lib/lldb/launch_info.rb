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
    prepend NativeLifecycle

    # @rbs args: Array[String]?
    # @rbs return: void
    def initialize(args = nil, context: nil)
      @arguments = if args && !args.empty?
                     NativeStringArray.new(args)
                   end

      ptr = FFIBindings.lldb_launch_info_create(@arguments&.to_ptr)
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_launch_info_destroy(released) },
        context: context
      )
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_launch_info_destroy(ptr) unless ptr.null? }
    end

    # @rbs return: bool
    def valid?
      !@ptr.null?
    end

    # @rbs dir: String
    # @rbs return: void
    def working_directory=(dir)
      NativeStringArray.validate!(dir)
      FFIBindings.lldb_launch_info_set_working_directory(@ptr, dir)
    end

    # @rbs return: String?
    def working_directory
      FFIBindings.lldb_launch_info_get_working_directory(@ptr)
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

    # @rbs args: Array[String]
    # @rbs append: bool
    # @rbs return: void
    def arguments=(args, append: false)
      raise ArgumentError, 'arguments must be an Array' unless args.is_a?(Array)

      @arguments = NativeStringArray.new(args)
      FFIBindings.lldb_launch_info_set_arguments(@ptr, @arguments.to_ptr, append ? 1 : 0)
    end

    # @rbs return: Array[String]
    def arguments
      count = FFIBindings.lldb_launch_info_get_num_arguments(@ptr)
      (0...count).map { |index| FFIBindings.lldb_launch_info_get_argument_at_index(@ptr, index).to_s }
    end

    # @rbs return: Array[String]
    def environment_entries
      count = FFIBindings.lldb_launch_info_get_num_environment_entries(@ptr)
      (0...count).map do |index|
        FFIBindings.lldb_launch_info_get_environment_entry_at_index(@ptr, index).to_s
      end
    end

    # @rbs return: FileSpec?
    def executable_file
      file_ptr = FFIBindings.lldb_launch_info_get_executable_file(@ptr)
      return nil if file_ptr.nil? || file_ptr.null?

      FileSpec.from_ptr(file_ptr, context: context)
    end

    # @rbs file: FileSpec
    # @rbs add_as_first_arg: bool
    # @rbs return: void
    def executable_file=(file, add_as_first_arg: false)
      raise ArgumentError, 'file must be a FileSpec' unless file.is_a?(FileSpec)

      FFIBindings.lldb_launch_info_set_executable_file(@ptr, file.to_ptr, add_as_first_arg ? 1 : 0)
    end

    # @rbs return: Listener?
    def listener
      listener_ptr = FFIBindings.lldb_launch_info_get_listener(@ptr)
      return nil if listener_ptr.nil? || listener_ptr.null?

      Listener.from_ptr(listener_ptr, context: context)
    end

    # @rbs value: Listener
    # @rbs return: void
    def listener=(value)
      raise ArgumentError, 'listener must be a Listener' unless value.is_a?(Listener)

      FFIBindings.lldb_launch_info_set_listener(@ptr, value.to_ptr)
    end

    # @rbs return: String?
    def process_plugin_name
      FFIBindings.lldb_launch_info_get_process_plugin_name(@ptr)
    end

    # @rbs value: String
    # @rbs return: void
    def process_plugin_name=(value)
      NativeStringArray.validate!(value)
      FFIBindings.lldb_launch_info_set_process_plugin_name(@ptr, value)
    end

    # @rbs return: String?
    def shell
      FFIBindings.lldb_launch_info_get_shell(@ptr)
    end

    # @rbs value: String
    # @rbs return: void
    def shell=(value)
      NativeStringArray.validate!(value)
      FFIBindings.lldb_launch_info_set_shell(@ptr, value)
    end

    # @rbs fd: Integer
    # @rbs return: bool
    def add_close_file_action(fd)
      FFIBindings.lldb_launch_info_add_close_file_action(@ptr, fd) != 0
    end

    # @rbs fd: Integer
    # @rbs dup_fd: Integer
    # @rbs return: bool
    def add_duplicate_file_action(fd, dup_fd)
      FFIBindings.lldb_launch_info_add_duplicate_file_action(@ptr, fd, dup_fd) != 0
    end

    # @rbs fd: Integer
    # @rbs path: String
    # @rbs read: bool
    # @rbs write: bool
    # @rbs return: bool
    def add_open_file_action(fd, path, read: false, write: false)
      NativeStringArray.validate!(path)
      FFIBindings.lldb_launch_info_add_open_file_action(@ptr, fd, path, read ? 1 : 0, write ? 1 : 0) != 0
    end

    # @rbs fd: Integer
    # @rbs read: bool
    # @rbs write: bool
    # @rbs return: bool
    def add_suppress_file_action(fd, read: false, write: false)
      FFIBindings.lldb_launch_info_add_suppress_file_action(@ptr, fd, read ? 1 : 0, write ? 1 : 0) != 0
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
