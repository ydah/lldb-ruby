# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  class FileSpecList
    prepend NativeLifecycle

    # @rbs context: Context?
    # @rbs return: void
    def initialize(context: nil)
      initialize_native_object(
        FFIBindings.lldb_file_spec_list_create,
        release: ->(released) { FFIBindings.lldb_file_spec_list_destroy(released) },
        context: context
      )
    end

    # @rbs return: bool
    def valid?
      !@ptr.null? && FFIBindings.lldb_file_spec_list_is_valid(@ptr) != 0
    end

    # @rbs return: Integer
    def size
      return 0 unless valid?

      FFIBindings.lldb_file_spec_list_get_size(@ptr)
    end

    alias length size

    # @rbs file_spec: FileSpec
    # @rbs return: void
    def append(file_spec)
      ensure_open!
      raise ArgumentError, 'file_spec must be a FileSpec' unless file_spec.is_a?(FileSpec)

      FFIBindings.lldb_file_spec_list_append(@ptr, file_spec.to_ptr)
    end

    # @rbs file_spec: FileSpec
    # @rbs return: bool
    def append_if_unique(file_spec)
      ensure_open!
      raise ArgumentError, 'file_spec must be a FileSpec' unless file_spec.is_a?(FileSpec)

      FFIBindings.lldb_file_spec_list_append_if_unique(@ptr, file_spec.to_ptr) != 0
    end

    # @rbs return: void
    def clear
      ensure_open!
      FFIBindings.lldb_file_spec_list_clear(@ptr)
    end

    # @rbs index: Integer
    # @rbs return: FileSpec?
    def [](index)
      return nil unless valid?

      ptr = FFIBindings.lldb_file_spec_list_get_file_spec_at_index(@ptr, index)
      return nil if ptr.nil? || ptr.null?

      FileSpec.from_ptr(ptr, context: context)
    end

    # @rbs return: Array[FileSpec]
    def to_a
      (0...size).map { |index| self[index] }.compact
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
