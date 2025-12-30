# frozen_string_literal: true

require 'ffi'
require 'rbconfig'

module LLDB
  module FFIBindings
    extend FFI::Library

    class << self
      # @rbs return: String
      def library_name
        case RbConfig::CONFIG['host_os']
        when /darwin/
          'liblldb_wrapper.dylib'
        when /linux/
          'liblldb_wrapper.so'
        when /mswin|mingw/
          'lldb_wrapper.dll'
        else
          raise "Unsupported platform: #{RbConfig::CONFIG['host_os']}"
        end
      end

      # @rbs return: String
      def library_path
        current_dir = __dir__ || File.dirname(__FILE__)

        search_paths = [
          File.expand_path(current_dir),
          File.expand_path('../lldb', current_dir),
          File.join(current_dir, '..'),
          current_dir
        ]

        # steep:ignore:start
        if defined?(Gem) && Gem.loaded_specs['lldb']
          spec = Gem.loaded_specs['lldb']
          search_paths << File.join(spec.full_gem_path, 'lib', 'lldb') if spec.respond_to?(:full_gem_path)
        end
        # steep:ignore:end

        search_paths.compact.each do |path|
          lib_path = File.join(path, library_name)
          return lib_path if File.exist?(lib_path)
        end

        library_name
      end
    end

    ffi_lib library_path

    # =========================================================================
    # Initialization
    # =========================================================================
    attach_function :lldb_initialize, [], :void
    attach_function :lldb_terminate, [], :void

    # =========================================================================
    # SBDebugger
    # =========================================================================
    attach_function :lldb_debugger_create, [], :pointer
    attach_function :lldb_debugger_destroy, [:pointer], :void
    attach_function :lldb_debugger_is_valid, [:pointer], :int
    attach_function :lldb_debugger_create_target, %i[pointer string string string int pointer], :pointer
    attach_function :lldb_debugger_create_target_simple, %i[pointer string], :pointer
    attach_function :lldb_debugger_get_num_targets, [:pointer], :uint32
    attach_function :lldb_debugger_get_target_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_debugger_get_selected_target, [:pointer], :pointer
    attach_function :lldb_debugger_set_selected_target, %i[pointer pointer], :void
    attach_function :lldb_debugger_delete_target, %i[pointer pointer], :int
    attach_function :lldb_debugger_find_target_with_process_id, %i[pointer uint64], :pointer
    attach_function :lldb_debugger_set_async, %i[pointer int], :void
    attach_function :lldb_debugger_get_async, [:pointer], :int
    attach_function :lldb_debugger_get_version_string, [], :string
    attach_function :lldb_debugger_get_command_interpreter, [:pointer], :pointer
    attach_function :lldb_debugger_handle_command, %i[pointer string], :void

    # =========================================================================
    # SBTarget
    # =========================================================================
    attach_function :lldb_target_destroy, [:pointer], :void
    attach_function :lldb_target_is_valid, [:pointer], :int
    attach_function :lldb_target_launch_simple, %i[pointer pointer pointer string], :pointer
    attach_function :lldb_target_launch, %i[pointer pointer pointer], :pointer
    attach_function :lldb_target_attach_to_process_with_id, %i[pointer uint64 pointer], :pointer
    attach_function :lldb_target_attach_to_process_with_name, %i[pointer string int pointer], :pointer
    attach_function :lldb_target_breakpoint_create_by_name, %i[pointer string string], :pointer
    attach_function :lldb_target_breakpoint_create_by_location, %i[pointer string uint32], :pointer
    attach_function :lldb_target_breakpoint_create_by_address, %i[pointer uint64], :pointer
    attach_function :lldb_target_breakpoint_create_by_regex, %i[pointer string string], :pointer
    attach_function :lldb_target_breakpoint_create_by_source_regex, %i[pointer string string], :pointer
    attach_function :lldb_target_delete_breakpoint, %i[pointer int32], :int
    attach_function :lldb_target_delete_all_breakpoints, [:pointer], :int
    attach_function :lldb_target_enable_all_breakpoints, [:pointer], :int
    attach_function :lldb_target_disable_all_breakpoints, [:pointer], :int
    attach_function :lldb_target_find_breakpoint_by_id, %i[pointer int32], :pointer
    attach_function :lldb_target_get_num_breakpoints, [:pointer], :uint32
    attach_function :lldb_target_get_breakpoint_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_target_get_process, [:pointer], :pointer
    attach_function :lldb_target_get_executable_path, [:pointer], :string
    attach_function :lldb_target_get_num_modules, [:pointer], :uint32
    attach_function :lldb_target_get_module_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_target_evaluate_expression, %i[pointer string], :pointer
    attach_function :lldb_target_read_memory, %i[pointer uint64 pointer size_t pointer], :size_t
    attach_function :lldb_target_get_address_byte_size, [:pointer], :uint32
    attach_function :lldb_target_get_triple, [:pointer], :string
    attach_function :lldb_target_watch_address, %i[pointer uint64 size_t int int pointer], :pointer
    attach_function :lldb_target_delete_watchpoint, %i[pointer int32], :int
    attach_function :lldb_target_delete_all_watchpoints, [:pointer], :int
    attach_function :lldb_target_find_watchpoint_by_id, %i[pointer int32], :pointer
    attach_function :lldb_target_get_num_watchpoints, [:pointer], :uint32
    attach_function :lldb_target_get_watchpoint_at_index, %i[pointer uint32], :pointer

    # =========================================================================
    # SBLaunchInfo
    # =========================================================================
    attach_function :lldb_launch_info_create, [:pointer], :pointer
    attach_function :lldb_launch_info_destroy, [:pointer], :void
    attach_function :lldb_launch_info_set_working_directory, %i[pointer string], :void
    attach_function :lldb_launch_info_set_environment_entries, %i[pointer pointer int], :void
    attach_function :lldb_launch_info_get_launch_flags, [:pointer], :uint32
    attach_function :lldb_launch_info_set_launch_flags, %i[pointer uint32], :void

    # =========================================================================
    # SBProcess
    # =========================================================================
    attach_function :lldb_process_destroy, [:pointer], :void
    attach_function :lldb_process_is_valid, [:pointer], :int
    attach_function :lldb_process_continue, [:pointer], :int
    attach_function :lldb_process_stop, [:pointer], :int
    attach_function :lldb_process_kill, [:pointer], :int
    attach_function :lldb_process_detach, [:pointer], :int
    attach_function :lldb_process_destroy_process, [:pointer], :int
    attach_function :lldb_process_signal, %i[pointer int], :int
    attach_function :lldb_process_get_state, [:pointer], :int
    attach_function :lldb_process_get_num_threads, [:pointer], :uint32
    attach_function :lldb_process_get_thread_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_process_get_thread_by_id, %i[pointer uint64], :pointer
    attach_function :lldb_process_get_thread_by_index_id, %i[pointer uint32], :pointer
    attach_function :lldb_process_get_selected_thread, [:pointer], :pointer
    attach_function :lldb_process_set_selected_thread_by_id, %i[pointer uint64], :int
    attach_function :lldb_process_set_selected_thread_by_index_id, %i[pointer uint32], :int
    attach_function :lldb_process_get_process_id, [:pointer], :uint64
    attach_function :lldb_process_get_exit_status, [:pointer], :int
    attach_function :lldb_process_get_exit_description, [:pointer], :string
    attach_function :lldb_process_read_memory, %i[pointer uint64 pointer size_t pointer], :size_t
    attach_function :lldb_process_write_memory, %i[pointer uint64 pointer size_t pointer], :size_t
    attach_function :lldb_process_allocate_memory, %i[pointer size_t uint32 pointer], :uint64
    attach_function :lldb_process_deallocate_memory, %i[pointer uint64], :int
    attach_function :lldb_process_read_cstring_from_memory, %i[pointer uint64 pointer size_t pointer], :size_t
    attach_function :lldb_process_get_stdout, %i[pointer pointer size_t], :size_t
    attach_function :lldb_process_get_stderr, %i[pointer pointer size_t], :size_t
    attach_function :lldb_process_put_stdin, %i[pointer string size_t], :size_t
    attach_function :lldb_process_send_async_interrupt, [:pointer], :int
    attach_function :lldb_process_get_num_supported_hardware_watchpoints, %i[pointer pointer], :uint32
    attach_function :lldb_process_get_unique_id, [:pointer], :uint32

    # =========================================================================
    # SBThread
    # =========================================================================
    attach_function :lldb_thread_destroy, [:pointer], :void
    attach_function :lldb_thread_is_valid, [:pointer], :int
    attach_function :lldb_thread_step_over, [:pointer], :int
    attach_function :lldb_thread_step_into, [:pointer], :int
    attach_function :lldb_thread_step_out, [:pointer], :int
    attach_function :lldb_thread_step_instruction, %i[pointer int], :int
    attach_function :lldb_thread_run_to_address, %i[pointer uint64], :int
    attach_function :lldb_thread_get_num_frames, [:pointer], :uint32
    attach_function :lldb_thread_get_frame_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_thread_get_selected_frame, [:pointer], :pointer
    attach_function :lldb_thread_set_selected_frame, %i[pointer uint32], :int
    attach_function :lldb_thread_get_thread_id, [:pointer], :uint64
    attach_function :lldb_thread_get_index_id, [:pointer], :uint32
    attach_function :lldb_thread_get_name, [:pointer], :string
    attach_function :lldb_thread_get_queue_name, [:pointer], :string
    attach_function :lldb_thread_get_stop_reason, [:pointer], :int
    attach_function :lldb_thread_get_stop_description, %i[pointer size_t], :string
    attach_function :lldb_thread_get_stop_reason_data_count, [:pointer], :uint64
    attach_function :lldb_thread_get_stop_reason_data_at_index, %i[pointer uint32], :uint64
    attach_function :lldb_thread_is_stopped, [:pointer], :int
    attach_function :lldb_thread_is_suspended, [:pointer], :int
    attach_function :lldb_thread_suspend, [:pointer], :int
    attach_function :lldb_thread_resume, [:pointer], :int
    attach_function :lldb_thread_get_process, [:pointer], :pointer

    # =========================================================================
    # SBFrame
    # =========================================================================
    attach_function :lldb_frame_destroy, [:pointer], :void
    attach_function :lldb_frame_is_valid, [:pointer], :int
    attach_function :lldb_frame_get_function_name, [:pointer], :string
    attach_function :lldb_frame_get_display_function_name, [:pointer], :string
    attach_function :lldb_frame_get_line, [:pointer], :uint32
    attach_function :lldb_frame_get_file_path, [:pointer], :string
    attach_function :lldb_frame_get_column, [:pointer], :uint32
    attach_function :lldb_frame_get_pc, [:pointer], :uint64
    attach_function :lldb_frame_set_pc, %i[pointer uint64], :int
    attach_function :lldb_frame_get_sp, [:pointer], :uint64
    attach_function :lldb_frame_get_fp, [:pointer], :uint64
    attach_function :lldb_frame_find_variable, %i[pointer string], :pointer
    attach_function :lldb_frame_evaluate_expression, %i[pointer string], :pointer
    attach_function :lldb_frame_get_value_for_variable_path, %i[pointer string], :pointer
    attach_function :lldb_frame_get_frame_id, [:pointer], :uint32
    attach_function :lldb_frame_get_thread, [:pointer], :pointer
    attach_function :lldb_frame_get_symbol_context, %i[pointer uint32], :pointer
    attach_function :lldb_frame_get_variables, %i[pointer int int int int], :pointer
    attach_function :lldb_frame_get_registers, [:pointer], :pointer
    attach_function :lldb_frame_is_inlined, [:pointer], :int
    attach_function :lldb_frame_disassemble, [:pointer], :string
    attach_function :lldb_frame_get_module, [:pointer], :pointer

    # =========================================================================
    # SBBreakpoint
    # =========================================================================
    attach_function :lldb_breakpoint_destroy, [:pointer], :void
    attach_function :lldb_breakpoint_is_valid, [:pointer], :int
    attach_function :lldb_breakpoint_get_id, [:pointer], :int32
    attach_function :lldb_breakpoint_is_enabled, [:pointer], :int
    attach_function :lldb_breakpoint_set_enabled, %i[pointer int], :void
    attach_function :lldb_breakpoint_is_one_shot, [:pointer], :int
    attach_function :lldb_breakpoint_set_one_shot, %i[pointer int], :void
    attach_function :lldb_breakpoint_get_hit_count, [:pointer], :uint32
    attach_function :lldb_breakpoint_get_ignore_count, [:pointer], :uint32
    attach_function :lldb_breakpoint_set_ignore_count, %i[pointer uint32], :void
    attach_function :lldb_breakpoint_get_condition, [:pointer], :string
    attach_function :lldb_breakpoint_set_condition, %i[pointer string], :void
    attach_function :lldb_breakpoint_get_num_locations, [:pointer], :uint32
    attach_function :lldb_breakpoint_get_location_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_breakpoint_find_location_by_id, %i[pointer int32], :pointer
    attach_function :lldb_breakpoint_is_hardware, [:pointer], :int
    attach_function :lldb_breakpoint_get_auto_continue, [:pointer], :int
    attach_function :lldb_breakpoint_set_auto_continue, %i[pointer int], :void
    attach_function :lldb_breakpoint_get_thread_id, [:pointer], :uint64
    attach_function :lldb_breakpoint_set_thread_id, %i[pointer uint64], :void
    attach_function :lldb_breakpoint_get_thread_name, [:pointer], :string
    attach_function :lldb_breakpoint_set_thread_name, %i[pointer string], :void
    attach_function :lldb_breakpoint_get_thread_index, [:pointer], :uint32
    attach_function :lldb_breakpoint_set_thread_index, %i[pointer uint32], :void

    # =========================================================================
    # SBBreakpointLocation
    # =========================================================================
    attach_function :lldb_breakpoint_location_destroy, [:pointer], :void
    attach_function :lldb_breakpoint_location_is_valid, [:pointer], :int
    attach_function :lldb_breakpoint_location_get_id, [:pointer], :int32
    attach_function :lldb_breakpoint_location_get_load_address, [:pointer], :uint64
    attach_function :lldb_breakpoint_location_is_enabled, [:pointer], :int
    attach_function :lldb_breakpoint_location_set_enabled, %i[pointer int], :void
    attach_function :lldb_breakpoint_location_get_hit_count, [:pointer], :uint32
    attach_function :lldb_breakpoint_location_get_ignore_count, [:pointer], :uint32
    attach_function :lldb_breakpoint_location_set_ignore_count, %i[pointer uint32], :void
    attach_function :lldb_breakpoint_location_get_condition, [:pointer], :string
    attach_function :lldb_breakpoint_location_set_condition, %i[pointer string], :void
    attach_function :lldb_breakpoint_location_get_breakpoint, [:pointer], :pointer

    # =========================================================================
    # SBValue
    # =========================================================================
    attach_function :lldb_value_destroy, [:pointer], :void
    attach_function :lldb_value_is_valid, [:pointer], :int
    attach_function :lldb_value_get_name, [:pointer], :string
    attach_function :lldb_value_get_value, [:pointer], :string
    attach_function :lldb_value_get_summary, [:pointer], :string
    attach_function :lldb_value_get_type_name, [:pointer], :string
    attach_function :lldb_value_get_type, [:pointer], :pointer
    attach_function :lldb_value_get_num_children, [:pointer], :uint32
    attach_function :lldb_value_get_child_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_value_get_child_member_with_name, %i[pointer string], :pointer
    attach_function :lldb_value_get_value_as_signed, [:pointer], :int64
    attach_function :lldb_value_get_value_as_unsigned, [:pointer], :uint64
    attach_function :lldb_value_get_byte_size, [:pointer], :uint64
    attach_function :lldb_value_might_have_children, [:pointer], :int
    attach_function :lldb_value_get_error, %i[pointer pointer], :int
    attach_function :lldb_value_dereference, [:pointer], :pointer
    attach_function :lldb_value_address_of, [:pointer], :pointer
    attach_function :lldb_value_cast, %i[pointer pointer], :pointer
    attach_function :lldb_value_get_load_address, [:pointer], :uint64
    attach_function :lldb_value_get_value_type, [:pointer], :int
    attach_function :lldb_value_set_value_from_cstring, %i[pointer string pointer], :int
    attach_function :lldb_value_create_child_at_offset, %i[pointer string pointer uint32], :pointer
    attach_function :lldb_value_create_value_from_address, %i[pointer string uint64 pointer], :pointer
    attach_function :lldb_value_create_value_from_expression, %i[pointer string string], :pointer
    attach_function :lldb_value_watch, %i[pointer int int int pointer], :pointer
    attach_function :lldb_value_get_expression_path, [:pointer], :string
    attach_function :lldb_value_is_pointer_type, [:pointer], :int
    attach_function :lldb_value_get_non_synthetic_value, [:pointer], :pointer

    # =========================================================================
    # SBValueList
    # =========================================================================
    attach_function :lldb_value_list_destroy, [:pointer], :void
    attach_function :lldb_value_list_is_valid, [:pointer], :int
    attach_function :lldb_value_list_get_size, [:pointer], :uint32
    attach_function :lldb_value_list_get_value_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_value_list_get_first_value_by_name, %i[pointer string], :pointer

    # =========================================================================
    # SBError
    # =========================================================================
    attach_function :lldb_error_create, [], :pointer
    attach_function :lldb_error_destroy, [:pointer], :void
    attach_function :lldb_error_success, [:pointer], :int
    attach_function :lldb_error_fail, [:pointer], :int
    attach_function :lldb_error_get_cstring, [:pointer], :string
    attach_function :lldb_error_get_error, [:pointer], :uint32
    attach_function :lldb_error_clear, [:pointer], :void
    attach_function :lldb_error_set_error_string, %i[pointer string], :void

    # =========================================================================
    # SBModule
    # =========================================================================
    attach_function :lldb_module_destroy, [:pointer], :void
    attach_function :lldb_module_is_valid, [:pointer], :int
    attach_function :lldb_module_get_file_path, [:pointer], :string
    attach_function :lldb_module_get_platform_file_path, [:pointer], :string
    attach_function :lldb_module_get_num_symbols, [:pointer], :uint32

    # =========================================================================
    # SBSymbolContext
    # =========================================================================
    attach_function :lldb_symbol_context_destroy, [:pointer], :void
    attach_function :lldb_symbol_context_is_valid, [:pointer], :int
    attach_function :lldb_symbol_context_get_module, [:pointer], :pointer
    attach_function :lldb_symbol_context_get_function_name, [:pointer], :string

    # =========================================================================
    # SBType
    # =========================================================================
    attach_function :lldb_type_destroy, [:pointer], :void
    attach_function :lldb_type_is_valid, [:pointer], :int
    attach_function :lldb_type_get_name, [:pointer], :string
    attach_function :lldb_type_get_display_type_name, [:pointer], :string
    attach_function :lldb_type_get_byte_size, [:pointer], :uint64
    attach_function :lldb_type_is_pointer_type, [:pointer], :int
    attach_function :lldb_type_is_reference_type, [:pointer], :int
    attach_function :lldb_type_is_array_type, [:pointer], :int
    attach_function :lldb_type_is_vector_type, [:pointer], :int
    attach_function :lldb_type_is_typedef_type, [:pointer], :int
    attach_function :lldb_type_is_function_type, [:pointer], :int
    attach_function :lldb_type_is_polymorphic_class, [:pointer], :int
    attach_function :lldb_type_get_pointer_type, [:pointer], :pointer
    attach_function :lldb_type_get_pointee_type, [:pointer], :pointer
    attach_function :lldb_type_get_reference_type, [:pointer], :pointer
    attach_function :lldb_type_get_dereferenced_type, [:pointer], :pointer
    attach_function :lldb_type_get_unqualified_type, [:pointer], :pointer
    attach_function :lldb_type_get_canonical_type, [:pointer], :pointer
    attach_function :lldb_type_get_array_element_type, [:pointer], :pointer
    attach_function :lldb_type_get_array_size, [:pointer], :uint64
    attach_function :lldb_type_get_num_fields, [:pointer], :uint32
    attach_function :lldb_type_get_num_direct_base_classes, [:pointer], :uint32
    attach_function :lldb_type_get_num_virtual_base_classes, [:pointer], :uint32
    attach_function :lldb_type_get_basic_type, [:pointer], :int

    # =========================================================================
    # SBWatchpoint
    # =========================================================================
    attach_function :lldb_watchpoint_destroy, [:pointer], :void
    attach_function :lldb_watchpoint_is_valid, [:pointer], :int
    attach_function :lldb_watchpoint_get_id, [:pointer], :int32
    attach_function :lldb_watchpoint_is_enabled, [:pointer], :int
    attach_function :lldb_watchpoint_set_enabled, %i[pointer int], :void
    attach_function :lldb_watchpoint_get_hit_count, [:pointer], :uint32
    attach_function :lldb_watchpoint_get_ignore_count, [:pointer], :uint32
    attach_function :lldb_watchpoint_set_ignore_count, %i[pointer uint32], :void
    attach_function :lldb_watchpoint_get_condition, [:pointer], :string
    attach_function :lldb_watchpoint_set_condition, %i[pointer string], :void
    attach_function :lldb_watchpoint_get_watch_address, [:pointer], :uint64
    attach_function :lldb_watchpoint_get_watch_size, [:pointer], :size_t
    attach_function :lldb_watchpoint_is_watching_reads, [:pointer], :int
    attach_function :lldb_watchpoint_is_watching_writes, [:pointer], :int

    # =========================================================================
    # SBCommandInterpreter
    # =========================================================================
    attach_function :lldb_command_interpreter_destroy, [:pointer], :void
    attach_function :lldb_command_interpreter_is_valid, [:pointer], :int
    attach_function :lldb_command_interpreter_handle_command, %i[pointer string pointer int], :int
    attach_function :lldb_command_interpreter_command_exists, %i[pointer string], :int
    attach_function :lldb_command_interpreter_alias_exists, %i[pointer string], :int

    # =========================================================================
    # SBCommandReturnObject
    # =========================================================================
    attach_function :lldb_command_return_object_create, [], :pointer
    attach_function :lldb_command_return_object_destroy, [:pointer], :void
    attach_function :lldb_command_return_object_is_valid, [:pointer], :int
    attach_function :lldb_command_return_object_get_output, [:pointer], :string
    attach_function :lldb_command_return_object_get_error, [:pointer], :string
    attach_function :lldb_command_return_object_succeeded, [:pointer], :int
    attach_function :lldb_command_return_object_clear, [:pointer], :void
  end
end
