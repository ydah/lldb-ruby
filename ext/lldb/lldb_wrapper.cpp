#include "lldb_wrapper.h"
#include <lldb/API/LLDB.h>

#include <string>
#include <cstring>

// LLDB version detection
// LLDB_VERSION_MAJOR is defined in lldb/lldb-defines.h (part of LLDB.h)
#ifndef LLDB_VERSION_MAJOR
#define LLDB_VERSION_MAJOR 0
#endif

// Thread-local storage for temporary strings
static thread_local std::string g_temp_string;
static thread_local std::string g_temp_string2;

extern "C" {

// ============================================================================
// Initialization
// ============================================================================

void lldb_initialize(void) {
    lldb::SBDebugger::Initialize();
}

void lldb_terminate(void) {
    lldb::SBDebugger::Terminate();
}

// ============================================================================
// SBDebugger
// ============================================================================

lldb_debugger_t lldb_debugger_create(void) {
    lldb::SBDebugger* dbg = new lldb::SBDebugger(lldb::SBDebugger::Create(false));
    return static_cast<lldb_debugger_t>(dbg);
}

void lldb_debugger_destroy(lldb_debugger_t dbg) {
    if (dbg) {
        lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
        if (debugger->IsValid()) {
            lldb::SBDebugger::Destroy(*debugger);
        }
        delete debugger;
    }
}

int lldb_debugger_is_valid(lldb_debugger_t dbg) {
    if (!dbg) return 0;
    return static_cast<lldb::SBDebugger*>(dbg)->IsValid() ? 1 : 0;
}

lldb_target_t lldb_debugger_create_target(lldb_debugger_t dbg,
                                           const char* filename,
                                           const char* arch,
                                           const char* platform,
                                           int add_dependent_modules,
                                           lldb_error_t error) {
    if (!dbg) return nullptr;

    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    lldb::SBTarget target = debugger->CreateTarget(
        filename,
        arch ? arch : "",
        platform ? platform : "",
        add_dependent_modules != 0,
        err ? *err : local_error
    );

    if (!target.IsValid()) return nullptr;

    return static_cast<lldb_target_t>(new lldb::SBTarget(target));
}

lldb_target_t lldb_debugger_create_target_simple(lldb_debugger_t dbg,
                                                  const char* filename) {
    if (!dbg) return nullptr;

    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget target = debugger->CreateTarget(filename);

    if (!target.IsValid()) return nullptr;

    return static_cast<lldb_target_t>(new lldb::SBTarget(target));
}

uint32_t lldb_debugger_get_num_targets(lldb_debugger_t dbg) {
    if (!dbg) return 0;
    return static_cast<lldb::SBDebugger*>(dbg)->GetNumTargets();
}

lldb_target_t lldb_debugger_get_target_at_index(lldb_debugger_t dbg, uint32_t index) {
    if (!dbg) return nullptr;

    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget target = debugger->GetTargetAtIndex(index);

    if (!target.IsValid()) return nullptr;

    return static_cast<lldb_target_t>(new lldb::SBTarget(target));
}

lldb_target_t lldb_debugger_get_selected_target(lldb_debugger_t dbg) {
    if (!dbg) return nullptr;

    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget target = debugger->GetSelectedTarget();

    if (!target.IsValid()) return nullptr;

    return static_cast<lldb_target_t>(new lldb::SBTarget(target));
}

void lldb_debugger_set_async(lldb_debugger_t dbg, int async) {
    if (!dbg) return;
    static_cast<lldb::SBDebugger*>(dbg)->SetAsync(async != 0);
}

int lldb_debugger_get_async(lldb_debugger_t dbg) {
    if (!dbg) return 0;
    return static_cast<lldb::SBDebugger*>(dbg)->GetAsync() ? 1 : 0;
}

void lldb_debugger_set_selected_target(lldb_debugger_t dbg, lldb_target_t target) {
    if (!dbg || !target) return;
    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    debugger->SetSelectedTarget(*t);
}

int lldb_debugger_delete_target(lldb_debugger_t dbg, lldb_target_t target) {
    if (!dbg || !target) return 0;
    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    return debugger->DeleteTarget(*t) ? 1 : 0;
}

lldb_target_t lldb_debugger_find_target_with_process_id(lldb_debugger_t dbg, uint64_t pid) {
    if (!dbg) return nullptr;
    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget target = debugger->FindTargetWithProcessID(pid);
    if (!target.IsValid()) return nullptr;
    return static_cast<lldb_target_t>(new lldb::SBTarget(target));
}

const char* lldb_debugger_get_version_string(void) {
    return lldb::SBDebugger::GetVersionString();
}

lldb_command_interpreter_t lldb_debugger_get_command_interpreter(lldb_debugger_t dbg) {
    if (!dbg) return nullptr;
    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBCommandInterpreter interp = debugger->GetCommandInterpreter();
    if (!interp.IsValid()) return nullptr;
    return static_cast<lldb_command_interpreter_t>(new lldb::SBCommandInterpreter(interp));
}

void lldb_debugger_handle_command(lldb_debugger_t dbg, const char* command) {
    if (!dbg || !command) return;
    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    debugger->HandleCommand(command);
}

// ============================================================================
// SBTarget
// ============================================================================

void lldb_target_destroy(lldb_target_t target) {
    if (target) {
        delete static_cast<lldb::SBTarget*>(target);
    }
}

int lldb_target_is_valid(lldb_target_t target) {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->IsValid() ? 1 : 0;
}

lldb_process_t lldb_target_launch_simple(lldb_target_t target,
                                          const char** argv,
                                          const char** envp,
                                          const char* working_dir) {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBProcess process = t->LaunchSimple(argv, envp, working_dir);

    if (!process.IsValid()) return nullptr;

    return static_cast<lldb_process_t>(new lldb::SBProcess(process));
}

lldb_process_t lldb_target_launch(lldb_target_t target,
                                   lldb_launch_info_t launch_info,
                                   lldb_error_t error) {
    if (!target || !launch_info) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBLaunchInfo* info = static_cast<lldb::SBLaunchInfo*>(launch_info);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    lldb::SBProcess process = t->Launch(*info, err ? *err : local_error);

    if (!process.IsValid()) return nullptr;

    return static_cast<lldb_process_t>(new lldb::SBProcess(process));
}

lldb_process_t lldb_target_attach_to_process_with_id(lldb_target_t target,
                                                      uint64_t pid,
                                                      lldb_error_t error) {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;
    lldb::SBListener listener;

    lldb::SBProcess process = t->AttachToProcessWithID(
        listener,
        pid,
        err ? *err : local_error
    );

    if (!process.IsValid()) return nullptr;

    return static_cast<lldb_process_t>(new lldb::SBProcess(process));
}

lldb_process_t lldb_target_attach_to_process_with_name(lldb_target_t target,
                                                        const char* name,
                                                        int wait_for,
                                                        lldb_error_t error) {
    if (!target || !name) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;
    lldb::SBListener listener;

    lldb::SBProcess process = t->AttachToProcessWithName(
        listener,
        name,
        wait_for != 0,
        err ? *err : local_error
    );

    if (!process.IsValid()) return nullptr;

    return static_cast<lldb_process_t>(new lldb::SBProcess(process));
}

lldb_breakpoint_t lldb_target_breakpoint_create_by_name(lldb_target_t target,
                                                         const char* symbol_name,
                                                         const char* module_name) {
    if (!target || !symbol_name) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->BreakpointCreateByName(symbol_name, module_name);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));
}

