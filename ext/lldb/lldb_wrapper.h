#ifndef LLDB_WRAPPER_H
#define LLDB_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// Opaque pointer types
typedef void* lldb_debugger_t;
typedef void* lldb_target_t;
typedef void* lldb_process_t;
typedef void* lldb_thread_t;
typedef void* lldb_frame_t;
typedef void* lldb_breakpoint_t;
typedef void* lldb_breakpoint_location_t;
typedef void* lldb_value_t;
typedef void* lldb_value_list_t;
typedef void* lldb_error_t;
typedef void* lldb_module_t;
typedef void* lldb_symbol_context_t;
typedef void* lldb_launch_info_t;
typedef void* lldb_type_t;
typedef void* lldb_watchpoint_t;
typedef void* lldb_command_interpreter_t;
typedef void* lldb_command_return_object_t;
typedef void* lldb_memory_region_info_t;

// Initialization
void lldb_initialize(void);
void lldb_terminate(void);

// SBDebugger
lldb_debugger_t lldb_debugger_create(void);
void lldb_debugger_destroy(lldb_debugger_t dbg);
int lldb_debugger_is_valid(lldb_debugger_t dbg);
lldb_target_t lldb_debugger_create_target(lldb_debugger_t dbg,
                                           const char* filename,
                                           const char* arch,
                                           const char* platform,
                                           int add_dependent_modules,
                                           lldb_error_t error);
lldb_target_t lldb_debugger_create_target_simple(lldb_debugger_t dbg,
                                                  const char* filename);
uint32_t lldb_debugger_get_num_targets(lldb_debugger_t dbg);
lldb_target_t lldb_debugger_get_target_at_index(lldb_debugger_t dbg, uint32_t index);
lldb_target_t lldb_debugger_get_selected_target(lldb_debugger_t dbg);
void lldb_debugger_set_selected_target(lldb_debugger_t dbg, lldb_target_t target);
int lldb_debugger_delete_target(lldb_debugger_t dbg, lldb_target_t target);
lldb_target_t lldb_debugger_find_target_with_process_id(lldb_debugger_t dbg, uint64_t pid);
void lldb_debugger_set_async(lldb_debugger_t dbg, int async);
int lldb_debugger_get_async(lldb_debugger_t dbg);
const char* lldb_debugger_get_version_string(void);
lldb_command_interpreter_t lldb_debugger_get_command_interpreter(lldb_debugger_t dbg);
void lldb_debugger_handle_command(lldb_debugger_t dbg, const char* command);

// SBTarget
void lldb_target_destroy(lldb_target_t target);
int lldb_target_is_valid(lldb_target_t target);
lldb_process_t lldb_target_launch_simple(lldb_target_t target,
                                          const char** argv,
                                          const char** envp,
                                          const char* working_dir);
lldb_process_t lldb_target_launch(lldb_target_t target,
                                   lldb_launch_info_t launch_info,
                                   lldb_error_t error);
lldb_process_t lldb_target_attach_to_process_with_id(lldb_target_t target,
                                                      uint64_t pid,
                                                      lldb_error_t error);
lldb_process_t lldb_target_attach_to_process_with_name(lldb_target_t target,
                                                        const char* name,
                                                        int wait_for,
                                                        lldb_error_t error);
lldb_breakpoint_t lldb_target_breakpoint_create_by_name(lldb_target_t target,
                                                         const char* symbol_name,
                                                         const char* module_name);
lldb_breakpoint_t lldb_target_breakpoint_create_by_location(lldb_target_t target,
                                                             const char* file,
                                                             uint32_t line);
lldb_breakpoint_t lldb_target_breakpoint_create_by_address(lldb_target_t target,
                                                            uint64_t address);
lldb_breakpoint_t lldb_target_breakpoint_create_by_regex(lldb_target_t target,
                                                          const char* symbol_regex,
                                                          const char* module_name);
lldb_breakpoint_t lldb_target_breakpoint_create_by_source_regex(lldb_target_t target,
                                                                 const char* source_regex,
                                                                 const char* source_file);
