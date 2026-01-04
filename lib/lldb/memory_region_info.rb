# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  # Represents information about a memory region in the target process.
  class MemoryRegionInfo
    # @rbs ptr: FFI::Pointer
    # @rbs return: void
    def initialize(ptr)
      @ptr = ptr # : FFI::Pointer
      ObjectSpace.define_finalizer(self, self.class.release(@ptr))
    end

    # @rbs ptr: FFI::Pointer
    # @rbs return: ^(Integer) -> void
    def self.release(ptr)
      ->(_id) { FFIBindings.lldb_memory_region_info_destroy(ptr) unless ptr.null? }
    end

    # Get the base address of the memory region
    #
    # @rbs return: Integer
    def base_address
      FFIBindings.lldb_memory_region_info_get_region_base(@ptr)
    end

    # Get the end address of the memory region
    #
    # @rbs return: Integer
    def end_address
      FFIBindings.lldb_memory_region_info_get_region_end(@ptr)
    end

    # Get the size of the memory region
    #
    # @rbs return: Integer
    def size
      end_address - base_address
    end

    # Check if the memory region is readable
    #
    # @rbs return: bool
    def readable?
      FFIBindings.lldb_memory_region_info_is_readable(@ptr) != 0
    end

    # Check if the memory region is writable
    #
    # @rbs return: bool
    def writable?
      FFIBindings.lldb_memory_region_info_is_writable(@ptr) != 0
    end

    # Check if the memory region is executable
    #
    # @rbs return: bool
    def executable?
      FFIBindings.lldb_memory_region_info_is_executable(@ptr) != 0
    end

    # Check if the memory region is mapped
    #
    # @rbs return: bool
    def mapped?
      FFIBindings.lldb_memory_region_info_is_mapped(@ptr) != 0
    end

    # Get the name of the memory region (if available)
    #
    # @rbs return: String?
    def name
      FFIBindings.lldb_memory_region_info_get_name(@ptr)
    end

    # Get the permissions string (e.g., "rwx", "r-x", etc.)
    #
    # @rbs return: String
    def permissions
      perms = +''
      perms << (readable? ? 'r' : '-')
      perms << (writable? ? 'w' : '-')
      perms << (executable? ? 'x' : '-')
      perms.freeze
    end

    # @rbs return: String
    def to_s
      format(
        '0x%016x-0x%016x %s %s',
        base_address,
        end_address,
        permissions,
        name || ''
      ).strip
    end

    # @rbs return: FFI::Pointer
    def to_ptr
      @ptr
    end
  end
end