lldb_breakpoint_t lldb_target_breakpoint_create_by_location(lldb_target_t target,
                                                             const char* file,
                                                             uint32_t line) {
    if (!target || !file) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->BreakpointCreateByLocation(file, line);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));
}

lldb_breakpoint_t lldb_target_breakpoint_create_by_address(lldb_target_t target,
                                                            uint64_t address) {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->BreakpointCreateByAddress(address);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));
}

lldb_breakpoint_t lldb_target_breakpoint_create_by_regex(lldb_target_t target,
                                                          const char* symbol_regex,
                                                          const char* module_name) {
    if (!target || !symbol_regex) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->BreakpointCreateByRegex(symbol_regex, module_name);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));
}

lldb_breakpoint_t lldb_target_breakpoint_create_by_source_regex(lldb_target_t target,
                                                                 const char* source_regex,
                                                                 const char* source_file) {
    if (!target || !source_regex) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBFileSpecList module_list;
    lldb::SBFileSpecList source_list;
    if (source_file) {
        source_list.Append(lldb::SBFileSpec(source_file));
    }
    lldb::SBBreakpoint bp = t->BreakpointCreateBySourceRegex(source_regex, module_list, source_list);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));
}

int lldb_target_delete_breakpoint(lldb_target_t target, int32_t breakpoint_id) {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->BreakpointDelete(breakpoint_id) ? 1 : 0;
}

int lldb_target_delete_all_breakpoints(lldb_target_t target) {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->DeleteAllBreakpoints() ? 1 : 0;
}

int lldb_target_enable_all_breakpoints(lldb_target_t target) {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->EnableAllBreakpoints() ? 1 : 0;
}

int lldb_target_disable_all_breakpoints(lldb_target_t target) {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->DisableAllBreakpoints() ? 1 : 0;
}

lldb_breakpoint_t lldb_target_find_breakpoint_by_id(lldb_target_t target, int32_t id) {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->FindBreakpointByID(id);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));
}

uint32_t lldb_target_get_num_breakpoints(lldb_target_t target) {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->GetNumBreakpoints();
}

lldb_breakpoint_t lldb_target_get_breakpoint_at_index(lldb_target_t target, uint32_t index) {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->GetBreakpointAtIndex(index);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));
}

lldb_process_t lldb_target_get_process(lldb_target_t target) {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBProcess process = t->GetProcess();

    if (!process.IsValid()) return nullptr;

    return static_cast<lldb_process_t>(new lldb::SBProcess(process));
}

const char* lldb_target_get_executable_path(lldb_target_t target) {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBFileSpec spec = t->GetExecutable();

    if (!spec.IsValid()) return nullptr;

    char path[4096];
    if (spec.GetPath(path, sizeof(path)) > 0) {
        g_temp_string = path;
        return g_temp_string.c_str();
    }

    return nullptr;
}

uint32_t lldb_target_get_num_modules(lldb_target_t target) {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->GetNumModules();
}

lldb_module_t lldb_target_get_module_at_index(lldb_target_t target, uint32_t index) {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBModule module = t->GetModuleAtIndex(index);

    if (!module.IsValid()) return nullptr;

    return static_cast<lldb_module_t>(new lldb::SBModule(module));
}

lldb_value_t lldb_target_evaluate_expression(lldb_target_t target, const char* expr) {
    if (!target || !expr) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBValue value = t->EvaluateExpression(expr);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));
}

size_t lldb_target_read_memory(lldb_target_t target, uint64_t addr, void* buf, size_t size, lldb_error_t error) {
    if (!target || !buf) return 0;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;
    lldb::SBAddress sb_addr(addr, *t);

    return t->ReadMemory(sb_addr, buf, size, err ? *err : local_error);
}

uint32_t lldb_target_get_address_byte_size(lldb_target_t target) {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->GetAddressByteSize();
}

const char* lldb_target_get_triple(lldb_target_t target) {
    if (!target) return nullptr;
    return static_cast<lldb::SBTarget*>(target)->GetTriple();
}

lldb_watchpoint_t lldb_target_watch_address(lldb_target_t target, uint64_t addr, size_t size, int read, int write, lldb_error_t error) {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    lldb::SBWatchpoint wp = t->WatchAddress(addr, size, read != 0, write != 0, err ? *err : local_error);

    if (!wp.IsValid()) return nullptr;

    return static_cast<lldb_watchpoint_t>(new lldb::SBWatchpoint(wp));
}

int lldb_target_delete_watchpoint(lldb_target_t target, int32_t watchpoint_id) {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->DeleteWatchpoint(watchpoint_id) ? 1 : 0;
}

int lldb_target_delete_all_watchpoints(lldb_target_t target) {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->DeleteAllWatchpoints() ? 1 : 0;
}

lldb_watchpoint_t lldb_target_find_watchpoint_by_id(lldb_target_t target, int32_t id) {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBWatchpoint wp = t->FindWatchpointByID(id);

    if (!wp.IsValid()) return nullptr;

    return static_cast<lldb_watchpoint_t>(new lldb::SBWatchpoint(wp));
}

uint32_t lldb_target_get_num_watchpoints(lldb_target_t target) {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->GetNumWatchpoints();
}

lldb_watchpoint_t lldb_target_get_watchpoint_at_index(lldb_target_t target, uint32_t index) {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBWatchpoint wp = t->GetWatchpointAtIndex(index);

    if (!wp.IsValid()) return nullptr;

    return static_cast<lldb_watchpoint_t>(new lldb::SBWatchpoint(wp));
}

// ============================================================================
// SBLaunchInfo
// ============================================================================

lldb_launch_info_t lldb_launch_info_create(const char** argv) {
    return static_cast<lldb_launch_info_t>(new lldb::SBLaunchInfo(argv));
}

void lldb_launch_info_destroy(lldb_launch_info_t info) {
    if (info) {
        delete static_cast<lldb::SBLaunchInfo*>(info);
    }
}

void lldb_launch_info_set_working_directory(lldb_launch_info_t info, const char* dir) {
    if (!info) return;
    static_cast<lldb::SBLaunchInfo*>(info)->SetWorkingDirectory(dir);
}

void lldb_launch_info_set_environment_entries(lldb_launch_info_t info,
                                               const char** envp,
                                               int append) {
    if (!info) return;
    lldb::SBEnvironment env;
    if (envp) {
        for (const char** e = envp; *e != nullptr; ++e) {
            std::string entry(*e);
            size_t pos = entry.find('=');
            if (pos != std::string::npos) {
                env.Set(entry.substr(0, pos).c_str(),
                       entry.substr(pos + 1).c_str(),
                       true);
            }
        }
    }
    static_cast<lldb::SBLaunchInfo*>(info)->SetEnvironment(env, append != 0);
}

uint32_t lldb_launch_info_get_launch_flags(lldb_launch_info_t info) {
    if (!info) return 0;
    return static_cast<lldb::SBLaunchInfo*>(info)->GetLaunchFlags();
}

void lldb_launch_info_set_launch_flags(lldb_launch_info_t info, uint32_t flags) {
    if (!info) return;
    static_cast<lldb::SBLaunchInfo*>(info)->SetLaunchFlags(flags);
}

// ============================================================================
// SBProcess
// ============================================================================

void lldb_process_destroy(lldb_process_t process) {
    if (process) {
        delete static_cast<lldb::SBProcess*>(process);
    }
}

