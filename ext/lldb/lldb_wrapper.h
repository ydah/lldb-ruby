#ifndef LLDB_WRAPPER_H
#define LLDB_WRAPPER_H

#ifdef __cplusplus
#define LLDB_WRAPPER_NOEXCEPT noexcept
#else
#define LLDB_WRAPPER_NOEXCEPT
#endif

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
typedef void* lldb_type_member_t;
typedef void* lldb_symbol_t;
typedef void* lldb_function_t;
typedef void* lldb_compile_unit_t;
typedef void* lldb_block_t;
typedef void* lldb_instruction_list_t;
typedef void* lldb_instruction_t;
typedef void* lldb_watchpoint_t;
typedef void* lldb_command_interpreter_t;
typedef void* lldb_command_return_object_t;
typedef void* lldb_memory_region_info_t;
typedef void* lldb_file_spec_t;
typedef void* lldb_broadcaster_t;
typedef void* lldb_listener_t;
typedef void* lldb_event_t;
typedef void* lldb_attach_info_t;
typedef void* lldb_expression_options_t;
typedef void* lldb_address_t;
typedef void* lldb_line_entry_t;
typedef void* lldb_file_spec_list_t;

typedef enum {
    LLDB_RUBY_STATUS_OK = 0,
    LLDB_RUBY_STATUS_INVALID_ARGUMENT = 1,
    LLDB_RUBY_STATUS_INVALID_HANDLE = 2,
    LLDB_RUBY_STATUS_UNSUPPORTED = 3,
    LLDB_RUBY_STATUS_LLDB_ERROR = 4,
    LLDB_RUBY_STATUS_INTERNAL_ERROR = 5
} lldb_ruby_status_t;

typedef enum {
    LLDB_RUBY_CAPABILITY_WATCHPOINT_ACCESS_KIND = 1
} lldb_ruby_capability_t;

// Wrapper metadata and capability discovery
uint32_t lldb_wrapper_abi_version(void) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_wrapper_build_lldb_version(void) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_wrapper_runtime_lldb_version(void) LLDB_WRAPPER_NOEXCEPT;
int lldb_wrapper_has_capability(uint32_t capability) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_wrapper_last_error_message(void) LLDB_WRAPPER_NOEXCEPT;
int lldb_wrapper_last_error_code(void) LLDB_WRAPPER_NOEXCEPT;
void lldb_wrapper_clear_last_error(void) LLDB_WRAPPER_NOEXCEPT;