int lldb_target_delete_breakpoint(lldb_target_t target, int32_t breakpoint_id);
int lldb_target_delete_all_breakpoints(lldb_target_t target);
int lldb_target_enable_all_breakpoints(lldb_target_t target);
int lldb_target_disable_all_breakpoints(lldb_target_t target);
lldb_breakpoint_t lldb_target_find_breakpoint_by_id(lldb_target_t target, int32_t id);
uint32_t lldb_target_get_num_breakpoints(lldb_target_t target);
lldb_breakpoint_t lldb_target_get_breakpoint_at_index(lldb_target_t target, uint32_t index);
lldb_process_t lldb_target_get_process(lldb_target_t target);
const char* lldb_target_get_executable_path(lldb_target_t target);
uint32_t lldb_target_get_num_modules(lldb_target_t target);
lldb_module_t lldb_target_get_module_at_index(lldb_target_t target, uint32_t index);
lldb_value_t lldb_target_evaluate_expression(lldb_target_t target, const char* expr);
size_t lldb_target_read_memory(lldb_target_t target, uint64_t addr, void* buf, size_t size, lldb_error_t error);
uint32_t lldb_target_get_address_byte_size(lldb_target_t target);
const char* lldb_target_get_triple(lldb_target_t target);
lldb_watchpoint_t lldb_target_watch_address(lldb_target_t target, uint64_t addr, size_t size, int read, int write, lldb_error_t error);
int lldb_target_delete_watchpoint(lldb_target_t target, int32_t watchpoint_id);
int lldb_target_delete_all_watchpoints(lldb_target_t target);
lldb_watchpoint_t lldb_target_find_watchpoint_by_id(lldb_target_t target, int32_t id);
uint32_t lldb_target_get_num_watchpoints(lldb_target_t target);
lldb_watchpoint_t lldb_target_get_watchpoint_at_index(lldb_target_t target, uint32_t index);

// SBLaunchInfo
lldb_launch_info_t lldb_launch_info_create(const char** argv);
void lldb_launch_info_destroy(lldb_launch_info_t info);
void lldb_launch_info_set_working_directory(lldb_launch_info_t info, const char* dir);
void lldb_launch_info_set_environment_entries(lldb_launch_info_t info,
                                               const char** envp,
                                               int append);
uint32_t lldb_launch_info_get_launch_flags(lldb_launch_info_t info);
void lldb_launch_info_set_launch_flags(lldb_launch_info_t info, uint32_t flags);

// SBProcess
void lldb_process_destroy(lldb_process_t process);
int lldb_process_is_valid(lldb_process_t process);
int lldb_process_continue(lldb_process_t process);
int lldb_process_stop(lldb_process_t process);
int lldb_process_kill(lldb_process_t process);
int lldb_process_detach(lldb_process_t process);
int lldb_process_destroy_process(lldb_process_t process);
int lldb_process_signal(lldb_process_t process, int signal);
int lldb_process_get_state(lldb_process_t process);
uint32_t lldb_process_get_num_threads(lldb_process_t process);
lldb_thread_t lldb_process_get_thread_at_index(lldb_process_t process, uint32_t index);
lldb_thread_t lldb_process_get_thread_by_id(lldb_process_t process, uint64_t tid);
lldb_thread_t lldb_process_get_thread_by_index_id(lldb_process_t process, uint32_t index_id);
lldb_thread_t lldb_process_get_selected_thread(lldb_process_t process);
int lldb_process_set_selected_thread_by_id(lldb_process_t process, uint64_t tid);
int lldb_process_set_selected_thread_by_index_id(lldb_process_t process, uint32_t index_id);
uint64_t lldb_process_get_process_id(lldb_process_t process);
int lldb_process_get_exit_status(lldb_process_t process);
const char* lldb_process_get_exit_description(lldb_process_t process);
size_t lldb_process_read_memory(lldb_process_t process, uint64_t addr, void* buf, size_t size, lldb_error_t error);
size_t lldb_process_write_memory(lldb_process_t process, uint64_t addr, const void* buf, size_t size, lldb_error_t error);
uint64_t lldb_process_allocate_memory(lldb_process_t process, size_t size, uint32_t permissions, lldb_error_t error);
int lldb_process_deallocate_memory(lldb_process_t process, uint64_t addr);
size_t lldb_process_read_cstring_from_memory(lldb_process_t process, uint64_t addr, void* buf, size_t size, lldb_error_t error);
size_t lldb_process_get_stdout(lldb_process_t process, char* buf, size_t size);
size_t lldb_process_get_stderr(lldb_process_t process, char* buf, size_t size);
size_t lldb_process_put_stdin(lldb_process_t process, const char* buf, size_t size);
int lldb_process_send_async_interrupt(lldb_process_t process);
uint32_t lldb_process_get_num_supported_hardware_watchpoints(lldb_process_t process, lldb_error_t error);
uint32_t lldb_process_get_unique_id(lldb_process_t process);
lldb_memory_region_info_t lldb_process_get_memory_region_info(lldb_process_t process, uint64_t addr, lldb_error_t error);