int lldb_process_is_valid(lldb_process_t process) {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->IsValid() ? 1 : 0;
}

int lldb_process_continue(lldb_process_t process) {
    if (!process) return 0;
    lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Continue();
    return error.Success() ? 1 : 0;
}

int lldb_process_stop(lldb_process_t process) {
    if (!process) return 0;
    lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Stop();
    return error.Success() ? 1 : 0;
}

int lldb_process_kill(lldb_process_t process) {
    if (!process) return 0;
    lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Kill();
    return error.Success() ? 1 : 0;
}

int lldb_process_detach(lldb_process_t process) {
    if (!process) return 0;
    lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Detach();
    return error.Success() ? 1 : 0;
}

int lldb_process_destroy_process(lldb_process_t process) {
    if (!process) return 0;
    lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Destroy();
    return error.Success() ? 1 : 0;
}

int lldb_process_signal(lldb_process_t process, int signal) {
    if (!process) return 0;
    lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Signal(signal);
    return error.Success() ? 1 : 0;
}

int lldb_process_get_state(lldb_process_t process) {
    if (!process) return LLDB_STATE_INVALID;
    return static_cast<int>(static_cast<lldb::SBProcess*>(process)->GetState());
}

uint32_t lldb_process_get_num_threads(lldb_process_t process) {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->GetNumThreads();
}

lldb_thread_t lldb_process_get_thread_at_index(lldb_process_t process, uint32_t index) {
    if (!process) return nullptr;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBThread thread = p->GetThreadAtIndex(index);

    if (!thread.IsValid()) return nullptr;

    return static_cast<lldb_thread_t>(new lldb::SBThread(thread));
}

lldb_thread_t lldb_process_get_thread_by_id(lldb_process_t process, uint64_t tid) {
    if (!process) return nullptr;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBThread thread = p->GetThreadByID(tid);

    if (!thread.IsValid()) return nullptr;

    return static_cast<lldb_thread_t>(new lldb::SBThread(thread));
}

lldb_thread_t lldb_process_get_thread_by_index_id(lldb_process_t process, uint32_t index_id) {
    if (!process) return nullptr;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBThread thread = p->GetThreadByIndexID(index_id);

    if (!thread.IsValid()) return nullptr;

    return static_cast<lldb_thread_t>(new lldb::SBThread(thread));
}

lldb_thread_t lldb_process_get_selected_thread(lldb_process_t process) {
    if (!process) return nullptr;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBThread thread = p->GetSelectedThread();

    if (!thread.IsValid()) return nullptr;

    return static_cast<lldb_thread_t>(new lldb::SBThread(thread));
}

int lldb_process_set_selected_thread_by_id(lldb_process_t process, uint64_t tid) {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->SetSelectedThreadByID(tid) ? 1 : 0;
}

int lldb_process_set_selected_thread_by_index_id(lldb_process_t process, uint32_t index_id) {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->SetSelectedThreadByIndexID(index_id) ? 1 : 0;
}

uint64_t lldb_process_get_process_id(lldb_process_t process) {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->GetProcessID();
}

int lldb_process_get_exit_status(lldb_process_t process) {
    if (!process) return -1;
    return static_cast<lldb::SBProcess*>(process)->GetExitStatus();
}

const char* lldb_process_get_exit_description(lldb_process_t process) {
    if (!process) return nullptr;
    return static_cast<lldb::SBProcess*>(process)->GetExitDescription();
}

size_t lldb_process_read_memory(lldb_process_t process, uint64_t addr, void* buf, size_t size, lldb_error_t error) {
    if (!process || !buf) return 0;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    return p->ReadMemory(addr, buf, size, err ? *err : local_error);
}

size_t lldb_process_write_memory(lldb_process_t process, uint64_t addr, const void* buf, size_t size, lldb_error_t error) {
    if (!process || !buf) return 0;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    return p->WriteMemory(addr, buf, size, err ? *err : local_error);
}

uint64_t lldb_process_allocate_memory(lldb_process_t process, size_t size, uint32_t permissions, lldb_error_t error) {
    if (!process) return LLDB_INVALID_ADDRESS;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    return p->AllocateMemory(size, permissions, err ? *err : local_error);
}

int lldb_process_deallocate_memory(lldb_process_t process, uint64_t addr) {
    if (!process) return 0;
    lldb::SBError error = static_cast<lldb::SBProcess*>(process)->DeallocateMemory(addr);
    return error.Success() ? 1 : 0;
}

size_t lldb_process_read_cstring_from_memory(lldb_process_t process, uint64_t addr, void* buf, size_t size, lldb_error_t error) {
    if (!process || !buf) return 0;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    return p->ReadCStringFromMemory(addr, buf, size, err ? *err : local_error);
}

size_t lldb_process_get_stdout(lldb_process_t process, char* buf, size_t size) {
    if (!process || !buf) return 0;
    return static_cast<lldb::SBProcess*>(process)->GetSTDOUT(buf, size);
}

size_t lldb_process_get_stderr(lldb_process_t process, char* buf, size_t size) {
    if (!process || !buf) return 0;
    return static_cast<lldb::SBProcess*>(process)->GetSTDERR(buf, size);
}

size_t lldb_process_put_stdin(lldb_process_t process, const char* buf, size_t size) {
    if (!process || !buf) return 0;
    return static_cast<lldb::SBProcess*>(process)->PutSTDIN(buf, size);
}

int lldb_process_send_async_interrupt(lldb_process_t process) {
    if (!process) return 0;
    static_cast<lldb::SBProcess*>(process)->SendAsyncInterrupt();
    return 1;
}

uint32_t lldb_process_get_num_supported_hardware_watchpoints(lldb_process_t process, lldb_error_t error) {
    if (!process) return 0;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    return p->GetNumSupportedHardwareWatchpoints(err ? *err : local_error);
}

uint32_t lldb_process_get_unique_id(lldb_process_t process) {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->GetUniqueID();
}

lldb_memory_region_info_t lldb_process_get_memory_region_info(lldb_process_t process, uint64_t addr, lldb_error_t error) {
    if (!process) return nullptr;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    lldb::SBMemoryRegionInfo* info = new lldb::SBMemoryRegionInfo();
    lldb::SBError result = p->GetMemoryRegionInfo(addr, *info);

    if (result.Fail()) {
        if (err) *err = result;
        delete info;
        return nullptr;
    }

    return static_cast<lldb_memory_region_info_t>(info);
}

// ============================================================================
// SBMemoryRegionInfo
// ============================================================================

void lldb_memory_region_info_destroy(lldb_memory_region_info_t info) {
    if (info) {
        delete static_cast<lldb::SBMemoryRegionInfo*>(info);
    }
}

uint64_t lldb_memory_region_info_get_region_base(lldb_memory_region_info_t info) {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->GetRegionBase();
}

uint64_t lldb_memory_region_info_get_region_end(lldb_memory_region_info_t info) {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->GetRegionEnd();
}

int lldb_memory_region_info_is_readable(lldb_memory_region_info_t info) {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->IsReadable() ? 1 : 0;
}

int lldb_memory_region_info_is_writable(lldb_memory_region_info_t info) {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->IsWritable() ? 1 : 0;
}

int lldb_memory_region_info_is_executable(lldb_memory_region_info_t info) {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->IsExecutable() ? 1 : 0;
}