// Initialization
lldb_ruby_status_t lldb_initialize(lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
void lldb_terminate(void) LLDB_WRAPPER_NOEXCEPT;

// SBDebugger
lldb_debugger_t lldb_debugger_create(void) LLDB_WRAPPER_NOEXCEPT;
lldb_debugger_t lldb_debugger_create_with_source_init_files(int source_init_files) LLDB_WRAPPER_NOEXCEPT;
void lldb_debugger_destroy(lldb_debugger_t dbg) LLDB_WRAPPER_NOEXCEPT;
int lldb_debugger_is_valid(lldb_debugger_t dbg) LLDB_WRAPPER_NOEXCEPT;
lldb_target_t lldb_debugger_create_target(lldb_debugger_t dbg,
                                           const char* filename,
                                           const char* arch,
                                           const char* platform,
                                           int add_dependent_modules,
                                           lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_target_t lldb_debugger_create_target_simple(lldb_debugger_t dbg,
                                                  const char* filename) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_debugger_get_num_targets(lldb_debugger_t dbg) LLDB_WRAPPER_NOEXCEPT;
lldb_target_t lldb_debugger_get_target_at_index(lldb_debugger_t dbg, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
lldb_target_t lldb_debugger_get_selected_target(lldb_debugger_t dbg) LLDB_WRAPPER_NOEXCEPT;
void lldb_debugger_set_selected_target(lldb_debugger_t dbg, lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
int lldb_debugger_delete_target(lldb_debugger_t dbg, lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
lldb_target_t lldb_debugger_find_target_with_process_id(lldb_debugger_t dbg, uint64_t pid) LLDB_WRAPPER_NOEXCEPT;
void lldb_debugger_set_async(lldb_debugger_t dbg, int async) LLDB_WRAPPER_NOEXCEPT;
int lldb_debugger_get_async(lldb_debugger_t dbg) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_debugger_get_version_string(void) LLDB_WRAPPER_NOEXCEPT;
lldb_command_interpreter_t lldb_debugger_get_command_interpreter(lldb_debugger_t dbg) LLDB_WRAPPER_NOEXCEPT;
lldb_broadcaster_t lldb_debugger_get_broadcaster(lldb_debugger_t dbg) LLDB_WRAPPER_NOEXCEPT;
lldb_listener_t lldb_debugger_get_listener(lldb_debugger_t dbg) LLDB_WRAPPER_NOEXCEPT;
void lldb_debugger_handle_command(lldb_debugger_t dbg, const char* command) LLDB_WRAPPER_NOEXCEPT;

// SBBroadcaster
lldb_broadcaster_t lldb_broadcaster_create(const char* name) LLDB_WRAPPER_NOEXCEPT;
void lldb_broadcaster_destroy(lldb_broadcaster_t broadcaster) LLDB_WRAPPER_NOEXCEPT;
int lldb_broadcaster_is_valid(lldb_broadcaster_t broadcaster) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_broadcaster_get_name(lldb_broadcaster_t broadcaster) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_broadcaster_add_listener(lldb_broadcaster_t broadcaster,
                                        lldb_listener_t listener,
                                        uint32_t event_mask) LLDB_WRAPPER_NOEXCEPT;
int lldb_broadcaster_remove_listener(lldb_broadcaster_t broadcaster,
                                     lldb_listener_t listener,
                                     uint32_t event_mask) LLDB_WRAPPER_NOEXCEPT;
int lldb_broadcaster_event_type_has_listeners(lldb_broadcaster_t broadcaster,
                                              uint32_t event_type) LLDB_WRAPPER_NOEXCEPT;
void lldb_broadcaster_broadcast_event_by_type(lldb_broadcaster_t broadcaster,
                                               uint32_t event_type,
                                               int unique) LLDB_WRAPPER_NOEXCEPT;

// SBListener
lldb_listener_t lldb_listener_create(const char* name) LLDB_WRAPPER_NOEXCEPT;
void lldb_listener_destroy(lldb_listener_t listener) LLDB_WRAPPER_NOEXCEPT;
int lldb_listener_is_valid(lldb_listener_t listener) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_listener_start_listening_for_events(lldb_listener_t listener,
                                                  lldb_broadcaster_t broadcaster,
                                                  uint32_t event_mask) LLDB_WRAPPER_NOEXCEPT;
int lldb_listener_stop_listening_for_events(lldb_listener_t listener,
                                            lldb_broadcaster_t broadcaster,
                                            uint32_t event_mask) LLDB_WRAPPER_NOEXCEPT;
lldb_event_t lldb_listener_wait_for_event(lldb_listener_t listener,
                                          uint32_t timeout_seconds) LLDB_WRAPPER_NOEXCEPT;
lldb_event_t lldb_listener_peek_event(lldb_listener_t listener) LLDB_WRAPPER_NOEXCEPT;
lldb_event_t lldb_listener_next_event(lldb_listener_t listener) LLDB_WRAPPER_NOEXCEPT;

// SBEvent
void lldb_event_destroy(lldb_event_t event) LLDB_WRAPPER_NOEXCEPT;
int lldb_event_is_valid(lldb_event_t event) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_event_get_type(lldb_event_t event) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_event_get_data_flavor(lldb_event_t event) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_event_get_broadcaster_class(lldb_event_t event) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_event_get_description(lldb_event_t event, char* buffer, size_t length) LLDB_WRAPPER_NOEXCEPT;
lldb_broadcaster_t lldb_event_get_broadcaster(lldb_event_t event) LLDB_WRAPPER_NOEXCEPT;
int lldb_event_is_process_event(lldb_event_t event) LLDB_WRAPPER_NOEXCEPT;
int lldb_event_get_process_state(lldb_event_t event) LLDB_WRAPPER_NOEXCEPT;
int lldb_event_get_restarted(lldb_event_t event) LLDB_WRAPPER_NOEXCEPT;
int lldb_event_get_interrupted(lldb_event_t event) LLDB_WRAPPER_NOEXCEPT;
lldb_process_t lldb_event_get_process(lldb_event_t event) LLDB_WRAPPER_NOEXCEPT;

// SBTarget
void lldb_target_destroy(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
int lldb_target_is_valid(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
lldb_process_t lldb_target_launch_simple(lldb_target_t target,
                                          const char** argv,
                                          const char** envp,
                                          const char* working_dir) LLDB_WRAPPER_NOEXCEPT;
lldb_process_t lldb_target_launch(lldb_target_t target,
                                   lldb_launch_info_t launch_info,
                                   lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_process_t lldb_target_attach_to_process_with_id(lldb_target_t target,
                                                      uint64_t pid,
                                                      lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_process_t lldb_target_attach_to_process_with_name(lldb_target_t target,
                                                        const char* name,
                                                        int wait_for,
                                                        lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_process_t lldb_target_attach_with_info(lldb_target_t target,
                                             lldb_attach_info_t attach_info,
                                             lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_breakpoint_t lldb_target_breakpoint_create_by_name(lldb_target_t target,
                                                         const char* symbol_name,
                                                         const char* module_name) LLDB_WRAPPER_NOEXCEPT;
lldb_breakpoint_t lldb_target_breakpoint_create_by_location(lldb_target_t target,
                                                             const char* file,
                                                             uint32_t line) LLDB_WRAPPER_NOEXCEPT;
lldb_breakpoint_t lldb_target_breakpoint_create_by_address(lldb_target_t target,
                                                            uint64_t address) LLDB_WRAPPER_NOEXCEPT;
lldb_breakpoint_t lldb_target_breakpoint_create_by_regex(lldb_target_t target,
                                                          const char* symbol_regex,
                                                          const char* module_name) LLDB_WRAPPER_NOEXCEPT;
lldb_breakpoint_t lldb_target_breakpoint_create_by_source_regex(lldb_target_t target,
                                                                 const char* source_regex,
                                                                 const char* source_file) LLDB_WRAPPER_NOEXCEPT;
int lldb_target_delete_breakpoint(lldb_target_t target, int32_t breakpoint_id) LLDB_WRAPPER_NOEXCEPT;
int lldb_target_delete_all_breakpoints(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
int lldb_target_enable_all_breakpoints(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
int lldb_target_disable_all_breakpoints(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
lldb_breakpoint_t lldb_target_find_breakpoint_by_id(lldb_target_t target, int32_t id) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_target_get_num_breakpoints(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
lldb_breakpoint_t lldb_target_get_breakpoint_at_index(lldb_target_t target, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
lldb_process_t lldb_target_get_process(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_target_get_executable_path(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
lldb_file_spec_t lldb_target_get_executable_file(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_target_get_num_modules(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
lldb_module_t lldb_target_get_module_at_index(lldb_target_t target, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_target_evaluate_expression(lldb_target_t target, const char* expr) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_target_evaluate_expression_with_options(lldb_target_t target,
                                                          const char* expr,
                                                          lldb_expression_options_t options) LLDB_WRAPPER_NOEXCEPT;
size_t lldb_target_read_memory(lldb_target_t target, uint64_t addr, void* buf, size_t size, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_target_get_address_byte_size(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_target_get_triple(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
lldb_watchpoint_t lldb_target_watch_address(lldb_target_t target, uint64_t addr, size_t size, int read, int write, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
int lldb_target_delete_watchpoint(lldb_target_t target, int32_t watchpoint_id) LLDB_WRAPPER_NOEXCEPT;
int lldb_target_delete_all_watchpoints(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
lldb_watchpoint_t lldb_target_find_watchpoint_by_id(lldb_target_t target, int32_t id) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_target_get_num_watchpoints(lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
lldb_watchpoint_t lldb_target_get_watchpoint_at_index(lldb_target_t target, uint32_t index) LLDB_WRAPPER_NOEXCEPT;

// SBLaunchInfo
lldb_launch_info_t lldb_launch_info_create(const char** argv) LLDB_WRAPPER_NOEXCEPT;
void lldb_launch_info_destroy(lldb_launch_info_t info) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_launch_info_get_num_arguments(lldb_launch_info_t info) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_launch_info_get_argument_at_index(lldb_launch_info_t info, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
void lldb_launch_info_set_working_directory(lldb_launch_info_t info, const char* dir) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_launch_info_get_working_directory(lldb_launch_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_launch_info_set_environment_entries(lldb_launch_info_t info,
                                               const char** envp,
                                               int append) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_launch_info_get_num_environment_entries(lldb_launch_info_t info) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_launch_info_get_environment_entry_at_index(lldb_launch_info_t info,
                                                             uint32_t index) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_launch_info_get_launch_flags(lldb_launch_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_launch_info_set_launch_flags(lldb_launch_info_t info, uint32_t flags) LLDB_WRAPPER_NOEXCEPT;
void lldb_launch_info_set_arguments(lldb_launch_info_t info, const char** argv, int append) LLDB_WRAPPER_NOEXCEPT;
lldb_file_spec_t lldb_launch_info_get_executable_file(lldb_launch_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_launch_info_set_executable_file(lldb_launch_info_t info,
                                           lldb_file_spec_t file_spec,
                                           int add_as_first_arg) LLDB_WRAPPER_NOEXCEPT;
lldb_listener_t lldb_launch_info_get_listener(lldb_launch_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_launch_info_set_listener(lldb_launch_info_t info, lldb_listener_t listener) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_launch_info_get_process_plugin_name(lldb_launch_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_launch_info_set_process_plugin_name(lldb_launch_info_t info, const char* name) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_launch_info_get_shell(lldb_launch_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_launch_info_set_shell(lldb_launch_info_t info, const char* path) LLDB_WRAPPER_NOEXCEPT;
int lldb_launch_info_add_close_file_action(lldb_launch_info_t info, int fd) LLDB_WRAPPER_NOEXCEPT;
int lldb_launch_info_add_duplicate_file_action(lldb_launch_info_t info, int fd, int dup_fd) LLDB_WRAPPER_NOEXCEPT;
int lldb_launch_info_add_open_file_action(lldb_launch_info_t info,
                                          int fd,
                                          const char* path,
                                          int read,
                                          int write) LLDB_WRAPPER_NOEXCEPT;
int lldb_launch_info_add_suppress_file_action(lldb_launch_info_t info,
                                              int fd,
                                              int read,
                                              int write) LLDB_WRAPPER_NOEXCEPT;

// SBAttachInfo
lldb_attach_info_t lldb_attach_info_create(uint64_t pid) LLDB_WRAPPER_NOEXCEPT;
void lldb_attach_info_destroy(lldb_attach_info_t info) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_attach_info_get_process_id(lldb_attach_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_attach_info_set_process_id(lldb_attach_info_t info, uint64_t pid) LLDB_WRAPPER_NOEXCEPT;
void lldb_attach_info_set_executable(lldb_attach_info_t info, const char* path) LLDB_WRAPPER_NOEXCEPT;
void lldb_attach_info_set_executable_file(lldb_attach_info_t info, lldb_file_spec_t file_spec) LLDB_WRAPPER_NOEXCEPT;
int lldb_attach_info_get_wait_for_launch(lldb_attach_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_attach_info_set_wait_for_launch(lldb_attach_info_t info, int wait_for) LLDB_WRAPPER_NOEXCEPT;
int lldb_attach_info_get_ignore_existing(lldb_attach_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_attach_info_set_ignore_existing(lldb_attach_info_t info, int ignore_existing) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_attach_info_get_resume_count(lldb_attach_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_attach_info_set_resume_count(lldb_attach_info_t info, uint32_t count) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_attach_info_get_process_plugin_name(lldb_attach_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_attach_info_set_process_plugin_name(lldb_attach_info_t info, const char* name) LLDB_WRAPPER_NOEXCEPT;
lldb_listener_t lldb_attach_info_get_listener(lldb_attach_info_t info) LLDB_WRAPPER_NOEXCEPT;
void lldb_attach_info_set_listener(lldb_attach_info_t info, lldb_listener_t listener) LLDB_WRAPPER_NOEXCEPT;

// SBExpressionOptions
lldb_expression_options_t lldb_expression_options_create(void) LLDB_WRAPPER_NOEXCEPT;
void lldb_expression_options_destroy(lldb_expression_options_t options) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_expression_options_get_timeout(lldb_expression_options_t options) LLDB_WRAPPER_NOEXCEPT;
void lldb_expression_options_set_timeout(lldb_expression_options_t options, uint32_t timeout) LLDB_WRAPPER_NOEXCEPT;
int lldb_expression_options_get_unwind_on_error(lldb_expression_options_t options) LLDB_WRAPPER_NOEXCEPT;
void lldb_expression_options_set_unwind_on_error(lldb_expression_options_t options, int value) LLDB_WRAPPER_NOEXCEPT;
int lldb_expression_options_get_ignore_breakpoints(lldb_expression_options_t options) LLDB_WRAPPER_NOEXCEPT;
void lldb_expression_options_set_ignore_breakpoints(lldb_expression_options_t options, int value) LLDB_WRAPPER_NOEXCEPT;
int lldb_expression_options_get_fetch_dynamic_value(lldb_expression_options_t options) LLDB_WRAPPER_NOEXCEPT;
void lldb_expression_options_set_fetch_dynamic_value(lldb_expression_options_t options, int value) LLDB_WRAPPER_NOEXCEPT;
int lldb_expression_options_get_try_all_threads(lldb_expression_options_t options) LLDB_WRAPPER_NOEXCEPT;
void lldb_expression_options_set_try_all_threads(lldb_expression_options_t options, int value) LLDB_WRAPPER_NOEXCEPT;
int lldb_expression_options_get_stop_others(lldb_expression_options_t options) LLDB_WRAPPER_NOEXCEPT;
void lldb_expression_options_set_stop_others(lldb_expression_options_t options, int value) LLDB_WRAPPER_NOEXCEPT;
void lldb_expression_options_set_language(lldb_expression_options_t options, int value) LLDB_WRAPPER_NOEXCEPT;
int lldb_expression_options_get_suppress_persistent_result(lldb_expression_options_t options) LLDB_WRAPPER_NOEXCEPT;
void lldb_expression_options_set_suppress_persistent_result(lldb_expression_options_t options, int value) LLDB_WRAPPER_NOEXCEPT;

// SBFileSpec
lldb_file_spec_t lldb_file_spec_create(const char* path, int resolve) LLDB_WRAPPER_NOEXCEPT;
void lldb_file_spec_destroy(lldb_file_spec_t file_spec) LLDB_WRAPPER_NOEXCEPT;
int lldb_file_spec_is_valid(lldb_file_spec_t file_spec) LLDB_WRAPPER_NOEXCEPT;
int lldb_file_spec_exists(lldb_file_spec_t file_spec) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_file_spec_get_filename(lldb_file_spec_t file_spec) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_file_spec_get_directory(lldb_file_spec_t file_spec) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_file_spec_get_path(lldb_file_spec_t file_spec, char* buffer, size_t length) LLDB_WRAPPER_NOEXCEPT;
void lldb_file_spec_set_filename(lldb_file_spec_t file_spec, const char* filename) LLDB_WRAPPER_NOEXCEPT;
void lldb_file_spec_set_directory(lldb_file_spec_t file_spec, const char* directory) LLDB_WRAPPER_NOEXCEPT;

// SBFileSpecList
lldb_file_spec_list_t lldb_file_spec_list_create(void) LLDB_WRAPPER_NOEXCEPT;
void lldb_file_spec_list_destroy(lldb_file_spec_list_t list) LLDB_WRAPPER_NOEXCEPT;
int lldb_file_spec_list_is_valid(lldb_file_spec_list_t list) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_file_spec_list_get_size(lldb_file_spec_list_t list) LLDB_WRAPPER_NOEXCEPT;
void lldb_file_spec_list_append(lldb_file_spec_list_t list, lldb_file_spec_t file_spec) LLDB_WRAPPER_NOEXCEPT;
int lldb_file_spec_list_append_if_unique(lldb_file_spec_list_t list, lldb_file_spec_t file_spec) LLDB_WRAPPER_NOEXCEPT;
void lldb_file_spec_list_clear(lldb_file_spec_list_t list) LLDB_WRAPPER_NOEXCEPT;
lldb_file_spec_t lldb_file_spec_list_get_file_spec_at_index(lldb_file_spec_list_t list,
                                                             uint32_t index) LLDB_WRAPPER_NOEXCEPT;

// SBAddress
lldb_address_t lldb_address_create(void) LLDB_WRAPPER_NOEXCEPT;
lldb_address_t lldb_address_create_from_load_address(uint64_t address, lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
void lldb_address_destroy(lldb_address_t address) LLDB_WRAPPER_NOEXCEPT;
int lldb_address_is_valid(lldb_address_t address) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_address_get_file_address(lldb_address_t address) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_address_get_load_address(lldb_address_t address, lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_address_get_offset(lldb_address_t address) LLDB_WRAPPER_NOEXCEPT;
lldb_line_entry_t lldb_address_get_line_entry(lldb_address_t address) LLDB_WRAPPER_NOEXCEPT;

// SBLineEntry
void lldb_line_entry_destroy(lldb_line_entry_t entry) LLDB_WRAPPER_NOEXCEPT;
int lldb_line_entry_is_valid(lldb_line_entry_t entry) LLDB_WRAPPER_NOEXCEPT;
lldb_address_t lldb_line_entry_get_start_address(lldb_line_entry_t entry) LLDB_WRAPPER_NOEXCEPT;
lldb_address_t lldb_line_entry_get_end_address(lldb_line_entry_t entry) LLDB_WRAPPER_NOEXCEPT;
lldb_file_spec_t lldb_line_entry_get_file_spec(lldb_line_entry_t entry) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_line_entry_get_line(lldb_line_entry_t entry) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_line_entry_get_column(lldb_line_entry_t entry) LLDB_WRAPPER_NOEXCEPT;

// SBProcess
void lldb_process_destroy(lldb_process_t process) LLDB_WRAPPER_NOEXCEPT;
int lldb_process_is_valid(lldb_process_t process) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_process_continue(lldb_process_t process, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_process_stop(lldb_process_t process, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_process_kill(lldb_process_t process, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_process_detach(lldb_process_t process, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_process_destroy_process(lldb_process_t process, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_process_signal(lldb_process_t process, int signal, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
int lldb_process_get_state(lldb_process_t process) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_process_get_num_threads(lldb_process_t process) LLDB_WRAPPER_NOEXCEPT;
lldb_thread_t lldb_process_get_thread_at_index(lldb_process_t process, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
lldb_thread_t lldb_process_get_thread_by_id(lldb_process_t process, uint64_t tid) LLDB_WRAPPER_NOEXCEPT;
lldb_thread_t lldb_process_get_thread_by_index_id(lldb_process_t process, uint32_t index_id) LLDB_WRAPPER_NOEXCEPT;
lldb_thread_t lldb_process_get_selected_thread(lldb_process_t process) LLDB_WRAPPER_NOEXCEPT;
int lldb_process_set_selected_thread_by_id(lldb_process_t process, uint64_t tid) LLDB_WRAPPER_NOEXCEPT;
int lldb_process_set_selected_thread_by_index_id(lldb_process_t process, uint32_t index_id) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_process_get_process_id(lldb_process_t process) LLDB_WRAPPER_NOEXCEPT;
int lldb_process_get_exit_status(lldb_process_t process) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_process_get_exit_description(lldb_process_t process) LLDB_WRAPPER_NOEXCEPT;
size_t lldb_process_read_memory(lldb_process_t process, uint64_t addr, void* buf, size_t size, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
size_t lldb_process_write_memory(lldb_process_t process, uint64_t addr, const void* buf, size_t size, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_process_allocate_memory(lldb_process_t process, size_t size, uint32_t permissions, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_process_deallocate_memory(lldb_process_t process, uint64_t addr, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
size_t lldb_process_read_cstring_from_memory(lldb_process_t process, uint64_t addr, void* buf, size_t size, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
size_t lldb_process_get_stdout(lldb_process_t process, char* buf, size_t size) LLDB_WRAPPER_NOEXCEPT;
size_t lldb_process_get_stderr(lldb_process_t process, char* buf, size_t size) LLDB_WRAPPER_NOEXCEPT;
size_t lldb_process_put_stdin(lldb_process_t process, const char* buf, size_t size) LLDB_WRAPPER_NOEXCEPT;
void lldb_process_send_async_interrupt(lldb_process_t process) LLDB_WRAPPER_NOEXCEPT;
lldb_broadcaster_t lldb_process_get_broadcaster(lldb_process_t process) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_process_get_num_supported_hardware_watchpoints(lldb_process_t process,
                                                                        uint32_t* result,
                                                                        lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_process_get_unique_id(lldb_process_t process) LLDB_WRAPPER_NOEXCEPT;
lldb_memory_region_info_t lldb_process_get_memory_region_info(lldb_process_t process, uint64_t addr, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;

// SBMemoryRegionInfo
void lldb_memory_region_info_destroy(lldb_memory_region_info_t info) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_memory_region_info_get_region_base(lldb_memory_region_info_t info) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_memory_region_info_get_region_end(lldb_memory_region_info_t info) LLDB_WRAPPER_NOEXCEPT;
int lldb_memory_region_info_is_readable(lldb_memory_region_info_t info) LLDB_WRAPPER_NOEXCEPT;
int lldb_memory_region_info_is_writable(lldb_memory_region_info_t info) LLDB_WRAPPER_NOEXCEPT;
int lldb_memory_region_info_is_executable(lldb_memory_region_info_t info) LLDB_WRAPPER_NOEXCEPT;
int lldb_memory_region_info_is_mapped(lldb_memory_region_info_t info) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_memory_region_info_get_name(lldb_memory_region_info_t info) LLDB_WRAPPER_NOEXCEPT;

// SBThread
void lldb_thread_destroy(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
int lldb_thread_is_valid(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_thread_step_over(lldb_thread_t thread, int run_mode, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_thread_step_into(lldb_thread_t thread,
                                         const char* target_name,
                                         uint32_t end_line,
                                         int run_mode,
                                         lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_thread_step_out(lldb_thread_t thread, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_thread_step_instruction(lldb_thread_t thread,
                                                int step_over,
                                                lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_thread_run_to_address(lldb_thread_t thread,
                                              uint64_t addr,
                                              lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_thread_get_num_frames(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
lldb_frame_t lldb_thread_get_frame_at_index(lldb_thread_t thread, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
lldb_frame_t lldb_thread_get_selected_frame(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
int lldb_thread_set_selected_frame(lldb_thread_t thread, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_thread_get_thread_id(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_thread_get_index_id(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_thread_get_name(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_thread_get_queue_name(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
int lldb_thread_get_stop_reason(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
size_t lldb_thread_get_stop_description(lldb_thread_t thread, char* buffer, size_t length) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_thread_get_stop_reason_data_count(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_thread_get_stop_reason_data_at_index(lldb_thread_t thread, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
int lldb_thread_is_stopped(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
int lldb_thread_is_suspended(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
int lldb_thread_suspend(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
int lldb_thread_resume(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;
lldb_process_t lldb_thread_get_process(lldb_thread_t thread) LLDB_WRAPPER_NOEXCEPT;

// SBFrame
void lldb_frame_destroy(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
int lldb_frame_is_valid(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_frame_get_function_name(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_frame_get_display_function_name(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_frame_get_line(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_frame_get_file_path(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
lldb_file_spec_t lldb_frame_get_file_spec(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
lldb_line_entry_t lldb_frame_get_line_entry(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_frame_get_column(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_frame_get_pc(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
int lldb_frame_set_pc(lldb_frame_t frame, uint64_t new_pc) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_frame_get_sp(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_frame_get_fp(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_frame_find_variable(lldb_frame_t frame, const char* name) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_frame_evaluate_expression(lldb_frame_t frame, const char* expr) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_frame_evaluate_expression_with_options(lldb_frame_t frame,
                                                         const char* expr,
                                                         lldb_expression_options_t options) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_frame_get_value_for_variable_path(lldb_frame_t frame, const char* path) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_frame_get_frame_id(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
lldb_thread_t lldb_frame_get_thread(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
lldb_function_t lldb_frame_get_function(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
lldb_symbol_t lldb_frame_get_symbol(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
lldb_compile_unit_t lldb_frame_get_compile_unit(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
lldb_block_t lldb_frame_get_block(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
lldb_instruction_list_t lldb_frame_get_instruction_list(lldb_frame_t frame,
                                                         lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
lldb_symbol_context_t lldb_frame_get_symbol_context(lldb_frame_t frame, uint32_t scope) LLDB_WRAPPER_NOEXCEPT;
lldb_value_list_t lldb_frame_get_variables(lldb_frame_t frame, int arguments, int locals, int statics, int in_scope_only) LLDB_WRAPPER_NOEXCEPT;
lldb_value_list_t lldb_frame_get_registers(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
int lldb_frame_is_inlined(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_frame_disassemble(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;
lldb_module_t lldb_frame_get_module(lldb_frame_t frame) LLDB_WRAPPER_NOEXCEPT;

// SBBreakpoint
void lldb_breakpoint_destroy(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
int lldb_breakpoint_is_valid(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
int32_t lldb_breakpoint_get_id(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
int lldb_breakpoint_is_enabled(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
void lldb_breakpoint_set_enabled(lldb_breakpoint_t bp, int enabled) LLDB_WRAPPER_NOEXCEPT;
int lldb_breakpoint_is_one_shot(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
void lldb_breakpoint_set_one_shot(lldb_breakpoint_t bp, int one_shot) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_breakpoint_get_hit_count(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_breakpoint_get_ignore_count(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
void lldb_breakpoint_set_ignore_count(lldb_breakpoint_t bp, uint32_t count) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_breakpoint_get_condition(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
void lldb_breakpoint_set_condition(lldb_breakpoint_t bp, const char* condition) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_breakpoint_get_num_locations(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
lldb_breakpoint_location_t lldb_breakpoint_get_location_at_index(lldb_breakpoint_t bp, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
lldb_breakpoint_location_t lldb_breakpoint_find_location_by_id(lldb_breakpoint_t bp, int32_t id) LLDB_WRAPPER_NOEXCEPT;
int lldb_breakpoint_is_hardware(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
int lldb_breakpoint_get_auto_continue(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
void lldb_breakpoint_set_auto_continue(lldb_breakpoint_t bp, int auto_continue) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_breakpoint_get_thread_id(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
void lldb_breakpoint_set_thread_id(lldb_breakpoint_t bp, uint64_t tid) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_breakpoint_get_thread_name(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
void lldb_breakpoint_set_thread_name(lldb_breakpoint_t bp, const char* name) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_breakpoint_get_thread_index(lldb_breakpoint_t bp) LLDB_WRAPPER_NOEXCEPT;
void lldb_breakpoint_set_thread_index(lldb_breakpoint_t bp, uint32_t index) LLDB_WRAPPER_NOEXCEPT;

// SBBreakpointLocation
void lldb_breakpoint_location_destroy(lldb_breakpoint_location_t loc) LLDB_WRAPPER_NOEXCEPT;
int lldb_breakpoint_location_is_valid(lldb_breakpoint_location_t loc) LLDB_WRAPPER_NOEXCEPT;
int32_t lldb_breakpoint_location_get_id(lldb_breakpoint_location_t loc) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_breakpoint_location_get_load_address(lldb_breakpoint_location_t loc) LLDB_WRAPPER_NOEXCEPT;
lldb_address_t lldb_breakpoint_location_get_address(lldb_breakpoint_location_t loc) LLDB_WRAPPER_NOEXCEPT;
int lldb_breakpoint_location_is_enabled(lldb_breakpoint_location_t loc) LLDB_WRAPPER_NOEXCEPT;
void lldb_breakpoint_location_set_enabled(lldb_breakpoint_location_t loc, int enabled) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_breakpoint_location_get_hit_count(lldb_breakpoint_location_t loc) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_breakpoint_location_get_ignore_count(lldb_breakpoint_location_t loc) LLDB_WRAPPER_NOEXCEPT;
void lldb_breakpoint_location_set_ignore_count(lldb_breakpoint_location_t loc, uint32_t count) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_breakpoint_location_get_condition(lldb_breakpoint_location_t loc) LLDB_WRAPPER_NOEXCEPT;
void lldb_breakpoint_location_set_condition(lldb_breakpoint_location_t loc, const char* condition) LLDB_WRAPPER_NOEXCEPT;
lldb_breakpoint_t lldb_breakpoint_location_get_breakpoint(lldb_breakpoint_location_t loc) LLDB_WRAPPER_NOEXCEPT;

// SBValue
void lldb_value_destroy(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
int lldb_value_is_valid(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_value_get_name(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_value_get_value(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_value_get_summary(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_value_get_type_name(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
lldb_type_t lldb_value_get_type(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_value_get_num_children(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_value_get_child_at_index(lldb_value_t value, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_value_get_child_member_with_name(lldb_value_t value, const char* name) LLDB_WRAPPER_NOEXCEPT;
int64_t lldb_value_get_value_as_signed(lldb_value_t value, lldb_error_t error, int64_t fail_value) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_value_get_value_as_unsigned(lldb_value_t value, lldb_error_t error, uint64_t fail_value) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_value_get_byte_size(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
int lldb_value_might_have_children(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
int lldb_value_get_error(lldb_value_t value, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_value_dereference(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_value_address_of(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_value_cast(lldb_value_t value, lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_value_get_load_address(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
int lldb_value_get_value_type(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
int lldb_value_set_value_from_cstring(lldb_value_t value, const char* str, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_value_create_child_at_offset(lldb_value_t value, const char* name, lldb_type_t type, uint32_t offset) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_value_create_value_from_address(lldb_value_t value, const char* name, uint64_t addr, lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_value_create_value_from_expression(lldb_value_t value, const char* name, const char* expr) LLDB_WRAPPER_NOEXCEPT;
lldb_watchpoint_t lldb_value_watch(lldb_value_t value, int resolve_location, int read, int write, lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_value_get_expression_path(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
int lldb_value_is_pointer_type(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_value_get_non_synthetic_value(lldb_value_t value) LLDB_WRAPPER_NOEXCEPT;

// SBValueList
void lldb_value_list_destroy(lldb_value_list_t list) LLDB_WRAPPER_NOEXCEPT;
int lldb_value_list_is_valid(lldb_value_list_t list) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_value_list_get_size(lldb_value_list_t list) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_value_list_get_value_at_index(lldb_value_list_t list, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
lldb_value_t lldb_value_list_get_first_value_by_name(lldb_value_list_t list, const char* name) LLDB_WRAPPER_NOEXCEPT;

// SBError
lldb_error_t lldb_error_create(void) LLDB_WRAPPER_NOEXCEPT;
void lldb_error_destroy(lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
int lldb_error_success(lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
int lldb_error_fail(lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_error_get_cstring(lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_error_get_error(lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
int lldb_error_get_type(lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
void lldb_error_clear(lldb_error_t error) LLDB_WRAPPER_NOEXCEPT;
void lldb_error_set_error_string(lldb_error_t error, const char* str) LLDB_WRAPPER_NOEXCEPT;

// SBModule
void lldb_module_destroy(lldb_module_t module) LLDB_WRAPPER_NOEXCEPT;
int lldb_module_is_valid(lldb_module_t module) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_module_get_file_path(lldb_module_t module) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_module_get_platform_file_path(lldb_module_t module) LLDB_WRAPPER_NOEXCEPT;
lldb_file_spec_t lldb_module_get_file(lldb_module_t module) LLDB_WRAPPER_NOEXCEPT;
lldb_file_spec_t lldb_module_get_platform_file(lldb_module_t module) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_module_get_num_symbols(lldb_module_t module) LLDB_WRAPPER_NOEXCEPT;
lldb_symbol_t lldb_module_get_symbol_at_index(lldb_module_t module, uint32_t index) LLDB_WRAPPER_NOEXCEPT;

// SBSymbol
void lldb_symbol_destroy(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
int lldb_symbol_is_valid(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_symbol_get_name(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_symbol_get_display_name(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_symbol_get_mangled_name(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_symbol_get_base_name(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
lldb_address_t lldb_symbol_get_start_address(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
lldb_address_t lldb_symbol_get_end_address(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_symbol_get_value(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_symbol_get_size(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_symbol_get_prologue_byte_size(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
int lldb_symbol_get_type(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_symbol_get_id(lldb_symbol_t symbol) LLDB_WRAPPER_NOEXCEPT;

// SBFunction
void lldb_function_destroy(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
int lldb_function_is_valid(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_function_get_name(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_function_get_display_name(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_function_get_mangled_name(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_function_get_base_name(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
lldb_address_t lldb_function_get_start_address(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
lldb_address_t lldb_function_get_end_address(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_function_get_prologue_byte_size(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
lldb_type_t lldb_function_get_type(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
lldb_block_t lldb_function_get_block(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
int lldb_function_is_optimized(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;
int lldb_function_get_language(lldb_function_t function) LLDB_WRAPPER_NOEXCEPT;

// SBCompileUnit
void lldb_compile_unit_destroy(lldb_compile_unit_t unit) LLDB_WRAPPER_NOEXCEPT;
int lldb_compile_unit_is_valid(lldb_compile_unit_t unit) LLDB_WRAPPER_NOEXCEPT;
lldb_file_spec_t lldb_compile_unit_get_file_spec(lldb_compile_unit_t unit) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_compile_unit_get_num_line_entries(lldb_compile_unit_t unit) LLDB_WRAPPER_NOEXCEPT;
lldb_line_entry_t lldb_compile_unit_get_line_entry_at_index(lldb_compile_unit_t unit,
                                                             uint32_t index) LLDB_WRAPPER_NOEXCEPT;

// SBBlock
void lldb_block_destroy(lldb_block_t block) LLDB_WRAPPER_NOEXCEPT;
int lldb_block_is_valid(lldb_block_t block) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_block_get_inlined_name(lldb_block_t block) LLDB_WRAPPER_NOEXCEPT;
lldb_file_spec_t lldb_block_get_inlined_call_site_file(lldb_block_t block) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_block_get_inlined_call_site_line(lldb_block_t block) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_block_get_inlined_call_site_column(lldb_block_t block) LLDB_WRAPPER_NOEXCEPT;
lldb_block_t lldb_block_get_parent(lldb_block_t block) LLDB_WRAPPER_NOEXCEPT;
lldb_block_t lldb_block_get_sibling(lldb_block_t block) LLDB_WRAPPER_NOEXCEPT;
lldb_block_t lldb_block_get_first_child(lldb_block_t block) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_block_get_num_ranges(lldb_block_t block) LLDB_WRAPPER_NOEXCEPT;
lldb_address_t lldb_block_get_range_start_address(lldb_block_t block, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
lldb_address_t lldb_block_get_range_end_address(lldb_block_t block, uint32_t index) LLDB_WRAPPER_NOEXCEPT;

// SBInstructionList
void lldb_instruction_list_destroy(lldb_instruction_list_t list) LLDB_WRAPPER_NOEXCEPT;
int lldb_instruction_list_is_valid(lldb_instruction_list_t list) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_instruction_list_get_size(lldb_instruction_list_t list) LLDB_WRAPPER_NOEXCEPT;
lldb_instruction_t lldb_instruction_list_get_instruction_at_index(lldb_instruction_list_t list,
                                                                   uint32_t index) LLDB_WRAPPER_NOEXCEPT;

// SBInstruction
void lldb_instruction_destroy(lldb_instruction_t instruction) LLDB_WRAPPER_NOEXCEPT;
int lldb_instruction_is_valid(lldb_instruction_t instruction) LLDB_WRAPPER_NOEXCEPT;
lldb_address_t lldb_instruction_get_address(lldb_instruction_t instruction) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_instruction_get_mnemonic(lldb_instruction_t instruction, lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_instruction_get_operands(lldb_instruction_t instruction, lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_instruction_get_comment(lldb_instruction_t instruction, lldb_target_t target) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_instruction_get_byte_size(lldb_instruction_t instruction) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_instruction_get_bytes(lldb_instruction_t instruction,
                                    lldb_target_t target,
                                    uint8_t* buffer,
                                    uint32_t length) LLDB_WRAPPER_NOEXCEPT;

// SBSymbolContext
void lldb_symbol_context_destroy(lldb_symbol_context_t ctx) LLDB_WRAPPER_NOEXCEPT;
int lldb_symbol_context_is_valid(lldb_symbol_context_t ctx) LLDB_WRAPPER_NOEXCEPT;
lldb_module_t lldb_symbol_context_get_module(lldb_symbol_context_t ctx) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_symbol_context_get_function_name(lldb_symbol_context_t ctx) LLDB_WRAPPER_NOEXCEPT;

// SBType
void lldb_type_destroy(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
int lldb_type_is_valid(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_type_get_name(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_type_get_display_type_name(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_type_get_byte_size(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
int lldb_type_is_pointer_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
int lldb_type_is_reference_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
int lldb_type_is_array_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
int lldb_type_is_vector_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
int lldb_type_is_typedef_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
int lldb_type_is_function_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
int lldb_type_is_polymorphic_class(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
lldb_type_t lldb_type_get_pointer_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
lldb_type_t lldb_type_get_pointee_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
lldb_type_t lldb_type_get_reference_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
lldb_type_t lldb_type_get_dereferenced_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
lldb_type_t lldb_type_get_unqualified_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
lldb_type_t lldb_type_get_canonical_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
lldb_type_t lldb_type_get_array_element_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_type_get_array_size(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_type_get_num_fields(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_type_get_num_direct_base_classes(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_type_get_num_virtual_base_classes(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;
lldb_type_member_t lldb_type_get_field_at_index(lldb_type_t type, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
lldb_type_member_t lldb_type_get_direct_base_class_at_index(lldb_type_t type, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
lldb_type_member_t lldb_type_get_virtual_base_class_at_index(lldb_type_t type, uint32_t index) LLDB_WRAPPER_NOEXCEPT;
int lldb_type_get_basic_type(lldb_type_t type) LLDB_WRAPPER_NOEXCEPT;

// SBTypeMember
void lldb_type_member_destroy(lldb_type_member_t member) LLDB_WRAPPER_NOEXCEPT;
int lldb_type_member_is_valid(lldb_type_member_t member) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_type_member_get_name(lldb_type_member_t member) LLDB_WRAPPER_NOEXCEPT;
lldb_type_t lldb_type_member_get_type(lldb_type_member_t member) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_type_member_get_offset_in_bytes(lldb_type_member_t member) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_type_member_get_offset_in_bits(lldb_type_member_t member) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_type_member_get_bitfield_size_in_bits(lldb_type_member_t member) LLDB_WRAPPER_NOEXCEPT;

// SBWatchpoint
void lldb_watchpoint_destroy(lldb_watchpoint_t wp) LLDB_WRAPPER_NOEXCEPT;
int lldb_watchpoint_is_valid(lldb_watchpoint_t wp) LLDB_WRAPPER_NOEXCEPT;
int32_t lldb_watchpoint_get_id(lldb_watchpoint_t wp) LLDB_WRAPPER_NOEXCEPT;
int lldb_watchpoint_is_enabled(lldb_watchpoint_t wp) LLDB_WRAPPER_NOEXCEPT;
void lldb_watchpoint_set_enabled(lldb_watchpoint_t wp, int enabled) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_watchpoint_get_hit_count(lldb_watchpoint_t wp) LLDB_WRAPPER_NOEXCEPT;
uint32_t lldb_watchpoint_get_ignore_count(lldb_watchpoint_t wp) LLDB_WRAPPER_NOEXCEPT;
void lldb_watchpoint_set_ignore_count(lldb_watchpoint_t wp, uint32_t count) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_watchpoint_get_condition(lldb_watchpoint_t wp) LLDB_WRAPPER_NOEXCEPT;
void lldb_watchpoint_set_condition(lldb_watchpoint_t wp, const char* condition) LLDB_WRAPPER_NOEXCEPT;
uint64_t lldb_watchpoint_get_watch_address(lldb_watchpoint_t wp) LLDB_WRAPPER_NOEXCEPT;
size_t lldb_watchpoint_get_watch_size(lldb_watchpoint_t wp) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_watchpoint_is_watching_reads(lldb_watchpoint_t wp, int* result) LLDB_WRAPPER_NOEXCEPT;
lldb_ruby_status_t lldb_watchpoint_is_watching_writes(lldb_watchpoint_t wp, int* result) LLDB_WRAPPER_NOEXCEPT;

// SBCommandInterpreter
void lldb_command_interpreter_destroy(lldb_command_interpreter_t interp) LLDB_WRAPPER_NOEXCEPT;
int lldb_command_interpreter_is_valid(lldb_command_interpreter_t interp) LLDB_WRAPPER_NOEXCEPT;
int lldb_command_interpreter_handle_command(lldb_command_interpreter_t interp,
                                             const char* command,
                                             lldb_command_return_object_t result,
                                             int add_to_history) LLDB_WRAPPER_NOEXCEPT;
int lldb_command_interpreter_command_exists(lldb_command_interpreter_t interp, const char* command) LLDB_WRAPPER_NOEXCEPT;
int lldb_command_interpreter_alias_exists(lldb_command_interpreter_t interp, const char* alias) LLDB_WRAPPER_NOEXCEPT;

// SBCommandReturnObject
lldb_command_return_object_t lldb_command_return_object_create(void) LLDB_WRAPPER_NOEXCEPT;
void lldb_command_return_object_destroy(lldb_command_return_object_t obj) LLDB_WRAPPER_NOEXCEPT;
int lldb_command_return_object_is_valid(lldb_command_return_object_t obj) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_command_return_object_get_output(lldb_command_return_object_t obj) LLDB_WRAPPER_NOEXCEPT;
const char* lldb_command_return_object_get_error(lldb_command_return_object_t obj) LLDB_WRAPPER_NOEXCEPT;
int lldb_command_return_object_get_status(lldb_command_return_object_t obj) LLDB_WRAPPER_NOEXCEPT;
int lldb_command_return_object_succeeded(lldb_command_return_object_t obj) LLDB_WRAPPER_NOEXCEPT;
int lldb_command_return_object_has_result(lldb_command_return_object_t obj) LLDB_WRAPPER_NOEXCEPT;
void lldb_command_return_object_clear(lldb_command_return_object_t obj) LLDB_WRAPPER_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#endif // LLDB_WRAPPER_H