// SBMemoryRegionInfo
void lldb_memory_region_info_destroy(lldb_memory_region_info_t info);
uint64_t lldb_memory_region_info_get_region_base(lldb_memory_region_info_t info);
uint64_t lldb_memory_region_info_get_region_end(lldb_memory_region_info_t info);
int lldb_memory_region_info_is_readable(lldb_memory_region_info_t info);
int lldb_memory_region_info_is_writable(lldb_memory_region_info_t info);
int lldb_memory_region_info_is_executable(lldb_memory_region_info_t info);
int lldb_memory_region_info_is_mapped(lldb_memory_region_info_t info);
const char* lldb_memory_region_info_get_name(lldb_memory_region_info_t info);

// SBThread
void lldb_thread_destroy(lldb_thread_t thread);
int lldb_thread_is_valid(lldb_thread_t thread);
int lldb_thread_step_over(lldb_thread_t thread);
int lldb_thread_step_into(lldb_thread_t thread);
int lldb_thread_step_out(lldb_thread_t thread);
int lldb_thread_step_instruction(lldb_thread_t thread, int step_over);
int lldb_thread_run_to_address(lldb_thread_t thread, uint64_t addr);
uint32_t lldb_thread_get_num_frames(lldb_thread_t thread);
lldb_frame_t lldb_thread_get_frame_at_index(lldb_thread_t thread, uint32_t index);
lldb_frame_t lldb_thread_get_selected_frame(lldb_thread_t thread);
int lldb_thread_set_selected_frame(lldb_thread_t thread, uint32_t index);
uint64_t lldb_thread_get_thread_id(lldb_thread_t thread);
uint32_t lldb_thread_get_index_id(lldb_thread_t thread);
const char* lldb_thread_get_name(lldb_thread_t thread);
const char* lldb_thread_get_queue_name(lldb_thread_t thread);
int lldb_thread_get_stop_reason(lldb_thread_t thread);
const char* lldb_thread_get_stop_description(lldb_thread_t thread, size_t max_size);
uint64_t lldb_thread_get_stop_reason_data_count(lldb_thread_t thread);
uint64_t lldb_thread_get_stop_reason_data_at_index(lldb_thread_t thread, uint32_t index);
int lldb_thread_is_stopped(lldb_thread_t thread);
int lldb_thread_is_suspended(lldb_thread_t thread);
int lldb_thread_suspend(lldb_thread_t thread);
int lldb_thread_resume(lldb_thread_t thread);
lldb_process_t lldb_thread_get_process(lldb_thread_t thread);