int lldb_memory_region_info_is_mapped(lldb_memory_region_info_t info) {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->IsMapped() ? 1 : 0;
}

const char* lldb_memory_region_info_get_name(lldb_memory_region_info_t info) {
    if (!info) return nullptr;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->GetName();
}

// ============================================================================
// SBThread
// ============================================================================

void lldb_thread_destroy(lldb_thread_t thread) {
    if (thread) {
        delete static_cast<lldb::SBThread*>(thread);
    }
}

int lldb_thread_is_valid(lldb_thread_t thread) {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->IsValid() ? 1 : 0;
}

int lldb_thread_step_over(lldb_thread_t thread) {
    if (!thread) return 0;
    static_cast<lldb::SBThread*>(thread)->StepOver();
    return 1;
}

int lldb_thread_step_into(lldb_thread_t thread) {
    if (!thread) return 0;
    static_cast<lldb::SBThread*>(thread)->StepInto();
    return 1;
}

int lldb_thread_step_out(lldb_thread_t thread) {
    if (!thread) return 0;
    static_cast<lldb::SBThread*>(thread)->StepOut();
    return 1;
}

int lldb_thread_step_instruction(lldb_thread_t thread, int step_over) {
    if (!thread) return 0;
    static_cast<lldb::SBThread*>(thread)->StepInstruction(step_over != 0);
    return 1;
}

int lldb_thread_run_to_address(lldb_thread_t thread, uint64_t addr) {
    if (!thread) return 0;
    static_cast<lldb::SBThread*>(thread)->RunToAddress(addr);
    return 1;
}

uint32_t lldb_thread_get_num_frames(lldb_thread_t thread) {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->GetNumFrames();
}

lldb_frame_t lldb_thread_get_frame_at_index(lldb_thread_t thread, uint32_t index) {
    if (!thread) return nullptr;

    lldb::SBThread* t = static_cast<lldb::SBThread*>(thread);
    lldb::SBFrame frame = t->GetFrameAtIndex(index);

    if (!frame.IsValid()) return nullptr;

    return static_cast<lldb_frame_t>(new lldb::SBFrame(frame));
}

lldb_frame_t lldb_thread_get_selected_frame(lldb_thread_t thread) {
    if (!thread) return nullptr;

    lldb::SBThread* t = static_cast<lldb::SBThread*>(thread);
    lldb::SBFrame frame = t->GetSelectedFrame();

    if (!frame.IsValid()) return nullptr;

    return static_cast<lldb_frame_t>(new lldb::SBFrame(frame));
}

int lldb_thread_set_selected_frame(lldb_thread_t thread, uint32_t index) {
    if (!thread) return 0;
    lldb::SBFrame frame = static_cast<lldb::SBThread*>(thread)->SetSelectedFrame(index);
    return frame.IsValid() ? 1 : 0;
}

uint64_t lldb_thread_get_thread_id(lldb_thread_t thread) {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->GetThreadID();
}

uint32_t lldb_thread_get_index_id(lldb_thread_t thread) {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->GetIndexID();
}

const char* lldb_thread_get_name(lldb_thread_t thread) {
    if (!thread) return nullptr;
    return static_cast<lldb::SBThread*>(thread)->GetName();
}

const char* lldb_thread_get_queue_name(lldb_thread_t thread) {
    if (!thread) return nullptr;
    return static_cast<lldb::SBThread*>(thread)->GetQueueName();
}

int lldb_thread_get_stop_reason(lldb_thread_t thread) {
    if (!thread) return LLDB_STOP_REASON_INVALID;
    return static_cast<int>(static_cast<lldb::SBThread*>(thread)->GetStopReason());
}

const char* lldb_thread_get_stop_description(lldb_thread_t thread, size_t max_size) {
    if (!thread) return nullptr;

    static thread_local std::string stop_desc;
    stop_desc.resize(max_size);
    size_t len = static_cast<lldb::SBThread*>(thread)->GetStopDescription(&stop_desc[0], max_size);
    stop_desc.resize(len);
    return stop_desc.c_str();
}

uint64_t lldb_thread_get_stop_reason_data_count(lldb_thread_t thread) {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->GetStopReasonDataCount();
}

uint64_t lldb_thread_get_stop_reason_data_at_index(lldb_thread_t thread, uint32_t index) {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->GetStopReasonDataAtIndex(index);
}

int lldb_thread_is_stopped(lldb_thread_t thread) {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->IsStopped() ? 1 : 0;
}

int lldb_thread_is_suspended(lldb_thread_t thread) {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->IsSuspended() ? 1 : 0;
}

int lldb_thread_suspend(lldb_thread_t thread) {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->Suspend() ? 1 : 0;
}

int lldb_thread_resume(lldb_thread_t thread) {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->Resume() ? 1 : 0;
}

lldb_process_t lldb_thread_get_process(lldb_thread_t thread) {
    if (!thread) return nullptr;

    lldb::SBThread* t = static_cast<lldb::SBThread*>(thread);
    lldb::SBProcess process = t->GetProcess();

    if (!process.IsValid()) return nullptr;

    return static_cast<lldb_process_t>(new lldb::SBProcess(process));
}

// ============================================================================
// SBFrame
// ============================================================================

void lldb_frame_destroy(lldb_frame_t frame) {
    if (frame) {
        delete static_cast<lldb::SBFrame*>(frame);
    }
}

int lldb_frame_is_valid(lldb_frame_t frame) {
    if (!frame) return 0;
    return static_cast<lldb::SBFrame*>(frame)->IsValid() ? 1 : 0;
}

const char* lldb_frame_get_function_name(lldb_frame_t frame) {
    if (!frame) return nullptr;
    return static_cast<lldb::SBFrame*>(frame)->GetFunctionName();
}

const char* lldb_frame_get_display_function_name(lldb_frame_t frame) {
    if (!frame) return nullptr;
    return static_cast<lldb::SBFrame*>(frame)->GetDisplayFunctionName();
}

uint32_t lldb_frame_get_line(lldb_frame_t frame) {
    if (!frame) return 0;
    lldb::SBLineEntry line_entry = static_cast<lldb::SBFrame*>(frame)->GetLineEntry();
    return line_entry.GetLine();
}

const char* lldb_frame_get_file_path(lldb_frame_t frame) {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBLineEntry line_entry = f->GetLineEntry();
    lldb::SBFileSpec file_spec = line_entry.GetFileSpec();

    if (!file_spec.IsValid()) return nullptr;

    char path[4096];
    if (file_spec.GetPath(path, sizeof(path)) > 0) {
        g_temp_string = path;
        return g_temp_string.c_str();
    }

    return nullptr;
}

uint32_t lldb_frame_get_column(lldb_frame_t frame) {
    if (!frame) return 0;
    lldb::SBLineEntry line_entry = static_cast<lldb::SBFrame*>(frame)->GetLineEntry();
    return line_entry.GetColumn();
}

uint64_t lldb_frame_get_pc(lldb_frame_t frame) {
    if (!frame) return 0;
    return static_cast<lldb::SBFrame*>(frame)->GetPC();
}

int lldb_frame_set_pc(lldb_frame_t frame, uint64_t new_pc) {
    if (!frame) return 0;
    return static_cast<lldb::SBFrame*>(frame)->SetPC(new_pc) ? 1 : 0;
}

uint64_t lldb_frame_get_sp(lldb_frame_t frame) {
    if (!frame) return 0;
    return static_cast<lldb::SBFrame*>(frame)->GetSP();
}

