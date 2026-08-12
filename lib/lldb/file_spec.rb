# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class FileSpec
    prepend NativeLifecycle

    # @rbs path: String?
    # @rbs resolve: bool
    # @rbs context: Context?
    # @rbs pointer: FFI::Pointer?
    # @rbs return: void
    def initialize(path = nil, resolve: false, context: nil, pointer: nil)
      NativeStringArray.validate!(path) if path
      ptr = pointer || FFIBindings.lldb_file_spec_create(path, resolve ? 1 : 0)
      initialize_native_object(
        ptr,
        release: ->(released) { FFIBindings.lldb_file_spec_destroy(released) },
        context: context
      )
    end

    # @rbs ptr: FFI::Pointer
    # @rbs context: Context?
    # @rbs return: FileSpec
    def self.from_ptr(ptr, context: nil)
      new(context: context, pointer: ptr)
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_file_spec_is_valid(@ptr) != 0
    end

    # @rbs return: bool
    def exists?
      valid? && FFIBindings.lldb_file_spec_exists(@ptr) != 0
    end

    # @rbs return: String?
    def filename
      return nil unless valid?

      FFIBindings.lldb_file_spec_get_filename(@ptr)
    end

    # @rbs return: String?
    def directory
      return nil unless valid?

      FFIBindings.lldb_file_spec_get_directory(@ptr)
    end

    # @rbs return: String
    def path
      return '' unless valid?

      NativeBuffer.read_c_string do |buffer, length|
        FFIBindings.lldb_file_spec_get_path(@ptr, buffer, length)
      end
    end

    # @rbs value: String
    # @rbs return: void
    def filename=(value)
      ensure_open!
      NativeStringArray.validate!(value)
      FFIBindings.lldb_file_spec_set_filename(@ptr, value)
    end

    # @rbs value: String
    # @rbs return: void
    def directory=(value)
      ensure_open!
      NativeStringArray.validate!(value)
      FFIBindings.lldb_file_spec_set_directory(@ptr, value)
    end

    # @rbs return: String
    def to_s
      path
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