// SBFrame
void lldb_frame_destroy(lldb_frame_t frame);
int lldb_frame_is_valid(lldb_frame_t frame);
const char* lldb_frame_get_function_name(lldb_frame_t frame);
const char* lldb_frame_get_display_function_name(lldb_frame_t frame);
uint32_t lldb_frame_get_line(lldb_frame_t frame);
const char* lldb_frame_get_file_path(lldb_frame_t frame);
uint32_t lldb_frame_get_column(lldb_frame_t frame);
uint64_t lldb_frame_get_pc(lldb_frame_t frame);
int lldb_frame_set_pc(lldb_frame_t frame, uint64_t new_pc);
uint64_t lldb_frame_get_sp(lldb_frame_t frame);
uint64_t lldb_frame_get_fp(lldb_frame_t frame);
lldb_value_t lldb_frame_find_variable(lldb_frame_t frame, const char* name);
lldb_value_t lldb_frame_evaluate_expression(lldb_frame_t frame, const char* expr);
lldb_value_t lldb_frame_get_value_for_variable_path(lldb_frame_t frame, const char* path);
uint32_t lldb_frame_get_frame_id(lldb_frame_t frame);
lldb_thread_t lldb_frame_get_thread(lldb_frame_t frame);
lldb_symbol_context_t lldb_frame_get_symbol_context(lldb_frame_t frame, uint32_t scope);
lldb_value_list_t lldb_frame_get_variables(lldb_frame_t frame, int arguments, int locals, int statics, int in_scope_only);
lldb_value_list_t lldb_frame_get_registers(lldb_frame_t frame);
int lldb_frame_is_inlined(lldb_frame_t frame);
const char* lldb_frame_disassemble(lldb_frame_t frame);
lldb_module_t lldb_frame_get_module(lldb_frame_t frame);

// SBBreakpoint
void lldb_breakpoint_destroy(lldb_breakpoint_t bp);
int lldb_breakpoint_is_valid(lldb_breakpoint_t bp);
int32_t lldb_breakpoint_get_id(lldb_breakpoint_t bp);
int lldb_breakpoint_is_enabled(lldb_breakpoint_t bp);
void lldb_breakpoint_set_enabled(lldb_breakpoint_t bp, int enabled);
int lldb_breakpoint_is_one_shot(lldb_breakpoint_t bp);
void lldb_breakpoint_set_one_shot(lldb_breakpoint_t bp, int one_shot);
uint32_t lldb_breakpoint_get_hit_count(lldb_breakpoint_t bp);
uint32_t lldb_breakpoint_get_ignore_count(lldb_breakpoint_t bp);
void lldb_breakpoint_set_ignore_count(lldb_breakpoint_t bp, uint32_t count);
const char* lldb_breakpoint_get_condition(lldb_breakpoint_t bp);
void lldb_breakpoint_set_condition(lldb_breakpoint_t bp, const char* condition);
uint32_t lldb_breakpoint_get_num_locations(lldb_breakpoint_t bp);
lldb_breakpoint_location_t lldb_breakpoint_get_location_at_index(lldb_breakpoint_t bp, uint32_t index);
lldb_breakpoint_location_t lldb_breakpoint_find_location_by_id(lldb_breakpoint_t bp, int32_t id);
int lldb_breakpoint_is_hardware(lldb_breakpoint_t bp);
int lldb_breakpoint_get_auto_continue(lldb_breakpoint_t bp);
void lldb_breakpoint_set_auto_continue(lldb_breakpoint_t bp, int auto_continue);
uint64_t lldb_breakpoint_get_thread_id(lldb_breakpoint_t bp);
void lldb_breakpoint_set_thread_id(lldb_breakpoint_t bp, uint64_t tid);
const char* lldb_breakpoint_get_thread_name(lldb_breakpoint_t bp);
void lldb_breakpoint_set_thread_name(lldb_breakpoint_t bp, const char* name);
uint32_t lldb_breakpoint_get_thread_index(lldb_breakpoint_t bp);
void lldb_breakpoint_set_thread_index(lldb_breakpoint_t bp, uint32_t index);