uint64_t lldb_frame_get_fp(lldb_frame_t frame) {
    if (!frame) return 0;
    return static_cast<lldb::SBFrame*>(frame)->GetFP();
}

lldb_value_t lldb_frame_find_variable(lldb_frame_t frame, const char* name) {
    if (!frame || !name) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBValue value = f->FindVariable(name);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));
}

lldb_value_t lldb_frame_evaluate_expression(lldb_frame_t frame, const char* expr) {
    if (!frame || !expr) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBValue value = f->EvaluateExpression(expr);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));
}

lldb_value_t lldb_frame_get_value_for_variable_path(lldb_frame_t frame, const char* path) {
    if (!frame || !path) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBValue value = f->GetValueForVariablePath(path);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));
}

uint32_t lldb_frame_get_frame_id(lldb_frame_t frame) {
    if (!frame) return 0;
    return static_cast<lldb::SBFrame*>(frame)->GetFrameID();
}

lldb_thread_t lldb_frame_get_thread(lldb_frame_t frame) {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBThread thread = f->GetThread();

    if (!thread.IsValid()) return nullptr;

    return static_cast<lldb_thread_t>(new lldb::SBThread(thread));
}

lldb_symbol_context_t lldb_frame_get_symbol_context(lldb_frame_t frame, uint32_t scope) {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBSymbolContext ctx = f->GetSymbolContext(scope);

    return static_cast<lldb_symbol_context_t>(new lldb::SBSymbolContext(ctx));
}

lldb_value_list_t lldb_frame_get_variables(lldb_frame_t frame, int arguments, int locals, int statics, int in_scope_only) {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBValueList list = f->GetVariables(arguments != 0, locals != 0, statics != 0, in_scope_only != 0);

    return static_cast<lldb_value_list_t>(new lldb::SBValueList(list));
}

lldb_value_list_t lldb_frame_get_registers(lldb_frame_t frame) {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBValueList list = f->GetRegisters();

    return static_cast<lldb_value_list_t>(new lldb::SBValueList(list));
}

int lldb_frame_is_inlined(lldb_frame_t frame) {
    if (!frame) return 0;
    return static_cast<lldb::SBFrame*>(frame)->IsInlined() ? 1 : 0;
}

const char* lldb_frame_disassemble(lldb_frame_t frame) {
    if (!frame) return nullptr;
    return static_cast<lldb::SBFrame*>(frame)->Disassemble();
}

lldb_module_t lldb_frame_get_module(lldb_frame_t frame) {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBModule module = f->GetModule();

    if (!module.IsValid()) return nullptr;

    return static_cast<lldb_module_t>(new lldb::SBModule(module));
}

// ============================================================================
// SBBreakpoint
// ============================================================================

void lldb_breakpoint_destroy(lldb_breakpoint_t bp) {
    if (bp) {
        delete static_cast<lldb::SBBreakpoint*>(bp);
    }
}

int lldb_breakpoint_is_valid(lldb_breakpoint_t bp) {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->IsValid() ? 1 : 0;
}

int32_t lldb_breakpoint_get_id(lldb_breakpoint_t bp) {
    if (!bp) return -1;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetID();
}

int lldb_breakpoint_is_enabled(lldb_breakpoint_t bp) {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->IsEnabled() ? 1 : 0;
}

void lldb_breakpoint_set_enabled(lldb_breakpoint_t bp, int enabled) {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetEnabled(enabled != 0);
}

int lldb_breakpoint_is_one_shot(lldb_breakpoint_t bp) {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->IsOneShot() ? 1 : 0;
}

void lldb_breakpoint_set_one_shot(lldb_breakpoint_t bp, int one_shot) {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetOneShot(one_shot != 0);
}

uint32_t lldb_breakpoint_get_hit_count(lldb_breakpoint_t bp) {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetHitCount();
}

uint32_t lldb_breakpoint_get_ignore_count(lldb_breakpoint_t bp) {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetIgnoreCount();
}

void lldb_breakpoint_set_ignore_count(lldb_breakpoint_t bp, uint32_t count) {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetIgnoreCount(count);
}

const char* lldb_breakpoint_get_condition(lldb_breakpoint_t bp) {
    if (!bp) return nullptr;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetCondition();
}

void lldb_breakpoint_set_condition(lldb_breakpoint_t bp, const char* condition) {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetCondition(condition);
}

uint32_t lldb_breakpoint_get_num_locations(lldb_breakpoint_t bp) {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetNumLocations();
}

lldb_breakpoint_location_t lldb_breakpoint_get_location_at_index(lldb_breakpoint_t bp, uint32_t index) {
    if (!bp) return nullptr;

    lldb::SBBreakpoint* b = static_cast<lldb::SBBreakpoint*>(bp);
    lldb::SBBreakpointLocation loc = b->GetLocationAtIndex(index);

    if (!loc.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_location_t>(new lldb::SBBreakpointLocation(loc));
}

lldb_breakpoint_location_t lldb_breakpoint_find_location_by_id(lldb_breakpoint_t bp, int32_t id) {
    if (!bp) return nullptr;

    lldb::SBBreakpoint* b = static_cast<lldb::SBBreakpoint*>(bp);
    lldb::SBBreakpointLocation loc = b->FindLocationByID(id);

    if (!loc.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_location_t>(new lldb::SBBreakpointLocation(loc));
}

int lldb_breakpoint_is_hardware(lldb_breakpoint_t bp) {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->IsHardware() ? 1 : 0;
}

int lldb_breakpoint_get_auto_continue(lldb_breakpoint_t bp) {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetAutoContinue() ? 1 : 0;
}

void lldb_breakpoint_set_auto_continue(lldb_breakpoint_t bp, int auto_continue) {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetAutoContinue(auto_continue != 0);
}

uint64_t lldb_breakpoint_get_thread_id(lldb_breakpoint_t bp) {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetThreadID();
}

void lldb_breakpoint_set_thread_id(lldb_breakpoint_t bp, uint64_t tid) {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetThreadID(tid);
}

const char* lldb_breakpoint_get_thread_name(lldb_breakpoint_t bp) {
    if (!bp) return nullptr;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetThreadName();
}

void lldb_breakpoint_set_thread_name(lldb_breakpoint_t bp, const char* name) {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetThreadName(name);
}

uint32_t lldb_breakpoint_get_thread_index(lldb_breakpoint_t bp) {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetThreadIndex();
}

void lldb_breakpoint_set_thread_index(lldb_breakpoint_t bp, uint32_t index) {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetThreadIndex(index);
}

// ============================================================================
// SBBreakpointLocation
// ============================================================================

void lldb_breakpoint_location_destroy(lldb_breakpoint_location_t loc) {
    if (loc) {
        delete static_cast<lldb::SBBreakpointLocation*>(loc);
    }
}

int lldb_breakpoint_location_is_valid(lldb_breakpoint_location_t loc) {
    if (!loc) return 0;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->IsValid() ? 1 : 0;
}

int32_t lldb_breakpoint_location_get_id(lldb_breakpoint_location_t loc) {
    if (!loc) return -1;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->GetID();
}

uint64_t lldb_breakpoint_location_get_load_address(lldb_breakpoint_location_t loc) {
    if (!loc) return LLDB_INVALID_ADDRESS;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->GetLoadAddress();
}

int lldb_breakpoint_location_is_enabled(lldb_breakpoint_location_t loc) {
    if (!loc) return 0;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->IsEnabled() ? 1 : 0;
}

void lldb_breakpoint_location_set_enabled(lldb_breakpoint_location_t loc, int enabled) {
    if (!loc) return;
    static_cast<lldb::SBBreakpointLocation*>(loc)->SetEnabled(enabled != 0);
}

uint32_t lldb_breakpoint_location_get_hit_count(lldb_breakpoint_location_t loc) {
    if (!loc) return 0;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->GetHitCount();
}

uint32_t lldb_breakpoint_location_get_ignore_count(lldb_breakpoint_location_t loc) {
    if (!loc) return 0;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->GetIgnoreCount();
}

void lldb_breakpoint_location_set_ignore_count(lldb_breakpoint_location_t loc, uint32_t count) {
    if (!loc) return;
    static_cast<lldb::SBBreakpointLocation*>(loc)->SetIgnoreCount(count);
}

const char* lldb_breakpoint_location_get_condition(lldb_breakpoint_location_t loc) {
    if (!loc) return nullptr;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->GetCondition();
}

void lldb_breakpoint_location_set_condition(lldb_breakpoint_location_t loc, const char* condition) {
    if (!loc) return;
    static_cast<lldb::SBBreakpointLocation*>(loc)->SetCondition(condition);
}

lldb_breakpoint_t lldb_breakpoint_location_get_breakpoint(lldb_breakpoint_location_t loc) {
    if (!loc) return nullptr;

    lldb::SBBreakpointLocation* l = static_cast<lldb::SBBreakpointLocation*>(loc);
    lldb::SBBreakpoint bp = l->GetBreakpoint();

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));
}

