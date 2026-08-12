# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class AttachInfo
    prepend NativeLifecycle

    # @rbs pid: Integer?
    # @rbs context: Context?
    # @rbs return: void
    def initialize(pid = nil, context: nil)
      ptr = FFIBindings.lldb_attach_info_create(pid || 0)
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_attach_info_destroy(released) },
        context: context
      )
    end

    # @rbs return: bool
    def valid?
      !@ptr.null?
    end

    # @rbs return: Integer
    def process_id
      FFIBindings.lldb_attach_info_get_process_id(@ptr)
    end

    alias pid process_id

    # @rbs value: Integer
    # @rbs return: void
    def process_id=(value)
      FFIBindings.lldb_attach_info_set_process_id(@ptr, value)
    end

    alias pid= process_id=

    # @rbs path: String
    # @rbs return: void
    def executable=(path)
      NativeStringArray.validate!(path)
      FFIBindings.lldb_attach_info_set_executable(@ptr, path)
    end

    # @rbs file: FileSpec
    # @rbs return: void
    def executable_file=(file)
      raise ArgumentError, 'file must be a FileSpec' unless file.is_a?(FileSpec)

      FFIBindings.lldb_attach_info_set_executable_file(@ptr, file.to_ptr)
    end

    # @rbs return: bool
    def wait_for_launch?
      FFIBindings.lldb_attach_info_get_wait_for_launch(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def wait_for_launch=(value)
      FFIBindings.lldb_attach_info_set_wait_for_launch(@ptr, value ? 1 : 0)
    end

    # @rbs return: bool
    def ignore_existing?
      FFIBindings.lldb_attach_info_get_ignore_existing(@ptr) != 0
    end

    # @rbs value: bool
    # @rbs return: void
    def ignore_existing=(value)
      FFIBindings.lldb_attach_info_set_ignore_existing(@ptr, value ? 1 : 0)
    end

    # @rbs return: Integer
    def resume_count
      FFIBindings.lldb_attach_info_get_resume_count(@ptr)
    end

    # @rbs value: Integer
    # @rbs return: void
    def resume_count=(value)
      FFIBindings.lldb_attach_info_set_resume_count(@ptr, value)
    end

    # @rbs return: String?
    def process_plugin_name
      FFIBindings.lldb_attach_info_get_process_plugin_name(@ptr)
    end

    # @rbs value: String
    # @rbs return: void
    def process_plugin_name=(value)
      NativeStringArray.validate!(value)
      FFIBindings.lldb_attach_info_set_process_plugin_name(@ptr, value)
    end

    # @rbs return: Listener?
    def listener
      listener_ptr = FFIBindings.lldb_attach_info_get_listener(@ptr)
      return nil if listener_ptr.nil? || listener_ptr.null?

      Listener.from_ptr(listener_ptr, context: context)
    end

    # @rbs value: Listener
    # @rbs return: void
    def listener=(value)
      raise ArgumentError, 'listener must be a Listener' unless value.is_a?(Listener)

      FFIBindings.lldb_attach_info_set_listener(@ptr, value.to_ptr)
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