// SBBreakpointLocation
void lldb_breakpoint_location_destroy(lldb_breakpoint_location_t loc);
int lldb_breakpoint_location_is_valid(lldb_breakpoint_location_t loc);
int32_t lldb_breakpoint_location_get_id(lldb_breakpoint_location_t loc);
uint64_t lldb_breakpoint_location_get_load_address(lldb_breakpoint_location_t loc);
int lldb_breakpoint_location_is_enabled(lldb_breakpoint_location_t loc);
void lldb_breakpoint_location_set_enabled(lldb_breakpoint_location_t loc, int enabled);
uint32_t lldb_breakpoint_location_get_hit_count(lldb_breakpoint_location_t loc);
uint32_t lldb_breakpoint_location_get_ignore_count(lldb_breakpoint_location_t loc);
void lldb_breakpoint_location_set_ignore_count(lldb_breakpoint_location_t loc, uint32_t count);
const char* lldb_breakpoint_location_get_condition(lldb_breakpoint_location_t loc);
void lldb_breakpoint_location_set_condition(lldb_breakpoint_location_t loc, const char* condition);
lldb_breakpoint_t lldb_breakpoint_location_get_breakpoint(lldb_breakpoint_location_t loc);

// SBValue
void lldb_value_destroy(lldb_value_t value);
int lldb_value_is_valid(lldb_value_t value);
const char* lldb_value_get_name(lldb_value_t value);
const char* lldb_value_get_value(lldb_value_t value);
const char* lldb_value_get_summary(lldb_value_t value);
const char* lldb_value_get_type_name(lldb_value_t value);
lldb_type_t lldb_value_get_type(lldb_value_t value);
uint32_t lldb_value_get_num_children(lldb_value_t value);
lldb_value_t lldb_value_get_child_at_index(lldb_value_t value, uint32_t index);
lldb_value_t lldb_value_get_child_member_with_name(lldb_value_t value, const char* name);
int64_t lldb_value_get_value_as_signed(lldb_value_t value);
uint64_t lldb_value_get_value_as_unsigned(lldb_value_t value);
uint64_t lldb_value_get_byte_size(lldb_value_t value);
int lldb_value_might_have_children(lldb_value_t value);
int lldb_value_get_error(lldb_value_t value, lldb_error_t error);
lldb_value_t lldb_value_dereference(lldb_value_t value);
lldb_value_t lldb_value_address_of(lldb_value_t value);
lldb_value_t lldb_value_cast(lldb_value_t value, lldb_type_t type);
uint64_t lldb_value_get_load_address(lldb_value_t value);
int lldb_value_get_value_type(lldb_value_t value);
int lldb_value_set_value_from_cstring(lldb_value_t value, const char* str, lldb_error_t error);
lldb_value_t lldb_value_create_child_at_offset(lldb_value_t value, const char* name, lldb_type_t type, uint32_t offset);
lldb_value_t lldb_value_create_value_from_address(lldb_value_t value, const char* name, uint64_t addr, lldb_type_t type);
lldb_value_t lldb_value_create_value_from_expression(lldb_value_t value, const char* name, const char* expr);
lldb_watchpoint_t lldb_value_watch(lldb_value_t value, int resolve_location, int read, int write, lldb_error_t error);
const char* lldb_value_get_expression_path(lldb_value_t value);
int lldb_value_is_pointer_type(lldb_value_t value);
lldb_value_t lldb_value_get_non_synthetic_value(lldb_value_t value);

// SBValueList
void lldb_value_list_destroy(lldb_value_list_t list);
int lldb_value_list_is_valid(lldb_value_list_t list);
uint32_t lldb_value_list_get_size(lldb_value_list_t list);
lldb_value_t lldb_value_list_get_value_at_index(lldb_value_list_t list, uint32_t index);
lldb_value_t lldb_value_list_get_first_value_by_name(lldb_value_list_t list, const char* name);

// SBError
lldb_error_t lldb_error_create(void);
void lldb_error_destroy(lldb_error_t error);
int lldb_error_success(lldb_error_t error);
int lldb_error_fail(lldb_error_t error);
const char* lldb_error_get_cstring(lldb_error_t error);
uint32_t lldb_error_get_error(lldb_error_t error);
void lldb_error_clear(lldb_error_t error);
void lldb_error_set_error_string(lldb_error_t error, const char* str);