// ============================================================================
// SBValue
// ============================================================================

void lldb_value_destroy(lldb_value_t value) {
    if (value) {
        delete static_cast<lldb::SBValue*>(value);
    }
}

int lldb_value_is_valid(lldb_value_t value) {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->IsValid() ? 1 : 0;
}

const char* lldb_value_get_name(lldb_value_t value) {
    if (!value) return nullptr;
    return static_cast<lldb::SBValue*>(value)->GetName();
}

const char* lldb_value_get_value(lldb_value_t value) {
    if (!value) return nullptr;
    return static_cast<lldb::SBValue*>(value)->GetValue();
}

const char* lldb_value_get_summary(lldb_value_t value) {
    if (!value) return nullptr;
    return static_cast<lldb::SBValue*>(value)->GetSummary();
}

const char* lldb_value_get_type_name(lldb_value_t value) {
    if (!value) return nullptr;
    return static_cast<lldb::SBValue*>(value)->GetTypeName();
}

lldb_type_t lldb_value_get_type(lldb_value_t value) {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBType type = v->GetType();

    if (!type.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(type));
}

uint32_t lldb_value_get_num_children(lldb_value_t value) {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->GetNumChildren();
}

lldb_value_t lldb_value_get_child_at_index(lldb_value_t value, uint32_t index) {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue child = v->GetChildAtIndex(index);

    if (!child.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(child));
}

lldb_value_t lldb_value_get_child_member_with_name(lldb_value_t value, const char* name) {
    if (!value || !name) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue child = v->GetChildMemberWithName(name);

    if (!child.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(child));
}

int64_t lldb_value_get_value_as_signed(lldb_value_t value) {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->GetValueAsSigned();
}

uint64_t lldb_value_get_value_as_unsigned(lldb_value_t value) {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->GetValueAsUnsigned();
}

uint64_t lldb_value_get_byte_size(lldb_value_t value) {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->GetByteSize();
}

int lldb_value_might_have_children(lldb_value_t value) {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->MightHaveChildren() ? 1 : 0;
}

int lldb_value_get_error(lldb_value_t value, lldb_error_t error) {
    if (!value) return 0;
    lldb::SBError err = static_cast<lldb::SBValue*>(value)->GetError();
    if (error) {
        *static_cast<lldb::SBError*>(error) = err;
    }
    return err.Fail() ? 0 : 1;
}

lldb_value_t lldb_value_dereference(lldb_value_t value) {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue deref = v->Dereference();

    if (!deref.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(deref));
}

lldb_value_t lldb_value_address_of(lldb_value_t value) {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue addr = v->AddressOf();

    if (!addr.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(addr));
}

lldb_value_t lldb_value_cast(lldb_value_t value, lldb_type_t type) {
    if (!value || !type) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBValue casted = v->Cast(*t);

    if (!casted.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(casted));
}

uint64_t lldb_value_get_load_address(lldb_value_t value) {
    if (!value) return LLDB_INVALID_ADDRESS;
    return static_cast<lldb::SBValue*>(value)->GetLoadAddress();
}

int lldb_value_get_value_type(lldb_value_t value) {
    if (!value) return LLDB_VALUE_TYPE_INVALID;
    return static_cast<int>(static_cast<lldb::SBValue*>(value)->GetValueType());
}

int lldb_value_set_value_from_cstring(lldb_value_t value, const char* str, lldb_error_t error) {
    if (!value || !str) return 0;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    return v->SetValueFromCString(str, err ? *err : local_error) ? 1 : 0;
}

lldb_value_t lldb_value_create_child_at_offset(lldb_value_t value, const char* name, lldb_type_t type, uint32_t offset) {
    if (!value || !type) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBValue child = v->CreateChildAtOffset(name, offset, *t);

    if (!child.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(child));
}

lldb_value_t lldb_value_create_value_from_address(lldb_value_t value, const char* name, uint64_t addr, lldb_type_t type) {
    if (!value || !type) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBValue created = v->CreateValueFromAddress(name, addr, *t);

    if (!created.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(created));
}

lldb_value_t lldb_value_create_value_from_expression(lldb_value_t value, const char* name, const char* expr) {
    if (!value || !name || !expr) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue created = v->CreateValueFromExpression(name, expr);

    if (!created.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(created));
}

lldb_watchpoint_t lldb_value_watch(lldb_value_t value, int resolve_location, int read, int write, lldb_error_t error) {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    lldb::SBWatchpoint wp = v->Watch(resolve_location != 0, read != 0, write != 0, err ? *err : local_error);

    if (!wp.IsValid()) return nullptr;

    return static_cast<lldb_watchpoint_t>(new lldb::SBWatchpoint(wp));
}

const char* lldb_value_get_expression_path(lldb_value_t value) {
    if (!value) return nullptr;

    static thread_local std::string expr_path;
    lldb::SBStream stream;
    static_cast<lldb::SBValue*>(value)->GetExpressionPath(stream);
    expr_path = stream.GetData();
    return expr_path.c_str();
}

int lldb_value_is_pointer_type(lldb_value_t value) {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->TypeIsPointerType() ? 1 : 0;
}

lldb_value_t lldb_value_get_non_synthetic_value(lldb_value_t value) {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue non_synth = v->GetNonSyntheticValue();

    if (!non_synth.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(non_synth));
}

// ============================================================================
// SBValueList
// ============================================================================

void lldb_value_list_destroy(lldb_value_list_t list) {
    if (list) {
        delete static_cast<lldb::SBValueList*>(list);
    }
}

int lldb_value_list_is_valid(lldb_value_list_t list) {
    if (!list) return 0;
    return static_cast<lldb::SBValueList*>(list)->IsValid() ? 1 : 0;
}

