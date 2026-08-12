# frozen_string_literal: true

require 'ffi'
require 'rbconfig'

module LLDB
  module FFIBindings
    extend FFI::Library

    EXPECTED_WRAPPER_ABI_VERSION = 1
    CAPABILITIES = {
      watchpoint_access_kind: 1
    }.freeze

    class << self
      # @rbs return: String
      def library_name
        case RbConfig::CONFIG['host_os']
        when /darwin/
          'liblldb_wrapper.dylib'
        when /linux/
          'liblldb_wrapper.so'
        else
          raise UnsupportedPlatformError,
                "Unsupported platform: #{RbConfig::CONFIG['host_os']} (supported: Linux, macOS)"
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

      # @rbs return: Integer
      def wrapper_abi_version
        lldb_wrapper_abi_version
      end

      # @rbs return: String
      def build_lldb_version
        lldb_wrapper_build_lldb_version
      end

      # @rbs return: String
      def runtime_lldb_version
        lldb_wrapper_runtime_lldb_version
      end

      # @rbs feature: ::Symbol
      # @rbs return: bool
      def capability_supported?(feature)
        capability = CAPABILITIES[feature]
        return false unless capability

        lldb_wrapper_has_capability(capability) != 0
      end
    end

    LIBRARY_PATH = library_path

    ffi_lib LIBRARY_PATH

    begin
      attach_function :lldb_wrapper_abi_version, [], :uint32
      attach_function :lldb_wrapper_build_lldb_version, [], :string
      attach_function :lldb_wrapper_runtime_lldb_version, [], :string
      attach_function :lldb_wrapper_has_capability, [:uint32], :int

      actual_abi_version = lldb_wrapper_abi_version
      if actual_abi_version != EXPECTED_WRAPPER_ABI_VERSION
        raise IncompatibleWrapperError,
              "Incompatible LLDB wrapper at #{LIBRARY_PATH}: " \
              "expected ABI #{EXPECTED_WRAPPER_ABI_VERSION}, got #{actual_abi_version}"
      end
    rescue FFI::NotFoundError, LoadError => e
      raise IncompatibleWrapperError,
            "Incompatible LLDB wrapper at #{LIBRARY_PATH}: metadata is unavailable (#{e.message})"
    end

    # Wrapper metadata and capabilities
    attach_function :lldb_wrapper_abi_version, [], :uint32
    attach_function :lldb_wrapper_build_lldb_version, [], :string
    attach_function :lldb_wrapper_runtime_lldb_version, [], :string
    attach_function :lldb_wrapper_has_capability, [:uint32], :int
    attach_function :lldb_wrapper_last_error_message, [], :string
    attach_function :lldb_wrapper_last_error_code, [], :int
    attach_function :lldb_wrapper_clear_last_error, [], :void

    # =========================================================================
    # Initialization
    # =========================================================================
    attach_function :lldb_initialize, [:pointer], :int
    attach_function :lldb_terminate, [], :void

    # =========================================================================
    # SBDebugger
    # =========================================================================
    attach_function :lldb_debugger_create, [], :pointer
    attach_function :lldb_debugger_create_with_source_init_files, [:int], :pointer
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
    attach_function :lldb_debugger_get_broadcaster, [:pointer], :pointer
    attach_function :lldb_debugger_get_listener, [:pointer], :pointer
    # LLDB command execution can run user-provided command scripts.
    attach_function :lldb_debugger_handle_command, %i[pointer string], :void, blocking: true

    # =========================================================================
    # SBBroadcaster, SBListener, and SBEvent
    # =========================================================================
    attach_function :lldb_broadcaster_create, [:string], :pointer
    attach_function :lldb_broadcaster_destroy, [:pointer], :void
    attach_function :lldb_broadcaster_is_valid, [:pointer], :int
    attach_function :lldb_broadcaster_get_name, [:pointer], :string
    attach_function :lldb_broadcaster_add_listener, %i[pointer pointer uint32], :uint32
    attach_function :lldb_broadcaster_remove_listener, %i[pointer pointer uint32], :int
    attach_function :lldb_broadcaster_event_type_has_listeners, %i[pointer uint32], :int
    attach_function :lldb_broadcaster_broadcast_event_by_type, %i[pointer uint32 int], :void

    attach_function :lldb_listener_create, [:string], :pointer
    attach_function :lldb_listener_destroy, [:pointer], :void
    attach_function :lldb_listener_is_valid, [:pointer], :int
    attach_function :lldb_listener_start_listening_for_events, %i[pointer pointer uint32], :uint32
    attach_function :lldb_listener_stop_listening_for_events, %i[pointer pointer uint32], :int
    attach_function :lldb_listener_wait_for_event, [:pointer, :uint32], :pointer, blocking: true
    attach_function :lldb_listener_peek_event, [:pointer], :pointer
    attach_function :lldb_listener_next_event, [:pointer], :pointer

    attach_function :lldb_event_destroy, [:pointer], :void
    attach_function :lldb_event_is_valid, [:pointer], :int
    attach_function :lldb_event_get_type, [:pointer], :uint32
    attach_function :lldb_event_get_data_flavor, [:pointer], :string
    attach_function :lldb_event_get_broadcaster_class, [:pointer], :string
    attach_function :lldb_event_get_description, %i[pointer pointer size_t], :uint32
    attach_function :lldb_event_get_broadcaster, [:pointer], :pointer
    attach_function :lldb_event_is_process_event, [:pointer], :int
    attach_function :lldb_event_get_process_state, [:pointer], :int
    attach_function :lldb_event_get_restarted, [:pointer], :int
    attach_function :lldb_event_get_interrupted, [:pointer], :int
    attach_function :lldb_event_get_process, [:pointer], :pointer

    # =========================================================================
    # SBTarget
    # =========================================================================
    attach_function :lldb_target_destroy, [:pointer], :void
    attach_function :lldb_target_is_valid, [:pointer], :int
    # Launch and attach may wait for the inferior or its debug server.
    attach_function :lldb_target_launch_simple, %i[pointer pointer pointer string], :pointer, blocking: true
    attach_function :lldb_target_launch, %i[pointer pointer pointer], :pointer, blocking: true
    attach_function :lldb_target_attach_to_process_with_id, %i[pointer uint64 pointer], :pointer, blocking: true
    attach_function :lldb_target_attach_to_process_with_name, %i[pointer string int pointer], :pointer, blocking: true
    attach_function :lldb_target_attach_with_info, %i[pointer pointer pointer], :pointer, blocking: true
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
    attach_function :lldb_target_get_executable_file, [:pointer], :pointer
    attach_function :lldb_target_get_num_modules, [:pointer], :uint32
    attach_function :lldb_target_get_module_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_target_evaluate_expression, %i[pointer string], :pointer
    attach_function :lldb_target_evaluate_expression_with_options, %i[pointer string pointer], :pointer
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
    attach_function :lldb_launch_info_get_num_arguments, [:pointer], :uint32
    attach_function :lldb_launch_info_get_argument_at_index, %i[pointer uint32], :string
    attach_function :lldb_launch_info_set_working_directory, %i[pointer string], :void
    attach_function :lldb_launch_info_get_working_directory, [:pointer], :string
    attach_function :lldb_launch_info_set_environment_entries, %i[pointer pointer int], :void
    attach_function :lldb_launch_info_get_num_environment_entries, [:pointer], :uint32
    attach_function :lldb_launch_info_get_environment_entry_at_index, %i[pointer uint32], :string
    attach_function :lldb_launch_info_get_launch_flags, [:pointer], :uint32
    attach_function :lldb_launch_info_set_launch_flags, %i[pointer uint32], :void
    attach_function :lldb_launch_info_set_arguments, %i[pointer pointer int], :void
    attach_function :lldb_launch_info_get_executable_file, [:pointer], :pointer
    attach_function :lldb_launch_info_set_executable_file, %i[pointer pointer int], :void
    attach_function :lldb_launch_info_get_listener, [:pointer], :pointer
    attach_function :lldb_launch_info_set_listener, %i[pointer pointer], :void
    attach_function :lldb_launch_info_get_process_plugin_name, [:pointer], :string
    attach_function :lldb_launch_info_set_process_plugin_name, %i[pointer string], :void
    attach_function :lldb_launch_info_get_shell, [:pointer], :string
    attach_function :lldb_launch_info_set_shell, %i[pointer string], :void
    attach_function :lldb_launch_info_add_close_file_action, %i[pointer int], :int
    attach_function :lldb_launch_info_add_duplicate_file_action, %i[pointer int int], :int
    attach_function :lldb_launch_info_add_open_file_action, %i[pointer int string int int], :int
    attach_function :lldb_launch_info_add_suppress_file_action, %i[pointer int int int], :int

    # SBAttachInfo
    attach_function :lldb_attach_info_create, [:uint64], :pointer
    attach_function :lldb_attach_info_destroy, [:pointer], :void
    attach_function :lldb_attach_info_get_process_id, [:pointer], :uint64
    attach_function :lldb_attach_info_set_process_id, %i[pointer uint64], :void
    attach_function :lldb_attach_info_set_executable, %i[pointer string], :void
    attach_function :lldb_attach_info_set_executable_file, %i[pointer pointer], :void
    attach_function :lldb_attach_info_get_wait_for_launch, [:pointer], :int
    attach_function :lldb_attach_info_set_wait_for_launch, %i[pointer int], :void
    attach_function :lldb_attach_info_get_ignore_existing, [:pointer], :int
    attach_function :lldb_attach_info_set_ignore_existing, %i[pointer int], :void
    attach_function :lldb_attach_info_get_resume_count, [:pointer], :uint32
    attach_function :lldb_attach_info_set_resume_count, %i[pointer uint32], :void
    attach_function :lldb_attach_info_get_process_plugin_name, [:pointer], :string
    attach_function :lldb_attach_info_set_process_plugin_name, %i[pointer string], :void
    attach_function :lldb_attach_info_get_listener, [:pointer], :pointer
    attach_function :lldb_attach_info_set_listener, %i[pointer pointer], :void

    # SBExpressionOptions
    attach_function :lldb_expression_options_create, [], :pointer
    attach_function :lldb_expression_options_destroy, [:pointer], :void
    attach_function :lldb_expression_options_get_timeout, [:pointer], :uint32
    attach_function :lldb_expression_options_set_timeout, %i[pointer uint32], :void
    attach_function :lldb_expression_options_get_unwind_on_error, [:pointer], :int
    attach_function :lldb_expression_options_set_unwind_on_error, %i[pointer int], :void
    attach_function :lldb_expression_options_get_ignore_breakpoints, [:pointer], :int
    attach_function :lldb_expression_options_set_ignore_breakpoints, %i[pointer int], :void
    attach_function :lldb_expression_options_get_fetch_dynamic_value, [:pointer], :int
    attach_function :lldb_expression_options_set_fetch_dynamic_value, %i[pointer int], :void
    attach_function :lldb_expression_options_get_try_all_threads, [:pointer], :int
    attach_function :lldb_expression_options_set_try_all_threads, %i[pointer int], :void
    attach_function :lldb_expression_options_get_stop_others, [:pointer], :int
    attach_function :lldb_expression_options_set_stop_others, %i[pointer int], :void
    attach_function :lldb_expression_options_set_language, %i[pointer int], :void
    attach_function :lldb_expression_options_get_suppress_persistent_result, [:pointer], :int
    attach_function :lldb_expression_options_set_suppress_persistent_result, %i[pointer int], :void

    # =========================================================================
    # SBFileSpec
    # =========================================================================
    attach_function :lldb_file_spec_create, %i[string int], :pointer
    attach_function :lldb_file_spec_destroy, [:pointer], :void
    attach_function :lldb_file_spec_is_valid, [:pointer], :int
    attach_function :lldb_file_spec_exists, [:pointer], :int
    attach_function :lldb_file_spec_get_filename, [:pointer], :string
    attach_function :lldb_file_spec_get_directory, [:pointer], :string
    attach_function :lldb_file_spec_get_path, %i[pointer pointer size_t], :uint32
    attach_function :lldb_file_spec_set_filename, %i[pointer string], :void
    attach_function :lldb_file_spec_set_directory, %i[pointer string], :void
    attach_function :lldb_file_spec_list_create, [], :pointer
    attach_function :lldb_file_spec_list_destroy, [:pointer], :void
    attach_function :lldb_file_spec_list_is_valid, [:pointer], :int
    attach_function :lldb_file_spec_list_get_size, [:pointer], :uint32
    attach_function :lldb_file_spec_list_append, %i[pointer pointer], :void
    attach_function :lldb_file_spec_list_append_if_unique, %i[pointer pointer], :int
    attach_function :lldb_file_spec_list_clear, [:pointer], :void
    attach_function :lldb_file_spec_list_get_file_spec_at_index, %i[pointer uint32], :pointer

    # =========================================================================
    # SBAddress and SBLineEntry
    # =========================================================================
    attach_function :lldb_address_create, [], :pointer
    attach_function :lldb_address_create_from_load_address, %i[uint64 pointer], :pointer
    attach_function :lldb_address_destroy, [:pointer], :void
    attach_function :lldb_address_is_valid, [:pointer], :int
    attach_function :lldb_address_get_file_address, [:pointer], :uint64
    attach_function :lldb_address_get_load_address, %i[pointer pointer], :uint64
    attach_function :lldb_address_get_offset, [:pointer], :uint64
    attach_function :lldb_address_get_line_entry, [:pointer], :pointer
    attach_function :lldb_line_entry_destroy, [:pointer], :void
    attach_function :lldb_line_entry_is_valid, [:pointer], :int
    attach_function :lldb_line_entry_get_start_address, [:pointer], :pointer
    attach_function :lldb_line_entry_get_end_address, [:pointer], :pointer
    attach_function :lldb_line_entry_get_file_spec, [:pointer], :pointer
    attach_function :lldb_line_entry_get_line, [:pointer], :uint32
    attach_function :lldb_line_entry_get_column, [:pointer], :uint32

    # =========================================================================
    # SBProcess
    # =========================================================================
    attach_function :lldb_process_destroy, [:pointer], :void
    attach_function :lldb_process_is_valid, [:pointer], :int
    attach_function :lldb_process_continue, %i[pointer pointer], :int, blocking: true
    attach_function :lldb_process_stop, %i[pointer pointer], :int
    attach_function :lldb_process_kill, %i[pointer pointer], :int
    attach_function :lldb_process_detach, %i[pointer pointer], :int
    attach_function :lldb_process_destroy_process, %i[pointer pointer], :int
    attach_function :lldb_process_signal, %i[pointer int pointer], :int
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
    attach_function :lldb_process_deallocate_memory, %i[pointer uint64 pointer], :int
    attach_function :lldb_process_read_cstring_from_memory, %i[pointer uint64 pointer size_t pointer], :size_t
    attach_function :lldb_process_get_stdout, %i[pointer pointer size_t], :size_t
    attach_function :lldb_process_get_stderr, %i[pointer pointer size_t], :size_t
    attach_function :lldb_process_put_stdin, %i[pointer pointer size_t], :size_t
    attach_function :lldb_process_send_async_interrupt, [:pointer], :void
    attach_function :lldb_process_get_broadcaster, [:pointer], :pointer
    attach_function :lldb_process_get_num_supported_hardware_watchpoints, %i[pointer pointer pointer], :int
    attach_function :lldb_process_get_unique_id, [:pointer], :uint32
    attach_function :lldb_process_get_memory_region_info, %i[pointer uint64 pointer], :pointer

    # =========================================================================
    # SBMemoryRegionInfo
    # =========================================================================
    attach_function :lldb_memory_region_info_destroy, [:pointer], :void
    attach_function :lldb_memory_region_info_get_region_base, [:pointer], :uint64
    attach_function :lldb_memory_region_info_get_region_end, [:pointer], :uint64
    attach_function :lldb_memory_region_info_is_readable, [:pointer], :int
    attach_function :lldb_memory_region_info_is_writable, [:pointer], :int
    attach_function :lldb_memory_region_info_is_executable, [:pointer], :int
    attach_function :lldb_memory_region_info_is_mapped, [:pointer], :int
    attach_function :lldb_memory_region_info_get_name, [:pointer], :string

    # =========================================================================
    # SBThread
    # =========================================================================
    attach_function :lldb_thread_destroy, [:pointer], :void
    attach_function :lldb_thread_is_valid, [:pointer], :int
    attach_function :lldb_thread_step_over, %i[pointer int pointer], :int, blocking: true
    attach_function :lldb_thread_step_into, %i[pointer string uint32 int pointer], :int, blocking: true
    attach_function :lldb_thread_step_out, %i[pointer pointer], :int, blocking: true
    attach_function :lldb_thread_step_instruction, %i[pointer int pointer], :int, blocking: true
    attach_function :lldb_thread_run_to_address, %i[pointer uint64 pointer], :int, blocking: true
    attach_function :lldb_thread_get_num_frames, [:pointer], :uint32
    attach_function :lldb_thread_get_frame_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_thread_get_selected_frame, [:pointer], :pointer
    attach_function :lldb_thread_set_selected_frame, %i[pointer uint32], :int
    attach_function :lldb_thread_get_thread_id, [:pointer], :uint64
    attach_function :lldb_thread_get_index_id, [:pointer], :uint32
    attach_function :lldb_thread_get_name, [:pointer], :string
    attach_function :lldb_thread_get_queue_name, [:pointer], :string
    attach_function :lldb_thread_get_stop_reason, [:pointer], :int
    attach_function :lldb_thread_get_stop_description, %i[pointer pointer size_t], :size_t
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
    attach_function :lldb_frame_get_file_spec, [:pointer], :pointer
    attach_function :lldb_frame_get_line_entry, [:pointer], :pointer
    attach_function :lldb_frame_get_column, [:pointer], :uint32
    attach_function :lldb_frame_get_pc, [:pointer], :uint64
    attach_function :lldb_frame_set_pc, %i[pointer uint64], :int
    attach_function :lldb_frame_get_sp, [:pointer], :uint64
    attach_function :lldb_frame_get_fp, [:pointer], :uint64
    attach_function :lldb_frame_find_variable, %i[pointer string], :pointer
    attach_function :lldb_frame_evaluate_expression, %i[pointer string], :pointer
    attach_function :lldb_frame_evaluate_expression_with_options, %i[pointer string pointer], :pointer
    attach_function :lldb_frame_get_value_for_variable_path, %i[pointer string], :pointer
    attach_function :lldb_frame_get_frame_id, [:pointer], :uint32
    attach_function :lldb_frame_get_thread, [:pointer], :pointer
    attach_function :lldb_frame_get_function, [:pointer], :pointer
    attach_function :lldb_frame_get_symbol, [:pointer], :pointer
    attach_function :lldb_frame_get_compile_unit, [:pointer], :pointer
    attach_function :lldb_frame_get_block, [:pointer], :pointer
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
    attach_function :lldb_breakpoint_location_get_address, [:pointer], :pointer
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
    attach_function :lldb_value_get_value_as_signed, %i[pointer pointer int64], :int64
    attach_function :lldb_value_get_value_as_unsigned, %i[pointer pointer uint64], :uint64
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
    attach_function :lldb_error_get_type, [:pointer], :int
    attach_function :lldb_error_clear, [:pointer], :void
    attach_function :lldb_error_set_error_string, %i[pointer string], :void

    # =========================================================================
    # SBModule
    # =========================================================================
    attach_function :lldb_module_destroy, [:pointer], :void
    attach_function :lldb_module_is_valid, [:pointer], :int
    attach_function :lldb_module_get_file_path, [:pointer], :string
    attach_function :lldb_module_get_platform_file_path, [:pointer], :string
    attach_function :lldb_module_get_file, [:pointer], :pointer
    attach_function :lldb_module_get_platform_file, [:pointer], :pointer
    attach_function :lldb_module_get_num_symbols, [:pointer], :uint32
    attach_function :lldb_module_get_symbol_at_index, %i[pointer uint32], :pointer

    # =========================================================================
    # SBSymbol, SBFunction, SBCompileUnit, and SBBlock
    # =========================================================================
    attach_function :lldb_symbol_destroy, [:pointer], :void
    attach_function :lldb_symbol_is_valid, [:pointer], :int
    attach_function :lldb_symbol_get_name, [:pointer], :string
    attach_function :lldb_symbol_get_display_name, [:pointer], :string
    attach_function :lldb_symbol_get_mangled_name, [:pointer], :string
    attach_function :lldb_symbol_get_base_name, [:pointer], :string
    attach_function :lldb_symbol_get_start_address, [:pointer], :pointer
    attach_function :lldb_symbol_get_end_address, [:pointer], :pointer
    attach_function :lldb_symbol_get_value, [:pointer], :uint64
    attach_function :lldb_symbol_get_size, [:pointer], :uint64
    attach_function :lldb_symbol_get_prologue_byte_size, [:pointer], :uint32
    attach_function :lldb_symbol_get_type, [:pointer], :int
    attach_function :lldb_symbol_get_id, [:pointer], :uint32
    attach_function :lldb_function_destroy, [:pointer], :void
    attach_function :lldb_function_is_valid, [:pointer], :int
    attach_function :lldb_function_get_name, [:pointer], :string
    attach_function :lldb_function_get_display_name, [:pointer], :string
    attach_function :lldb_function_get_mangled_name, [:pointer], :string
    attach_function :lldb_function_get_base_name, [:pointer], :string
    attach_function :lldb_function_get_start_address, [:pointer], :pointer
    attach_function :lldb_function_get_end_address, [:pointer], :pointer
    attach_function :lldb_function_get_prologue_byte_size, [:pointer], :uint32
    attach_function :lldb_function_get_type, [:pointer], :pointer
    attach_function :lldb_function_get_block, [:pointer], :pointer
    attach_function :lldb_function_is_optimized, [:pointer], :int
    attach_function :lldb_function_get_language, [:pointer], :int
    attach_function :lldb_compile_unit_destroy, [:pointer], :void
    attach_function :lldb_compile_unit_is_valid, [:pointer], :int
    attach_function :lldb_compile_unit_get_file_spec, [:pointer], :pointer
    attach_function :lldb_compile_unit_get_num_line_entries, [:pointer], :uint32
    attach_function :lldb_compile_unit_get_line_entry_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_block_destroy, [:pointer], :void
    attach_function :lldb_block_is_valid, [:pointer], :int
    attach_function :lldb_block_get_inlined_name, [:pointer], :string
    attach_function :lldb_block_get_inlined_call_site_file, [:pointer], :pointer
    attach_function :lldb_block_get_inlined_call_site_line, [:pointer], :uint32
    attach_function :lldb_block_get_inlined_call_site_column, [:pointer], :uint32
    attach_function :lldb_block_get_parent, [:pointer], :pointer
    attach_function :lldb_block_get_sibling, [:pointer], :pointer
    attach_function :lldb_block_get_first_child, [:pointer], :pointer
    attach_function :lldb_block_get_num_ranges, [:pointer], :uint32
    attach_function :lldb_block_get_range_start_address, %i[pointer uint32], :pointer
    attach_function :lldb_block_get_range_end_address, %i[pointer uint32], :pointer

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
    attach_function :lldb_type_get_field_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_type_get_direct_base_class_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_type_get_virtual_base_class_at_index, %i[pointer uint32], :pointer
    attach_function :lldb_type_get_basic_type, [:pointer], :int
    attach_function :lldb_type_member_destroy, [:pointer], :void
    attach_function :lldb_type_member_is_valid, [:pointer], :int
    attach_function :lldb_type_member_get_name, [:pointer], :string
    attach_function :lldb_type_member_get_type, [:pointer], :pointer
    attach_function :lldb_type_member_get_offset_in_bytes, [:pointer], :uint64
    attach_function :lldb_type_member_get_offset_in_bits, [:pointer], :uint64
    attach_function :lldb_type_member_get_bitfield_size_in_bits, [:pointer], :uint32

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
    attach_function :lldb_watchpoint_is_watching_reads, %i[pointer pointer], :int
    attach_function :lldb_watchpoint_is_watching_writes, %i[pointer pointer], :int

    # =========================================================================
    # SBCommandInterpreter
    # =========================================================================
    attach_function :lldb_command_interpreter_destroy, [:pointer], :void
    attach_function :lldb_command_interpreter_is_valid, [:pointer], :int
    attach_function :lldb_command_interpreter_handle_command, %i[pointer string pointer int], :int, blocking: true
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
    attach_function :lldb_command_return_object_get_status, [:pointer], :int
    attach_function :lldb_command_return_object_succeeded, [:pointer], :int
    attach_function :lldb_command_return_object_has_result, [:pointer], :int
    attach_function :lldb_command_return_object_clear, [:pointer], :void
  end
end