// SBModule
void lldb_module_destroy(lldb_module_t module);
int lldb_module_is_valid(lldb_module_t module);
const char* lldb_module_get_file_path(lldb_module_t module);
const char* lldb_module_get_platform_file_path(lldb_module_t module);
uint32_t lldb_module_get_num_symbols(lldb_module_t module);

// SBSymbolContext
void lldb_symbol_context_destroy(lldb_symbol_context_t ctx);
int lldb_symbol_context_is_valid(lldb_symbol_context_t ctx);
lldb_module_t lldb_symbol_context_get_module(lldb_symbol_context_t ctx);
const char* lldb_symbol_context_get_function_name(lldb_symbol_context_t ctx);

// SBType
void lldb_type_destroy(lldb_type_t type);
int lldb_type_is_valid(lldb_type_t type);
const char* lldb_type_get_name(lldb_type_t type);
const char* lldb_type_get_display_type_name(lldb_type_t type);
uint64_t lldb_type_get_byte_size(lldb_type_t type);
int lldb_type_is_pointer_type(lldb_type_t type);
int lldb_type_is_reference_type(lldb_type_t type);
int lldb_type_is_array_type(lldb_type_t type);
int lldb_type_is_vector_type(lldb_type_t type);
int lldb_type_is_typedef_type(lldb_type_t type);
int lldb_type_is_function_type(lldb_type_t type);
int lldb_type_is_polymorphic_class(lldb_type_t type);
lldb_type_t lldb_type_get_pointer_type(lldb_type_t type);
lldb_type_t lldb_type_get_pointee_type(lldb_type_t type);
lldb_type_t lldb_type_get_reference_type(lldb_type_t type);
lldb_type_t lldb_type_get_dereferenced_type(lldb_type_t type);
lldb_type_t lldb_type_get_unqualified_type(lldb_type_t type);
lldb_type_t lldb_type_get_canonical_type(lldb_type_t type);
lldb_type_t lldb_type_get_array_element_type(lldb_type_t type);
uint64_t lldb_type_get_array_size(lldb_type_t type);
uint32_t lldb_type_get_num_fields(lldb_type_t type);
uint32_t lldb_type_get_num_direct_base_classes(lldb_type_t type);
uint32_t lldb_type_get_num_virtual_base_classes(lldb_type_t type);
int lldb_type_get_basic_type(lldb_type_t type);

// SBWatchpoint
void lldb_watchpoint_destroy(lldb_watchpoint_t wp);
int lldb_watchpoint_is_valid(lldb_watchpoint_t wp);
int32_t lldb_watchpoint_get_id(lldb_watchpoint_t wp);
int lldb_watchpoint_is_enabled(lldb_watchpoint_t wp);
void lldb_watchpoint_set_enabled(lldb_watchpoint_t wp, int enabled);
uint32_t lldb_watchpoint_get_hit_count(lldb_watchpoint_t wp);
uint32_t lldb_watchpoint_get_ignore_count(lldb_watchpoint_t wp);
void lldb_watchpoint_set_ignore_count(lldb_watchpoint_t wp, uint32_t count);
const char* lldb_watchpoint_get_condition(lldb_watchpoint_t wp);
void lldb_watchpoint_set_condition(lldb_watchpoint_t wp, const char* condition);
uint64_t lldb_watchpoint_get_watch_address(lldb_watchpoint_t wp);
size_t lldb_watchpoint_get_watch_size(lldb_watchpoint_t wp);
int lldb_watchpoint_is_watching_reads(lldb_watchpoint_t wp);
int lldb_watchpoint_is_watching_writes(lldb_watchpoint_t wp);

// SBCommandInterpreter
void lldb_command_interpreter_destroy(lldb_command_interpreter_t interp);
int lldb_command_interpreter_is_valid(lldb_command_interpreter_t interp);
int lldb_command_interpreter_handle_command(lldb_command_interpreter_t interp,
                                             const char* command,
                                             lldb_command_return_object_t result,
                                             int add_to_history);