uint32_t lldb_value_list_get_size(lldb_value_list_t list) {
    if (!list) return 0;
    return static_cast<lldb::SBValueList*>(list)->GetSize();
}

lldb_value_t lldb_value_list_get_value_at_index(lldb_value_list_t list, uint32_t index) {
    if (!list) return nullptr;

    lldb::SBValueList* l = static_cast<lldb::SBValueList*>(list);
    lldb::SBValue value = l->GetValueAtIndex(index);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));
}

lldb_value_t lldb_value_list_get_first_value_by_name(lldb_value_list_t list, const char* name) {
    if (!list || !name) return nullptr;

    lldb::SBValueList* l = static_cast<lldb::SBValueList*>(list);
    lldb::SBValue value = l->GetFirstValueByName(name);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));
}

// ============================================================================
// SBError
// ============================================================================

lldb_error_t lldb_error_create(void) {
    return static_cast<lldb_error_t>(new lldb::SBError());
}

void lldb_error_destroy(lldb_error_t error) {
    if (error) {
        delete static_cast<lldb::SBError*>(error);
    }
}

int lldb_error_success(lldb_error_t error) {
    if (!error) return 0;
    return static_cast<lldb::SBError*>(error)->Success() ? 1 : 0;
}

int lldb_error_fail(lldb_error_t error) {
    if (!error) return 1;
    return static_cast<lldb::SBError*>(error)->Fail() ? 1 : 0;
}

const char* lldb_error_get_cstring(lldb_error_t error) {
    if (!error) return nullptr;
    return static_cast<lldb::SBError*>(error)->GetCString();
}

uint32_t lldb_error_get_error(lldb_error_t error) {
    if (!error) return 0;
    return static_cast<lldb::SBError*>(error)->GetError();
}

void lldb_error_clear(lldb_error_t error) {
    if (!error) return;
    static_cast<lldb::SBError*>(error)->Clear();
}

void lldb_error_set_error_string(lldb_error_t error, const char* str) {
    if (!error) return;
    static_cast<lldb::SBError*>(error)->SetErrorString(str);
}

// ============================================================================
// SBModule
// ============================================================================

void lldb_module_destroy(lldb_module_t module) {
    if (module) {
        delete static_cast<lldb::SBModule*>(module);
    }
}

int lldb_module_is_valid(lldb_module_t module) {
    if (!module) return 0;
    return static_cast<lldb::SBModule*>(module)->IsValid() ? 1 : 0;
}

const char* lldb_module_get_file_path(lldb_module_t module) {
    if (!module) return nullptr;

    lldb::SBModule* m = static_cast<lldb::SBModule*>(module);
    lldb::SBFileSpec spec = m->GetFileSpec();

    if (!spec.IsValid()) return nullptr;

    char path[4096];
    if (spec.GetPath(path, sizeof(path)) > 0) {
        g_temp_string = path;
        return g_temp_string.c_str();
    }

    return nullptr;
}

const char* lldb_module_get_platform_file_path(lldb_module_t module) {
    if (!module) return nullptr;

    lldb::SBModule* m = static_cast<lldb::SBModule*>(module);
    lldb::SBFileSpec spec = m->GetPlatformFileSpec();

    if (!spec.IsValid()) return nullptr;

    char path[4096];
    if (spec.GetPath(path, sizeof(path)) > 0) {
        g_temp_string2 = path;
        return g_temp_string2.c_str();
    }

    return nullptr;
}

uint32_t lldb_module_get_num_symbols(lldb_module_t module) {
    if (!module) return 0;
    return static_cast<lldb::SBModule*>(module)->GetNumSymbols();
}

// ============================================================================
// SBSymbolContext
// ============================================================================

void lldb_symbol_context_destroy(lldb_symbol_context_t ctx) {
    if (ctx) {
        delete static_cast<lldb::SBSymbolContext*>(ctx);
    }
}

int lldb_symbol_context_is_valid(lldb_symbol_context_t ctx) {
    if (!ctx) return 0;
    lldb::SBSymbolContext* c = static_cast<lldb::SBSymbolContext*>(ctx);
    return (c->GetModule().IsValid() || c->GetFunction().IsValid()) ? 1 : 0;
}

lldb_module_t lldb_symbol_context_get_module(lldb_symbol_context_t ctx) {
    if (!ctx) return nullptr;

    lldb::SBSymbolContext* c = static_cast<lldb::SBSymbolContext*>(ctx);
    lldb::SBModule module = c->GetModule();

    if (!module.IsValid()) return nullptr;

    return static_cast<lldb_module_t>(new lldb::SBModule(module));
}

const char* lldb_symbol_context_get_function_name(lldb_symbol_context_t ctx) {
    if (!ctx) return nullptr;

    lldb::SBSymbolContext* c = static_cast<lldb::SBSymbolContext*>(ctx);
    lldb::SBFunction func = c->GetFunction();

    if (func.IsValid()) {
        return func.GetName();
    }

    lldb::SBSymbol sym = c->GetSymbol();
    if (sym.IsValid()) {
        return sym.GetName();
    }

    return nullptr;
}

// ============================================================================
// SBType
// ============================================================================

void lldb_type_destroy(lldb_type_t type) {
    if (type) {
        delete static_cast<lldb::SBType*>(type);
    }
}

int lldb_type_is_valid(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsValid() ? 1 : 0;
}

const char* lldb_type_get_name(lldb_type_t type) {
    if (!type) return nullptr;
    return static_cast<lldb::SBType*>(type)->GetName();
}

const char* lldb_type_get_display_type_name(lldb_type_t type) {
    if (!type) return nullptr;
    return static_cast<lldb::SBType*>(type)->GetDisplayTypeName();
}

uint64_t lldb_type_get_byte_size(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->GetByteSize();
}

int lldb_type_is_pointer_type(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsPointerType() ? 1 : 0;
}

int lldb_type_is_reference_type(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsReferenceType() ? 1 : 0;
}

int lldb_type_is_array_type(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsArrayType() ? 1 : 0;
}

int lldb_type_is_vector_type(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsVectorType() ? 1 : 0;
}

int lldb_type_is_typedef_type(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsTypedefType() ? 1 : 0;
}

int lldb_type_is_function_type(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsFunctionType() ? 1 : 0;
}

int lldb_type_is_polymorphic_class(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsPolymorphicClass() ? 1 : 0;
}

lldb_type_t lldb_type_get_pointer_type(lldb_type_t type) {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType ptr = t->GetPointerType();

    if (!ptr.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(ptr));
}

lldb_type_t lldb_type_get_pointee_type(lldb_type_t type) {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType pointee = t->GetPointeeType();

    if (!pointee.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(pointee));
}

lldb_type_t lldb_type_get_reference_type(lldb_type_t type) {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType ref = t->GetReferenceType();

    if (!ref.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(ref));
}

lldb_type_t lldb_type_get_dereferenced_type(lldb_type_t type) {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType deref = t->GetDereferencedType();

    if (!deref.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(deref));
}

lldb_type_t lldb_type_get_unqualified_type(lldb_type_t type) {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType unqual = t->GetUnqualifiedType();

    if (!unqual.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(unqual));
}

lldb_type_t lldb_type_get_canonical_type(lldb_type_t type) {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType canon = t->GetCanonicalType();

    if (!canon.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(canon));
}

lldb_type_t lldb_type_get_array_element_type(lldb_type_t type) {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType elem = t->GetArrayElementType();

    if (!elem.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(elem));
}

uint64_t lldb_type_get_array_size(lldb_type_t type) {
    if (!type) return 0;
    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    // GetArraySize may not be available in older LLDB versions
    // Use byte_size / element_type_byte_size as a fallback
    if (!t->IsArrayType()) return 0;
    lldb::SBType elem = t->GetArrayElementType();
    if (!elem.IsValid() || elem.GetByteSize() == 0) return 0;
    return t->GetByteSize() / elem.GetByteSize();
}

uint32_t lldb_type_get_num_fields(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->GetNumberOfFields();
}

uint32_t lldb_type_get_num_direct_base_classes(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->GetNumberOfDirectBaseClasses();
}

uint32_t lldb_type_get_num_virtual_base_classes(lldb_type_t type) {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->GetNumberOfVirtualBaseClasses();
}

int lldb_type_get_basic_type(lldb_type_t type) {
    if (!type) return LLDB_BASIC_TYPE_INVALID;
    return static_cast<int>(static_cast<lldb::SBType*>(type)->GetBasicType());
}

// ============================================================================
// SBWatchpoint
// ============================================================================

void lldb_watchpoint_destroy(lldb_watchpoint_t wp) {
    if (wp) {
        delete static_cast<lldb::SBWatchpoint*>(wp);
    }
}

int lldb_watchpoint_is_valid(lldb_watchpoint_t wp) {
    if (!wp) return 0;
    return static_cast<lldb::SBWatchpoint*>(wp)->IsValid() ? 1 : 0;
}

int32_t lldb_watchpoint_get_id(lldb_watchpoint_t wp) {
    if (!wp) return -1;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetID();
}

int lldb_watchpoint_is_enabled(lldb_watchpoint_t wp) {
    if (!wp) return 0;
    return static_cast<lldb::SBWatchpoint*>(wp)->IsEnabled() ? 1 : 0;
}

void lldb_watchpoint_set_enabled(lldb_watchpoint_t wp, int enabled) {
    if (!wp) return;
    static_cast<lldb::SBWatchpoint*>(wp)->SetEnabled(enabled != 0);
}

uint32_t lldb_watchpoint_get_hit_count(lldb_watchpoint_t wp) {
    if (!wp) return 0;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetHitCount();
}

uint32_t lldb_watchpoint_get_ignore_count(lldb_watchpoint_t wp) {
    if (!wp) return 0;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetIgnoreCount();
}

void lldb_watchpoint_set_ignore_count(lldb_watchpoint_t wp, uint32_t count) {
    if (!wp) return;
    static_cast<lldb::SBWatchpoint*>(wp)->SetIgnoreCount(count);
}

const char* lldb_watchpoint_get_condition(lldb_watchpoint_t wp) {
    if (!wp) return nullptr;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetCondition();
}

void lldb_watchpoint_set_condition(lldb_watchpoint_t wp, const char* condition) {
    if (!wp) return;
    static_cast<lldb::SBWatchpoint*>(wp)->SetCondition(condition);
}

uint64_t lldb_watchpoint_get_watch_address(lldb_watchpoint_t wp) {
    if (!wp) return 0;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetWatchAddress();
}

size_t lldb_watchpoint_get_watch_size(lldb_watchpoint_t wp) {
    if (!wp) return 0;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetWatchSize();
}

int lldb_watchpoint_is_watching_reads(lldb_watchpoint_t wp) {
    if (!wp) return 0;
#if LLDB_VERSION_MAJOR >= 15
    return static_cast<lldb::SBWatchpoint*>(wp)->IsWatchingReads() ? 1 : 0;
#else
    // IsWatchingReads() not available in LLDB < 15
    // Return -1 to indicate the method is not available
    return -1;
#endif
}

int lldb_watchpoint_is_watching_writes(lldb_watchpoint_t wp) {
    if (!wp) return 0;
#if LLDB_VERSION_MAJOR >= 15
    return static_cast<lldb::SBWatchpoint*>(wp)->IsWatchingWrites() ? 1 : 0;
#else
    // IsWatchingWrites() not available in LLDB < 15
    // Return -1 to indicate the method is not available
    return -1;
#endif
}

// ============================================================================
// SBCommandInterpreter
// ============================================================================

void lldb_command_interpreter_destroy(lldb_command_interpreter_t interp) {
    if (interp) {
        delete static_cast<lldb::SBCommandInterpreter*>(interp);
    }
}

int lldb_command_interpreter_is_valid(lldb_command_interpreter_t interp) {
    if (!interp) return 0;
    return static_cast<lldb::SBCommandInterpreter*>(interp)->IsValid() ? 1 : 0;
}

int lldb_command_interpreter_handle_command(lldb_command_interpreter_t interp,
                                             const char* command,
                                             lldb_command_return_object_t result,
                                             int add_to_history) {
    if (!interp || !command) return 0;

    lldb::SBCommandInterpreter* i = static_cast<lldb::SBCommandInterpreter*>(interp);
    lldb::SBCommandReturnObject* r = result ? static_cast<lldb::SBCommandReturnObject*>(result) : nullptr;
    lldb::SBCommandReturnObject local_result;

    lldb::ReturnStatus status = i->HandleCommand(command, r ? *r : local_result, add_to_history != 0);
    return status == lldb::eReturnStatusSuccessFinishResult ||
           status == lldb::eReturnStatusSuccessFinishNoResult ? 1 : 0;
}

int lldb_command_interpreter_command_exists(lldb_command_interpreter_t interp, const char* command) {
    if (!interp || !command) return 0;
    return static_cast<lldb::SBCommandInterpreter*>(interp)->CommandExists(command) ? 1 : 0;
}

int lldb_command_interpreter_alias_exists(lldb_command_interpreter_t interp, const char* alias) {
    if (!interp || !alias) return 0;
    return static_cast<lldb::SBCommandInterpreter*>(interp)->AliasExists(alias) ? 1 : 0;
}

// ============================================================================
// SBCommandReturnObject
// ============================================================================

lldb_command_return_object_t lldb_command_return_object_create(void) {
    return static_cast<lldb_command_return_object_t>(new lldb::SBCommandReturnObject());
}

void lldb_command_return_object_destroy(lldb_command_return_object_t obj) {
    if (obj) {
        delete static_cast<lldb::SBCommandReturnObject*>(obj);
    }
}

int lldb_command_return_object_is_valid(lldb_command_return_object_t obj) {
    if (!obj) return 0;
    return static_cast<lldb::SBCommandReturnObject*>(obj)->IsValid() ? 1 : 0;
}

const char* lldb_command_return_object_get_output(lldb_command_return_object_t obj) {
    if (!obj) return nullptr;
    return static_cast<lldb::SBCommandReturnObject*>(obj)->GetOutput();
}

const char* lldb_command_return_object_get_error(lldb_command_return_object_t obj) {
    if (!obj) return nullptr;
    return static_cast<lldb::SBCommandReturnObject*>(obj)->GetError();
}

int lldb_command_return_object_succeeded(lldb_command_return_object_t obj) {
    if (!obj) return 0;
    return static_cast<lldb::SBCommandReturnObject*>(obj)->Succeeded() ? 1 : 0;
}

void lldb_command_return_object_clear(lldb_command_return_object_t obj) {
    if (!obj) return;
    static_cast<lldb::SBCommandReturnObject*>(obj)->Clear();
}

} // extern "C"