int lldb_command_interpreter_command_exists(lldb_command_interpreter_t interp, const char* command);
int lldb_command_interpreter_alias_exists(lldb_command_interpreter_t interp, const char* alias);

// SBCommandReturnObject
lldb_command_return_object_t lldb_command_return_object_create(void);
void lldb_command_return_object_destroy(lldb_command_return_object_t obj);
int lldb_command_return_object_is_valid(lldb_command_return_object_t obj);
const char* lldb_command_return_object_get_output(lldb_command_return_object_t obj);
const char* lldb_command_return_object_get_error(lldb_command_return_object_t obj);
int lldb_command_return_object_succeeded(lldb_command_return_object_t obj);
void lldb_command_return_object_clear(lldb_command_return_object_t obj);

// State constants
#define LLDB_STATE_INVALID 0
#define LLDB_STATE_UNLOADED 1
#define LLDB_STATE_CONNECTED 2
#define LLDB_STATE_ATTACHING 3
#define LLDB_STATE_LAUNCHING 4
#define LLDB_STATE_STOPPED 5
#define LLDB_STATE_RUNNING 6
#define LLDB_STATE_STEPPING 7
#define LLDB_STATE_CRASHED 8
#define LLDB_STATE_DETACHED 9
#define LLDB_STATE_EXITED 10
#define LLDB_STATE_SUSPENDED 11

// Stop reason constants
#define LLDB_STOP_REASON_INVALID 0
#define LLDB_STOP_REASON_NONE 1
#define LLDB_STOP_REASON_TRACE 2
#define LLDB_STOP_REASON_BREAKPOINT 3
#define LLDB_STOP_REASON_WATCHPOINT 4
#define LLDB_STOP_REASON_SIGNAL 5
#define LLDB_STOP_REASON_EXCEPTION 6
#define LLDB_STOP_REASON_EXEC 7
#define LLDB_STOP_REASON_PLAN_COMPLETE 8
#define LLDB_STOP_REASON_THREAD_EXITING 9
#define LLDB_STOP_REASON_INSTRUMENTATION 10

// Value type constants
#define LLDB_VALUE_TYPE_INVALID 0
#define LLDB_VALUE_TYPE_VARIABLE_GLOBAL 1
#define LLDB_VALUE_TYPE_VARIABLE_STATIC 2
#define LLDB_VALUE_TYPE_VARIABLE_ARGUMENT 3
#define LLDB_VALUE_TYPE_VARIABLE_LOCAL 4
#define LLDB_VALUE_TYPE_REGISTER 5
#define LLDB_VALUE_TYPE_REGISTER_SET 6
#define LLDB_VALUE_TYPE_CONSTANT_RESULT 7

// Basic type constants
#define LLDB_BASIC_TYPE_INVALID 0
#define LLDB_BASIC_TYPE_VOID 1
#define LLDB_BASIC_TYPE_CHAR 2
#define LLDB_BASIC_TYPE_SIGNED_CHAR 3
#define LLDB_BASIC_TYPE_UNSIGNED_CHAR 4
#define LLDB_BASIC_TYPE_SHORT 6
#define LLDB_BASIC_TYPE_UNSIGNED_SHORT 7
#define LLDB_BASIC_TYPE_INT 8
#define LLDB_BASIC_TYPE_UNSIGNED_INT 9
#define LLDB_BASIC_TYPE_LONG 10
#define LLDB_BASIC_TYPE_UNSIGNED_LONG 11
#define LLDB_BASIC_TYPE_LONG_LONG 12
#define LLDB_BASIC_TYPE_UNSIGNED_LONG_LONG 13
#define LLDB_BASIC_TYPE_FLOAT 15
#define LLDB_BASIC_TYPE_DOUBLE 16
#define LLDB_BASIC_TYPE_LONG_DOUBLE 17
#define LLDB_BASIC_TYPE_BOOL 20

#ifdef __cplusplus
}
#endif

#endif // LLDB_WRAPPER_H
