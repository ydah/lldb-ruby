#include "lldb_wrapper.h"
#include "lldb_wrapper_config.h"
#include <lldb/API/LLDB.h>

#include <algorithm>
#include <string>
#include <cstring>
#include <exception>
#include <new>

// Thread-local storage for temporary strings
static thread_local std::string g_temp_string;
static thread_local std::string g_temp_string2;
static thread_local int g_last_error_code = 0;
static thread_local char g_last_error_message[1024] = {0};

static void wrapper_clear_error_state() noexcept {
    g_last_error_code = 0;
    g_last_error_message[0] = '\0';
}

static void wrapper_set_error_state(const char* message) noexcept {
    g_last_error_code = LLDB_RUBY_STATUS_INTERNAL_ERROR;
    if (!message) message = "unknown native exception";
    std::strncpy(g_last_error_message, message, sizeof(g_last_error_message) - 1);
    g_last_error_message[sizeof(g_last_error_message) - 1] = '\0';
}

static void wrapper_set_invalid_argument(lldb::SBError* error, const char* message) noexcept {
    if (error) error->SetErrorString(message);
}

static void wrapper_copy_error(lldb_error_t output, const lldb::SBError& error) {
    if (output) *static_cast<lldb::SBError*>(output) = error;
}

static lldb_ruby_status_t wrapper_status(lldb::SBError& error, lldb_error_t output) {
    wrapper_copy_error(output, error);
    return error.Success() ? LLDB_RUBY_STATUS_OK : LLDB_RUBY_STATUS_LLDB_ERROR;
}

static bool wrapper_copy_file_spec_path(const lldb::SBFileSpec& file_spec,
                                        std::string& output) {
    size_t capacity = 256;
    for (;;) {
        output.resize(capacity);
        uint32_t written = file_spec.GetPath(output.data(), output.size());
        if (written < output.size() - 1) {
            output.resize(written);
            return written > 0;
        }

        capacity = std::max(capacity * 2, static_cast<size_t>(written) + 2);
    }
}

static_assert(LLDB_INVALID_ADDRESS == UINT64_MAX, "unexpected LLDB invalid address sentinel");
static_assert(LLDB_INVALID_PROCESS_ID == 0, "unexpected LLDB invalid process sentinel");
static_assert(LLDB_INVALID_THREAD_ID == 0, "unexpected LLDB invalid thread sentinel");
static_assert(LLDB_INVALID_BREAK_ID == 0, "unexpected LLDB invalid breakpoint sentinel");
static_assert(LLDB_INVALID_LINE_NUMBER == UINT32_MAX, "unexpected LLDB invalid line sentinel");

extern "C" {

// ============================================================================
// Wrapper metadata and capabilities
// ============================================================================

uint32_t lldb_wrapper_abi_version(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return LLDB_RUBY_WRAPPER_ABI_VERSION;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_wrapper_build_lldb_version(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return LLDB_RUBY_BUILD_LLDB_VERSION;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_wrapper_runtime_lldb_version(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return lldb::SBDebugger::GetVersionString();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_wrapper_has_capability(uint32_t capability)  LLDB_WRAPPER_NOEXCEPT {
    try {
    switch (capability) {
        case LLDB_RUBY_CAPABILITY_WATCHPOINT_ACCESS_KIND:
            return LLDB_RUBY_HAVE_WATCHPOINT_ACCESS_KIND;
        default:
            return 0;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_wrapper_last_error_message(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return g_last_error_message;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_wrapper_last_error_code(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return g_last_error_code;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_wrapper_clear_last_error(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

// ============================================================================
// Initialization
// ============================================================================

lldb_ruby_status_t lldb_initialize(lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    try {
        lldb::SBError init_error = lldb::SBDebugger::InitializeWithErrorHandling();
        return wrapper_status(init_error, error);
    } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed during initialization");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception during initialization");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

void lldb_terminate(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    lldb::SBDebugger::Terminate();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

// ============================================================================
// SBDebugger
// ============================================================================

lldb_debugger_t lldb_debugger_create(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return lldb_debugger_create_with_source_init_files(0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_debugger_t lldb_debugger_create_with_source_init_files(int source_init_files)  LLDB_WRAPPER_NOEXCEPT {
    try {
    lldb::SBDebugger* dbg = new lldb::SBDebugger(
        lldb::SBDebugger::Create(source_init_files != 0));
    return static_cast<lldb_debugger_t>(dbg);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_debugger_destroy(lldb_debugger_t dbg)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (dbg) {
        lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
        if (debugger->IsValid()) {
            lldb::SBDebugger::Destroy(*debugger);
        }
        delete debugger;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_debugger_is_valid(lldb_debugger_t dbg)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg) return 0;
    return static_cast<lldb::SBDebugger*>(dbg)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_target_t lldb_debugger_create_target(lldb_debugger_t dbg,
                                           const char* filename,
                                           const char* arch,
                                           const char* platform,
                                           int add_dependent_modules,
                                           lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
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

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_target_t lldb_debugger_create_target_simple(lldb_debugger_t dbg,
                                                  const char* filename)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg) return nullptr;

    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget target = debugger->CreateTarget(filename);

    if (!target.IsValid()) return nullptr;

    return static_cast<lldb_target_t>(new lldb::SBTarget(target));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_debugger_get_num_targets(lldb_debugger_t dbg)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg) return 0;
    return static_cast<lldb::SBDebugger*>(dbg)->GetNumTargets();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_target_t lldb_debugger_get_target_at_index(lldb_debugger_t dbg, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg) return nullptr;

    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget target = debugger->GetTargetAtIndex(index);

    if (!target.IsValid()) return nullptr;

    return static_cast<lldb_target_t>(new lldb::SBTarget(target));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_target_t lldb_debugger_get_selected_target(lldb_debugger_t dbg)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg) return nullptr;

    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget target = debugger->GetSelectedTarget();

    if (!target.IsValid()) return nullptr;

    return static_cast<lldb_target_t>(new lldb::SBTarget(target));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_debugger_set_async(lldb_debugger_t dbg, int async)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg) return;
    static_cast<lldb::SBDebugger*>(dbg)->SetAsync(async != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_debugger_get_async(lldb_debugger_t dbg)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg) return 0;
    return static_cast<lldb::SBDebugger*>(dbg)->GetAsync() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_debugger_set_selected_target(lldb_debugger_t dbg, lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg || !target) return;
    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    debugger->SetSelectedTarget(*t);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_debugger_delete_target(lldb_debugger_t dbg, lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg || !target) return 0;
    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    return debugger->DeleteTarget(*t) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_target_t lldb_debugger_find_target_with_process_id(lldb_debugger_t dbg, uint64_t pid)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg) return nullptr;
    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBTarget target = debugger->FindTargetWithProcessID(pid);
    if (!target.IsValid()) return nullptr;
    return static_cast<lldb_target_t>(new lldb::SBTarget(target));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_debugger_get_version_string(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return lldb::SBDebugger::GetVersionString();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_command_interpreter_t lldb_debugger_get_command_interpreter(lldb_debugger_t dbg)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg) return nullptr;
    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    lldb::SBCommandInterpreter interp = debugger->GetCommandInterpreter();
    if (!interp.IsValid()) return nullptr;
    return static_cast<lldb_command_interpreter_t>(new lldb::SBCommandInterpreter(interp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_debugger_handle_command(lldb_debugger_t dbg, const char* command)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg || !command) return;
    lldb::SBDebugger* debugger = static_cast<lldb::SBDebugger*>(dbg);
    debugger->HandleCommand(command);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

lldb_broadcaster_t lldb_debugger_get_broadcaster(lldb_debugger_t dbg)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg) return nullptr;
    lldb::SBBroadcaster broadcaster = static_cast<lldb::SBDebugger*>(dbg)->GetBroadcaster();
    if (!broadcaster.IsValid()) return nullptr;
    return static_cast<lldb_broadcaster_t>(new lldb::SBBroadcaster(broadcaster));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_listener_t lldb_debugger_get_listener(lldb_debugger_t dbg)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!dbg) return nullptr;
    lldb::SBListener listener = static_cast<lldb::SBDebugger*>(dbg)->GetListener();
    if (!listener.IsValid()) return nullptr;
    return static_cast<lldb_listener_t>(new lldb::SBListener(listener));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBBroadcaster, SBListener, and SBEvent
// ============================================================================

lldb_broadcaster_t lldb_broadcaster_create(const char* name)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return static_cast<lldb_broadcaster_t>(new lldb::SBBroadcaster(name));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_broadcaster_destroy(lldb_broadcaster_t broadcaster)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (broadcaster) delete static_cast<lldb::SBBroadcaster*>(broadcaster);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_broadcaster_is_valid(lldb_broadcaster_t broadcaster)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return broadcaster && static_cast<lldb::SBBroadcaster*>(broadcaster)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_broadcaster_get_name(lldb_broadcaster_t broadcaster)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!broadcaster) return nullptr;
    return static_cast<lldb::SBBroadcaster*>(broadcaster)->GetName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_broadcaster_add_listener(lldb_broadcaster_t broadcaster,
                                        lldb_listener_t listener,
                                        uint32_t event_mask)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!broadcaster || !listener) return 0;
    return static_cast<lldb::SBBroadcaster*>(broadcaster)->AddListener(
        *static_cast<lldb::SBListener*>(listener), event_mask);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_broadcaster_remove_listener(lldb_broadcaster_t broadcaster,
                                     lldb_listener_t listener,
                                     uint32_t event_mask)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!broadcaster || !listener) return 0;
    return static_cast<lldb::SBBroadcaster*>(broadcaster)->RemoveListener(
               *static_cast<lldb::SBListener*>(listener), event_mask)
               ? 1
               : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_broadcaster_event_type_has_listeners(lldb_broadcaster_t broadcaster,
                                              uint32_t event_type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!broadcaster) return 0;
    return static_cast<lldb::SBBroadcaster*>(broadcaster)->EventTypeHasListeners(event_type) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_broadcaster_broadcast_event_by_type(lldb_broadcaster_t broadcaster,
                                               uint32_t event_type,
                                               int unique)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!broadcaster) return;
    static_cast<lldb::SBBroadcaster*>(broadcaster)->BroadcastEventByType(event_type, unique != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

lldb_listener_t lldb_listener_create(const char* name)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return static_cast<lldb_listener_t>(new lldb::SBListener(name));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_listener_destroy(lldb_listener_t listener)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (listener) delete static_cast<lldb::SBListener*>(listener);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_listener_is_valid(lldb_listener_t listener)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return listener && static_cast<lldb::SBListener*>(listener)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_listener_start_listening_for_events(lldb_listener_t listener,
                                                  lldb_broadcaster_t broadcaster,
                                                  uint32_t event_mask)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!listener || !broadcaster) return 0;
    return static_cast<lldb::SBListener*>(listener)->StartListeningForEvents(
        *static_cast<lldb::SBBroadcaster*>(broadcaster), event_mask);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_listener_stop_listening_for_events(lldb_listener_t listener,
                                            lldb_broadcaster_t broadcaster,
                                            uint32_t event_mask)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!listener || !broadcaster) return 0;
    return static_cast<lldb::SBListener*>(listener)->StopListeningForEvents(
               *static_cast<lldb::SBBroadcaster*>(broadcaster), event_mask)
               ? 1
               : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_event_t lldb_listener_wait_for_event(lldb_listener_t listener, uint32_t timeout_seconds)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!listener) return nullptr;
    lldb::SBEvent event;
    if (!static_cast<lldb::SBListener*>(listener)->WaitForEvent(timeout_seconds, event)) return nullptr;
    return static_cast<lldb_event_t>(new lldb::SBEvent(event));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_event_t lldb_listener_peek_event(lldb_listener_t listener)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!listener) return nullptr;
    lldb::SBEvent event;
    if (!static_cast<lldb::SBListener*>(listener)->PeekAtNextEvent(event)) return nullptr;
    return static_cast<lldb_event_t>(new lldb::SBEvent(event));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_event_t lldb_listener_next_event(lldb_listener_t listener)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!listener) return nullptr;
    lldb::SBEvent event;
    if (!static_cast<lldb::SBListener*>(listener)->GetNextEvent(event)) return nullptr;
    return static_cast<lldb_event_t>(new lldb::SBEvent(event));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_event_destroy(lldb_event_t event)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (event) delete static_cast<lldb::SBEvent*>(event);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_event_is_valid(lldb_event_t event)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return event && static_cast<lldb::SBEvent*>(event)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_event_get_type(lldb_event_t event)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!event) return 0;
    return static_cast<lldb::SBEvent*>(event)->GetType();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_event_get_data_flavor(lldb_event_t event)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!event) return nullptr;
    return static_cast<lldb::SBEvent*>(event)->GetDataFlavor();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_event_get_broadcaster_class(lldb_event_t event)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!event) return nullptr;
    return static_cast<lldb::SBEvent*>(event)->GetBroadcasterClass();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_event_get_description(lldb_event_t event, char* buffer, size_t length)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!event) return 0;
    lldb::SBStream stream;
    if (!static_cast<lldb::SBEvent*>(event)->GetDescription(stream)) return 0;
    const char* description = stream.GetData();
    if (!description) return 0;

    size_t required = std::strlen(description);
    if (!buffer || length == 0) return static_cast<uint32_t>(required);

    size_t copied = std::min(required, length - 1);
    std::memcpy(buffer, description, copied);
    buffer[copied] = '\0';
    return static_cast<uint32_t>(copied);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_broadcaster_t lldb_event_get_broadcaster(lldb_event_t event)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!event) return nullptr;
    lldb::SBBroadcaster broadcaster = static_cast<lldb::SBEvent*>(event)->GetBroadcaster();
    if (!broadcaster.IsValid()) return nullptr;
    return static_cast<lldb_broadcaster_t>(new lldb::SBBroadcaster(broadcaster));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_event_is_process_event(lldb_event_t event)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!event) return 0;
    return lldb::SBProcess::EventIsProcessEvent(*static_cast<lldb::SBEvent*>(event)) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_event_get_process_state(lldb_event_t event)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!event) return 0;
    return static_cast<int>(lldb::SBProcess::GetStateFromEvent(*static_cast<lldb::SBEvent*>(event)));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_event_get_restarted(lldb_event_t event)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!event) return 0;
    return lldb::SBProcess::GetRestartedFromEvent(*static_cast<lldb::SBEvent*>(event)) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_event_get_interrupted(lldb_event_t event)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!event) return 0;
    return lldb::SBProcess::GetInterruptedFromEvent(*static_cast<lldb::SBEvent*>(event)) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_process_t lldb_event_get_process(lldb_event_t event)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!event) return nullptr;
    lldb::SBProcess process = lldb::SBProcess::GetProcessFromEvent(
        *static_cast<lldb::SBEvent*>(event));
    if (!process.IsValid()) return nullptr;
    return static_cast<lldb_process_t>(new lldb::SBProcess(process));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBTarget
// ============================================================================

void lldb_target_destroy(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (target) {
        delete static_cast<lldb::SBTarget*>(target);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_target_is_valid(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_process_t lldb_target_launch_simple(lldb_target_t target,
                                          const char** argv,
                                          const char** envp,
                                          const char* working_dir)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBProcess process = t->LaunchSimple(argv, envp, working_dir);

    if (!process.IsValid()) return nullptr;

    return static_cast<lldb_process_t>(new lldb::SBProcess(process));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_process_t lldb_target_launch(lldb_target_t target,
                                   lldb_launch_info_t launch_info,
                                   lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target || !launch_info) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBLaunchInfo* info = static_cast<lldb::SBLaunchInfo*>(launch_info);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    lldb::SBProcess process = t->Launch(*info, err ? *err : local_error);

    if (!process.IsValid()) return nullptr;

    return static_cast<lldb_process_t>(new lldb::SBProcess(process));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_process_t lldb_target_attach_to_process_with_id(lldb_target_t target,
                                                      uint64_t pid,
                                                      lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
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

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_process_t lldb_target_attach_to_process_with_name(lldb_target_t target,
                                                        const char* name,
                                                        int wait_for,
                                                        lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
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

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_process_t lldb_target_attach_with_info(lldb_target_t target,
                                             lldb_attach_info_t attach_info,
                                             lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target || !attach_info) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBAttachInfo* info = static_cast<lldb::SBAttachInfo*>(attach_info);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;
    lldb::SBProcess process = t->Attach(*info, err ? *err : local_error);

    if (!process.IsValid()) return nullptr;
    return static_cast<lldb_process_t>(new lldb::SBProcess(process));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_breakpoint_t lldb_target_breakpoint_create_by_name(lldb_target_t target,
                                                         const char* symbol_name,
                                                         const char* module_name)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target || !symbol_name) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->BreakpointCreateByName(symbol_name, module_name);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_breakpoint_t lldb_target_breakpoint_create_by_location(lldb_target_t target,
                                                             const char* file,
                                                             uint32_t line)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target || !file) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->BreakpointCreateByLocation(file, line);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_breakpoint_t lldb_target_breakpoint_create_by_address(lldb_target_t target,
                                                            uint64_t address)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->BreakpointCreateByAddress(address);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_breakpoint_t lldb_target_breakpoint_create_by_regex(lldb_target_t target,
                                                          const char* symbol_regex,
                                                          const char* module_name)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target || !symbol_regex) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->BreakpointCreateByRegex(symbol_regex, module_name);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_breakpoint_t lldb_target_breakpoint_create_by_source_regex(lldb_target_t target,
                                                                 const char* source_regex,
                                                                 const char* source_file)  LLDB_WRAPPER_NOEXCEPT {
    try {
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

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_target_delete_breakpoint(lldb_target_t target, int32_t breakpoint_id)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->BreakpointDelete(breakpoint_id) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_target_delete_all_breakpoints(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->DeleteAllBreakpoints() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_target_enable_all_breakpoints(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->EnableAllBreakpoints() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_target_disable_all_breakpoints(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->DisableAllBreakpoints() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_breakpoint_t lldb_target_find_breakpoint_by_id(lldb_target_t target, int32_t id)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->FindBreakpointByID(id);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_target_get_num_breakpoints(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->GetNumBreakpoints();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_breakpoint_t lldb_target_get_breakpoint_at_index(lldb_target_t target, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBBreakpoint bp = t->GetBreakpointAtIndex(index);

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_process_t lldb_target_get_process(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBProcess process = t->GetProcess();

    if (!process.IsValid()) return nullptr;

    return static_cast<lldb_process_t>(new lldb::SBProcess(process));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_target_get_executable_path(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBFileSpec spec = t->GetExecutable();

    if (!spec.IsValid()) return nullptr;

    if (wrapper_copy_file_spec_path(spec, g_temp_string)) {
        return g_temp_string.c_str();
    }

    return nullptr;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_file_spec_t lldb_target_get_executable_file(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;

    lldb::SBFileSpec spec = static_cast<lldb::SBTarget*>(target)->GetExecutable();
    if (!spec.IsValid()) return nullptr;

    return static_cast<lldb_file_spec_t>(new lldb::SBFileSpec(spec));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_target_get_num_modules(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->GetNumModules();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_module_t lldb_target_get_module_at_index(lldb_target_t target, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBModule module = t->GetModuleAtIndex(index);

    if (!module.IsValid()) return nullptr;

    return static_cast<lldb_module_t>(new lldb::SBModule(module));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_target_evaluate_expression(lldb_target_t target, const char* expr)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target || !expr) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBValue value = t->EvaluateExpression(expr);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_target_evaluate_expression_with_options(lldb_target_t target,
                                                          const char* expr,
                                                          lldb_expression_options_t options)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target || !expr || !options) return nullptr;

    lldb::SBValue value = static_cast<lldb::SBTarget*>(target)->EvaluateExpression(
        expr, *static_cast<lldb::SBExpressionOptions*>(options));
    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

size_t lldb_target_read_memory(lldb_target_t target, uint64_t addr, void* buf, size_t size, lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target || !buf) return 0;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;
    lldb::SBAddress sb_addr(addr, *t);

    return t->ReadMemory(sb_addr, buf, size, err ? *err : local_error);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_target_get_address_byte_size(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->GetAddressByteSize();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_target_get_triple(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;
    return static_cast<lldb::SBTarget*>(target)->GetTriple();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_watchpoint_t lldb_target_watch_address(lldb_target_t target, uint64_t addr, size_t size, int read, int write, lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    lldb::SBWatchpoint wp = t->WatchAddress(addr, size, read != 0, write != 0, err ? *err : local_error);

    if (!wp.IsValid()) return nullptr;

    return static_cast<lldb_watchpoint_t>(new lldb::SBWatchpoint(wp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_target_delete_watchpoint(lldb_target_t target, int32_t watchpoint_id)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->DeleteWatchpoint(watchpoint_id) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_target_delete_all_watchpoints(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->DeleteAllWatchpoints() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_watchpoint_t lldb_target_find_watchpoint_by_id(lldb_target_t target, int32_t id)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBWatchpoint wp = t->FindWatchpointByID(id);

    if (!wp.IsValid()) return nullptr;

    return static_cast<lldb_watchpoint_t>(new lldb::SBWatchpoint(wp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_target_get_num_watchpoints(lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return 0;
    return static_cast<lldb::SBTarget*>(target)->GetNumWatchpoints();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_watchpoint_t lldb_target_get_watchpoint_at_index(lldb_target_t target, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;

    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBWatchpoint wp = t->GetWatchpointAtIndex(index);

    if (!wp.IsValid()) return nullptr;

    return static_cast<lldb_watchpoint_t>(new lldb::SBWatchpoint(wp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBLaunchInfo
// ============================================================================

lldb_launch_info_t lldb_launch_info_create(const char** argv)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return static_cast<lldb_launch_info_t>(new lldb::SBLaunchInfo(argv));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_launch_info_destroy(lldb_launch_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (info) {
        delete static_cast<lldb::SBLaunchInfo*>(info);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint32_t lldb_launch_info_get_num_arguments(lldb_launch_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return info ? static_cast<lldb::SBLaunchInfo*>(info)->GetNumArguments() : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_launch_info_get_argument_at_index(lldb_launch_info_t info, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return nullptr;
    return static_cast<lldb::SBLaunchInfo*>(info)->GetArgumentAtIndex(index);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_launch_info_set_working_directory(lldb_launch_info_t info, const char* dir)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return;
    static_cast<lldb::SBLaunchInfo*>(info)->SetWorkingDirectory(dir);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

const char* lldb_launch_info_get_working_directory(lldb_launch_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return nullptr;
    return static_cast<lldb::SBLaunchInfo*>(info)->GetWorkingDirectory();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_launch_info_set_environment_entries(lldb_launch_info_t info,
                                               const char** envp,
                                               int append)  LLDB_WRAPPER_NOEXCEPT {
    try {
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

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint32_t lldb_launch_info_get_num_environment_entries(lldb_launch_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return info ? static_cast<lldb::SBLaunchInfo*>(info)->GetNumEnvironmentEntries() : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_launch_info_get_environment_entry_at_index(lldb_launch_info_t info,
                                                             uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return nullptr;
    return static_cast<lldb::SBLaunchInfo*>(info)->GetEnvironmentEntryAtIndex(index);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_launch_info_get_launch_flags(lldb_launch_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return 0;
    return static_cast<lldb::SBLaunchInfo*>(info)->GetLaunchFlags();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_launch_info_set_launch_flags(lldb_launch_info_t info, uint32_t flags)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return;
    static_cast<lldb::SBLaunchInfo*>(info)->SetLaunchFlags(flags);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

void lldb_launch_info_set_arguments(lldb_launch_info_t info, const char** argv, int append)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return;
    static_cast<lldb::SBLaunchInfo*>(info)->SetArguments(argv, append != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

lldb_file_spec_t lldb_launch_info_get_executable_file(lldb_launch_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return nullptr;
    lldb::SBFileSpec file = static_cast<lldb::SBLaunchInfo*>(info)->GetExecutableFile();
    if (!file.IsValid()) return nullptr;
    return static_cast<lldb_file_spec_t>(new lldb::SBFileSpec(file));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_launch_info_set_executable_file(lldb_launch_info_t info,
                                           lldb_file_spec_t file_spec,
                                           int add_as_first_arg)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info || !file_spec) return;
    static_cast<lldb::SBLaunchInfo*>(info)->SetExecutableFile(
        *static_cast<lldb::SBFileSpec*>(file_spec), add_as_first_arg != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

lldb_listener_t lldb_launch_info_get_listener(lldb_launch_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return nullptr;
    lldb::SBListener listener = static_cast<lldb::SBLaunchInfo*>(info)->GetListener();
    if (!listener.IsValid()) return nullptr;
    return static_cast<lldb_listener_t>(new lldb::SBListener(listener));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_launch_info_set_listener(lldb_launch_info_t info, lldb_listener_t listener)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info || !listener) return;
    static_cast<lldb::SBLaunchInfo*>(info)->SetListener(
        *static_cast<lldb::SBListener*>(listener));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

const char* lldb_launch_info_get_process_plugin_name(lldb_launch_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return nullptr;
    return static_cast<lldb::SBLaunchInfo*>(info)->GetProcessPluginName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_launch_info_set_process_plugin_name(lldb_launch_info_t info, const char* name)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info || !name) return;
    static_cast<lldb::SBLaunchInfo*>(info)->SetProcessPluginName(name);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

const char* lldb_launch_info_get_shell(lldb_launch_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return nullptr;
    return static_cast<lldb::SBLaunchInfo*>(info)->GetShell();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_launch_info_set_shell(lldb_launch_info_t info, const char* path)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info || !path) return;
    static_cast<lldb::SBLaunchInfo*>(info)->SetShell(path);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_launch_info_add_close_file_action(lldb_launch_info_t info, int fd)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return info && static_cast<lldb::SBLaunchInfo*>(info)->AddCloseFileAction(fd) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_launch_info_add_duplicate_file_action(lldb_launch_info_t info, int fd, int dup_fd)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return info && static_cast<lldb::SBLaunchInfo*>(info)->AddDuplicateFileAction(fd, dup_fd) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_launch_info_add_open_file_action(lldb_launch_info_t info,
                                          int fd,
                                          const char* path,
                                          int read,
                                          int write)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return info && path && static_cast<lldb::SBLaunchInfo*>(info)->AddOpenFileAction(
                               fd, path, read != 0, write != 0)
               ? 1
               : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_launch_info_add_suppress_file_action(lldb_launch_info_t info,
                                              int fd,
                                              int read,
                                              int write)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return info && static_cast<lldb::SBLaunchInfo*>(info)->AddSuppressFileAction(
                               fd, read != 0, write != 0)
               ? 1
               : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBAttachInfo
// ============================================================================

lldb_attach_info_t lldb_attach_info_create(uint64_t pid)  LLDB_WRAPPER_NOEXCEPT {
    try {
    lldb::SBAttachInfo info;
    if (pid != 0) info.SetProcessID(pid);
    return static_cast<lldb_attach_info_t>(new lldb::SBAttachInfo(info));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_attach_info_destroy(lldb_attach_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (info) delete static_cast<lldb::SBAttachInfo*>(info);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint64_t lldb_attach_info_get_process_id(lldb_attach_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return info ? static_cast<lldb::SBAttachInfo*>(info)->GetProcessID() : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_attach_info_set_process_id(lldb_attach_info_t info, uint64_t pid)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (info) static_cast<lldb::SBAttachInfo*>(info)->SetProcessID(pid);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

void lldb_attach_info_set_executable(lldb_attach_info_t info, const char* path)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (info && path) static_cast<lldb::SBAttachInfo*>(info)->SetExecutable(path);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

void lldb_attach_info_set_executable_file(lldb_attach_info_t info, lldb_file_spec_t file_spec)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (info && file_spec) {
        static_cast<lldb::SBAttachInfo*>(info)->SetExecutable(
            *static_cast<lldb::SBFileSpec*>(file_spec));
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_attach_info_get_wait_for_launch(lldb_attach_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return info && static_cast<lldb::SBAttachInfo*>(info)->GetWaitForLaunch() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_attach_info_set_wait_for_launch(lldb_attach_info_t info, int wait_for)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (info) static_cast<lldb::SBAttachInfo*>(info)->SetWaitForLaunch(wait_for != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_attach_info_get_ignore_existing(lldb_attach_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return info && static_cast<lldb::SBAttachInfo*>(info)->GetIgnoreExisting() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_attach_info_set_ignore_existing(lldb_attach_info_t info, int ignore_existing)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (info) static_cast<lldb::SBAttachInfo*>(info)->SetIgnoreExisting(ignore_existing != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint32_t lldb_attach_info_get_resume_count(lldb_attach_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return info ? static_cast<lldb::SBAttachInfo*>(info)->GetResumeCount() : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_attach_info_set_resume_count(lldb_attach_info_t info, uint32_t count)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (info) static_cast<lldb::SBAttachInfo*>(info)->SetResumeCount(count);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

const char* lldb_attach_info_get_process_plugin_name(lldb_attach_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return nullptr;
    return static_cast<lldb::SBAttachInfo*>(info)->GetProcessPluginName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_attach_info_set_process_plugin_name(lldb_attach_info_t info, const char* name)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (info && name) static_cast<lldb::SBAttachInfo*>(info)->SetProcessPluginName(name);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

lldb_listener_t lldb_attach_info_get_listener(lldb_attach_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return nullptr;
    lldb::SBListener listener = static_cast<lldb::SBAttachInfo*>(info)->GetListener();
    if (!listener.IsValid()) return nullptr;
    return static_cast<lldb_listener_t>(new lldb::SBListener(listener));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_attach_info_set_listener(lldb_attach_info_t info, lldb_listener_t listener)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (info && listener) {
        static_cast<lldb::SBAttachInfo*>(info)->SetListener(
            *static_cast<lldb::SBListener*>(listener));
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

// ============================================================================
// SBExpressionOptions
// ============================================================================

lldb_expression_options_t lldb_expression_options_create(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return static_cast<lldb_expression_options_t>(new lldb::SBExpressionOptions());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_expression_options_destroy(lldb_expression_options_t options)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (options) delete static_cast<lldb::SBExpressionOptions*>(options);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint32_t lldb_expression_options_get_timeout(lldb_expression_options_t options)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return options ? static_cast<lldb::SBExpressionOptions*>(options)->GetTimeoutInMicroSeconds() : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_expression_options_set_timeout(lldb_expression_options_t options, uint32_t timeout)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (options) static_cast<lldb::SBExpressionOptions*>(options)->SetTimeoutInMicroSeconds(timeout);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_expression_options_get_unwind_on_error(lldb_expression_options_t options)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return options && static_cast<lldb::SBExpressionOptions*>(options)->GetUnwindOnError() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_expression_options_set_unwind_on_error(lldb_expression_options_t options, int value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (options) static_cast<lldb::SBExpressionOptions*>(options)->SetUnwindOnError(value != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_expression_options_get_ignore_breakpoints(lldb_expression_options_t options)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return options && static_cast<lldb::SBExpressionOptions*>(options)->GetIgnoreBreakpoints() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_expression_options_set_ignore_breakpoints(lldb_expression_options_t options, int value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (options) static_cast<lldb::SBExpressionOptions*>(options)->SetIgnoreBreakpoints(value != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_expression_options_get_fetch_dynamic_value(lldb_expression_options_t options)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return options ? static_cast<int>(static_cast<lldb::SBExpressionOptions*>(options)->GetFetchDynamicValue()) : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_expression_options_set_fetch_dynamic_value(lldb_expression_options_t options, int value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (options) {
        static_cast<lldb::SBExpressionOptions*>(options)->SetFetchDynamicValue(
            static_cast<lldb::DynamicValueType>(value));
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_expression_options_get_try_all_threads(lldb_expression_options_t options)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return options && static_cast<lldb::SBExpressionOptions*>(options)->GetTryAllThreads() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_expression_options_set_try_all_threads(lldb_expression_options_t options, int value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (options) static_cast<lldb::SBExpressionOptions*>(options)->SetTryAllThreads(value != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_expression_options_get_stop_others(lldb_expression_options_t options)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return options && static_cast<lldb::SBExpressionOptions*>(options)->GetStopOthers() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_expression_options_set_stop_others(lldb_expression_options_t options, int value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (options) static_cast<lldb::SBExpressionOptions*>(options)->SetStopOthers(value != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

void lldb_expression_options_set_language(lldb_expression_options_t options, int value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (options) {
        static_cast<lldb::SBExpressionOptions*>(options)->SetLanguage(
            static_cast<lldb::LanguageType>(value));
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_expression_options_get_suppress_persistent_result(lldb_expression_options_t options)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return options && static_cast<lldb::SBExpressionOptions*>(options)->GetSuppressPersistentResult() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_expression_options_set_suppress_persistent_result(lldb_expression_options_t options, int value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (options) {
        static_cast<lldb::SBExpressionOptions*>(options)->SetSuppressPersistentResult(value != 0);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

// ============================================================================
// SBFileSpec
// ============================================================================

lldb_file_spec_t lldb_file_spec_create(const char* path, int resolve)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (path) {
        return static_cast<lldb_file_spec_t>(new lldb::SBFileSpec(path, resolve != 0));
    }
    return static_cast<lldb_file_spec_t>(new lldb::SBFileSpec());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_file_spec_destroy(lldb_file_spec_t file_spec)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (file_spec) {
        delete static_cast<lldb::SBFileSpec*>(file_spec);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_file_spec_is_valid(lldb_file_spec_t file_spec)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!file_spec) return 0;
    return static_cast<lldb::SBFileSpec*>(file_spec)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_file_spec_exists(lldb_file_spec_t file_spec)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!file_spec) return 0;
    return static_cast<lldb::SBFileSpec*>(file_spec)->Exists() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_file_spec_get_filename(lldb_file_spec_t file_spec)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!file_spec) return nullptr;
    return static_cast<lldb::SBFileSpec*>(file_spec)->GetFilename();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_file_spec_get_directory(lldb_file_spec_t file_spec)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!file_spec) return nullptr;
    return static_cast<lldb::SBFileSpec*>(file_spec)->GetDirectory();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_file_spec_get_path(lldb_file_spec_t file_spec, char* buffer, size_t length)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!file_spec) return 0;
    return static_cast<lldb::SBFileSpec*>(file_spec)->GetPath(buffer, length);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_file_spec_set_filename(lldb_file_spec_t file_spec, const char* filename)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!file_spec || !filename) return;
    static_cast<lldb::SBFileSpec*>(file_spec)->SetFilename(filename);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

void lldb_file_spec_set_directory(lldb_file_spec_t file_spec, const char* directory)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!file_spec || !directory) return;
    static_cast<lldb::SBFileSpec*>(file_spec)->SetDirectory(directory);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

// ============================================================================
// SBFileSpecList
// ============================================================================

lldb_file_spec_list_t lldb_file_spec_list_create(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return static_cast<lldb_file_spec_list_t>(new lldb::SBFileSpecList());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_file_spec_list_destroy(lldb_file_spec_list_t list)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (list) delete static_cast<lldb::SBFileSpecList*>(list);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_file_spec_list_is_valid(lldb_file_spec_list_t list)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return list ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_file_spec_list_get_size(lldb_file_spec_list_t list)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!list) return 0;
    return static_cast<lldb::SBFileSpecList*>(list)->GetSize();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_file_spec_list_append(lldb_file_spec_list_t list, lldb_file_spec_t file_spec)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!list || !file_spec) return;
    static_cast<lldb::SBFileSpecList*>(list)->Append(
        *static_cast<lldb::SBFileSpec*>(file_spec));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_file_spec_list_append_if_unique(lldb_file_spec_list_t list,
                                         lldb_file_spec_t file_spec)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!list || !file_spec) return 0;
    return static_cast<lldb::SBFileSpecList*>(list)->AppendIfUnique(
               *static_cast<lldb::SBFileSpec*>(file_spec))
               ? 1
               : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_file_spec_list_clear(lldb_file_spec_list_t list)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (list) static_cast<lldb::SBFileSpecList*>(list)->Clear();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

lldb_file_spec_t lldb_file_spec_list_get_file_spec_at_index(lldb_file_spec_list_t list,
                                                             uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!list) return nullptr;
    lldb::SBFileSpec file_spec =
        static_cast<lldb::SBFileSpecList*>(list)->GetFileSpecAtIndex(index);
    if (!file_spec.IsValid()) return nullptr;
    return static_cast<lldb_file_spec_t>(new lldb::SBFileSpec(file_spec));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBAddress
// ============================================================================

lldb_address_t lldb_address_create(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return static_cast<lldb_address_t>(new lldb::SBAddress());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_address_t lldb_address_create_from_load_address(uint64_t address, lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!target) return nullptr;
    return static_cast<lldb_address_t>(new lldb::SBAddress(
        address, *static_cast<lldb::SBTarget*>(target)));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_address_destroy(lldb_address_t address)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (address) delete static_cast<lldb::SBAddress*>(address);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_address_is_valid(lldb_address_t address)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!address) return 0;
    return static_cast<lldb::SBAddress*>(address)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_address_get_file_address(lldb_address_t address)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!address) return LLDB_INVALID_ADDRESS;
    return static_cast<lldb::SBAddress*>(address)->GetFileAddress();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_address_get_load_address(lldb_address_t address, lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!address || !target) return LLDB_INVALID_ADDRESS;
    return static_cast<lldb::SBAddress*>(address)->GetLoadAddress(
        *static_cast<lldb::SBTarget*>(target));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_address_get_offset(lldb_address_t address)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!address) return 0;
    return static_cast<lldb::SBAddress*>(address)->GetOffset();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_line_entry_t lldb_address_get_line_entry(lldb_address_t address)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!address) return nullptr;
    lldb::SBLineEntry entry = static_cast<lldb::SBAddress*>(address)->GetLineEntry();
    if (!entry.IsValid()) return nullptr;
    return static_cast<lldb_line_entry_t>(new lldb::SBLineEntry(entry));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBLineEntry
// ============================================================================

void lldb_line_entry_destroy(lldb_line_entry_t entry)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (entry) delete static_cast<lldb::SBLineEntry*>(entry);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_line_entry_is_valid(lldb_line_entry_t entry)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!entry) return 0;
    return static_cast<lldb::SBLineEntry*>(entry)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_address_t lldb_line_entry_get_start_address(lldb_line_entry_t entry)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!entry) return nullptr;
    lldb::SBAddress address = static_cast<lldb::SBLineEntry*>(entry)->GetStartAddress();
    if (!address.IsValid()) return nullptr;
    return static_cast<lldb_address_t>(new lldb::SBAddress(address));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_address_t lldb_line_entry_get_end_address(lldb_line_entry_t entry)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!entry) return nullptr;
    lldb::SBAddress address = static_cast<lldb::SBLineEntry*>(entry)->GetEndAddress();
    if (!address.IsValid()) return nullptr;
    return static_cast<lldb_address_t>(new lldb::SBAddress(address));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_file_spec_t lldb_line_entry_get_file_spec(lldb_line_entry_t entry)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!entry) return nullptr;
    lldb::SBFileSpec file_spec = static_cast<lldb::SBLineEntry*>(entry)->GetFileSpec();
    if (!file_spec.IsValid()) return nullptr;
    return static_cast<lldb_file_spec_t>(new lldb::SBFileSpec(file_spec));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_line_entry_get_line(lldb_line_entry_t entry)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!entry) return LLDB_INVALID_LINE_NUMBER;
    return static_cast<lldb::SBLineEntry*>(entry)->GetLine();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_line_entry_get_column(lldb_line_entry_t entry)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!entry) return 0;
    return static_cast<lldb::SBLineEntry*>(entry)->GetColumn();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBProcess
// ============================================================================

void lldb_process_destroy(lldb_process_t process)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (process) {
        delete static_cast<lldb::SBProcess*>(process);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_process_is_valid(lldb_process_t process)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_ruby_status_t lldb_process_continue(lldb_process_t process, lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!process) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Continue();
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in process continue");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

lldb_ruby_status_t lldb_process_stop(lldb_process_t process, lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!process) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Stop();
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in process stop");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

lldb_ruby_status_t lldb_process_kill(lldb_process_t process, lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!process) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Kill();
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in process kill");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

lldb_ruby_status_t lldb_process_detach(lldb_process_t process, lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!process) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Detach();
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in process detach");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

lldb_ruby_status_t lldb_process_destroy_process(lldb_process_t process, lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!process) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Destroy();
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in process destroy");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

lldb_ruby_status_t lldb_process_signal(lldb_process_t process, int signal, lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!process) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error = static_cast<lldb::SBProcess*>(process)->Signal(signal);
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in process signal");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

int lldb_process_get_state(lldb_process_t process)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return static_cast<int>(lldb::eStateInvalid);
    return static_cast<int>(static_cast<lldb::SBProcess*>(process)->GetState());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_process_get_num_threads(lldb_process_t process)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->GetNumThreads();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_thread_t lldb_process_get_thread_at_index(lldb_process_t process, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return nullptr;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBThread thread = p->GetThreadAtIndex(index);

    if (!thread.IsValid()) return nullptr;

    return static_cast<lldb_thread_t>(new lldb::SBThread(thread));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_thread_t lldb_process_get_thread_by_id(lldb_process_t process, uint64_t tid)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return nullptr;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBThread thread = p->GetThreadByID(tid);

    if (!thread.IsValid()) return nullptr;

    return static_cast<lldb_thread_t>(new lldb::SBThread(thread));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_thread_t lldb_process_get_thread_by_index_id(lldb_process_t process, uint32_t index_id)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return nullptr;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBThread thread = p->GetThreadByIndexID(index_id);

    if (!thread.IsValid()) return nullptr;

    return static_cast<lldb_thread_t>(new lldb::SBThread(thread));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_thread_t lldb_process_get_selected_thread(lldb_process_t process)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return nullptr;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBThread thread = p->GetSelectedThread();

    if (!thread.IsValid()) return nullptr;

    return static_cast<lldb_thread_t>(new lldb::SBThread(thread));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_process_set_selected_thread_by_id(lldb_process_t process, uint64_t tid)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->SetSelectedThreadByID(tid) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_process_set_selected_thread_by_index_id(lldb_process_t process, uint32_t index_id)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->SetSelectedThreadByIndexID(index_id) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_process_get_process_id(lldb_process_t process)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->GetProcessID();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_process_get_exit_status(lldb_process_t process)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return -1;
    return static_cast<lldb::SBProcess*>(process)->GetExitStatus();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_process_get_exit_description(lldb_process_t process)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return nullptr;
    return static_cast<lldb::SBProcess*>(process)->GetExitDescription();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

size_t lldb_process_read_memory(lldb_process_t process, uint64_t addr, void* buf, size_t size, lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process || !buf) return 0;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    return p->ReadMemory(addr, buf, size, err ? *err : local_error);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

size_t lldb_process_write_memory(lldb_process_t process, uint64_t addr, const void* buf, size_t size, lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process || !buf) return 0;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    return p->WriteMemory(addr, buf, size, err ? *err : local_error);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_process_allocate_memory(lldb_process_t process, size_t size, uint32_t permissions, lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return LLDB_INVALID_ADDRESS;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    return p->AllocateMemory(size, permissions, err ? *err : local_error);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_ruby_status_t lldb_process_deallocate_memory(lldb_process_t process,
                                                  uint64_t addr,
                                                  lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!process) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error = static_cast<lldb::SBProcess*>(process)->DeallocateMemory(addr);
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in process deallocate");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

size_t lldb_process_read_cstring_from_memory(lldb_process_t process, uint64_t addr, void* buf, size_t size, lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process || !buf) return 0;

    lldb::SBProcess* p = static_cast<lldb::SBProcess*>(process);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    return p->ReadCStringFromMemory(addr, buf, size, err ? *err : local_error);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

size_t lldb_process_get_stdout(lldb_process_t process, char* buf, size_t size)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process || !buf) return 0;
    return static_cast<lldb::SBProcess*>(process)->GetSTDOUT(buf, size);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

size_t lldb_process_get_stderr(lldb_process_t process, char* buf, size_t size)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process || !buf) return 0;
    return static_cast<lldb::SBProcess*>(process)->GetSTDERR(buf, size);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

size_t lldb_process_put_stdin(lldb_process_t process, const char* buf, size_t size)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process || !buf) return 0;
    return static_cast<lldb::SBProcess*>(process)->PutSTDIN(buf, size);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_process_send_async_interrupt(lldb_process_t process)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return;
    static_cast<lldb::SBProcess*>(process)->SendAsyncInterrupt();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

lldb_broadcaster_t lldb_process_get_broadcaster(lldb_process_t process)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return nullptr;
    lldb::SBBroadcaster broadcaster = static_cast<lldb::SBProcess*>(process)->GetBroadcaster();
    if (!broadcaster.IsValid()) return nullptr;
    return static_cast<lldb_broadcaster_t>(new lldb::SBBroadcaster(broadcaster));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_ruby_status_t lldb_process_get_num_supported_hardware_watchpoints(lldb_process_t process,
                                                                        uint32_t* result,
                                                                        lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!process || !result) return LLDB_RUBY_STATUS_INVALID_ARGUMENT;

    try {
        lldb::SBError error;
        *result = static_cast<lldb::SBProcess*>(process)->GetNumSupportedHardwareWatchpoints(error);
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in hardware watchpoint query");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

uint32_t lldb_process_get_unique_id(lldb_process_t process)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!process) return 0;
    return static_cast<lldb::SBProcess*>(process)->GetUniqueID();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_memory_region_info_t lldb_process_get_memory_region_info(lldb_process_t process, uint64_t addr, lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
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

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBMemoryRegionInfo
// ============================================================================

void lldb_memory_region_info_destroy(lldb_memory_region_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (info) {
        delete static_cast<lldb::SBMemoryRegionInfo*>(info);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint64_t lldb_memory_region_info_get_region_base(lldb_memory_region_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->GetRegionBase();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_memory_region_info_get_region_end(lldb_memory_region_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->GetRegionEnd();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_memory_region_info_is_readable(lldb_memory_region_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->IsReadable() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_memory_region_info_is_writable(lldb_memory_region_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->IsWritable() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_memory_region_info_is_executable(lldb_memory_region_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->IsExecutable() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_memory_region_info_is_mapped(lldb_memory_region_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return 0;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->IsMapped() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_memory_region_info_get_name(lldb_memory_region_info_t info)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!info) return nullptr;
    return static_cast<lldb::SBMemoryRegionInfo*>(info)->GetName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBThread
// ============================================================================

void lldb_thread_destroy(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (thread) {
        delete static_cast<lldb::SBThread*>(thread);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_thread_is_valid(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_ruby_status_t lldb_thread_step_over(lldb_thread_t thread, int run_mode, lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!thread) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error;
        static_cast<lldb::SBThread*>(thread)->StepOver(static_cast<lldb::RunMode>(run_mode), error);
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in thread step over");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

lldb_ruby_status_t lldb_thread_step_into(lldb_thread_t thread,
                                         const char* target_name,
                                         uint32_t end_line,
                                         int run_mode,
                                         lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!thread) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error;
        static_cast<lldb::SBThread*>(thread)->StepInto(
            target_name,
            end_line,
            error,
            static_cast<lldb::RunMode>(run_mode)
        );
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in thread step into");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

lldb_ruby_status_t lldb_thread_step_out(lldb_thread_t thread, lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!thread) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error;
        static_cast<lldb::SBThread*>(thread)->StepOut(error);
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in thread step out");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

lldb_ruby_status_t lldb_thread_step_instruction(lldb_thread_t thread,
                                                int step_over,
                                                lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!thread) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error;
        static_cast<lldb::SBThread*>(thread)->StepInstruction(step_over != 0, error);
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in thread instruction step");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

lldb_ruby_status_t lldb_thread_run_to_address(lldb_thread_t thread,
                                              uint64_t addr,
                                              lldb_error_t output)  LLDB_WRAPPER_NOEXCEPT {
    try {
    wrapper_clear_error_state();
    if (!thread) return LLDB_RUBY_STATUS_INVALID_HANDLE;
    try {
        lldb::SBError error;
        static_cast<lldb::SBThread*>(thread)->RunToAddress(addr, error);
        return wrapper_status(error, output);
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception in thread run to address");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

uint32_t lldb_thread_get_num_frames(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->GetNumFrames();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_frame_t lldb_thread_get_frame_at_index(lldb_thread_t thread, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return nullptr;

    lldb::SBThread* t = static_cast<lldb::SBThread*>(thread);
    lldb::SBFrame frame = t->GetFrameAtIndex(index);

    if (!frame.IsValid()) return nullptr;

    return static_cast<lldb_frame_t>(new lldb::SBFrame(frame));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_frame_t lldb_thread_get_selected_frame(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return nullptr;

    lldb::SBThread* t = static_cast<lldb::SBThread*>(thread);
    lldb::SBFrame frame = t->GetSelectedFrame();

    if (!frame.IsValid()) return nullptr;

    return static_cast<lldb_frame_t>(new lldb::SBFrame(frame));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_thread_set_selected_frame(lldb_thread_t thread, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    lldb::SBFrame frame = static_cast<lldb::SBThread*>(thread)->SetSelectedFrame(index);
    return frame.IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_thread_get_thread_id(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->GetThreadID();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_thread_get_index_id(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->GetIndexID();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_thread_get_name(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return nullptr;
    return static_cast<lldb::SBThread*>(thread)->GetName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_thread_get_queue_name(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return nullptr;
    return static_cast<lldb::SBThread*>(thread)->GetQueueName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_thread_get_stop_reason(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return static_cast<int>(lldb::eStopReasonInvalid);
    return static_cast<int>(static_cast<lldb::SBThread*>(thread)->GetStopReason());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

size_t lldb_thread_get_stop_description(lldb_thread_t thread, char* buffer, size_t length)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->GetStopDescription(buffer, length);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_thread_get_stop_reason_data_count(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->GetStopReasonDataCount();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_thread_get_stop_reason_data_at_index(lldb_thread_t thread, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->GetStopReasonDataAtIndex(index);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_thread_is_stopped(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->IsStopped() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_thread_is_suspended(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->IsSuspended() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_thread_suspend(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->Suspend() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_thread_resume(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return 0;
    return static_cast<lldb::SBThread*>(thread)->Resume() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_process_t lldb_thread_get_process(lldb_thread_t thread)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!thread) return nullptr;

    lldb::SBThread* t = static_cast<lldb::SBThread*>(thread);
    lldb::SBProcess process = t->GetProcess();

    if (!process.IsValid()) return nullptr;

    return static_cast<lldb_process_t>(new lldb::SBProcess(process));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBFrame
// ============================================================================

void lldb_frame_destroy(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (frame) {
        delete static_cast<lldb::SBFrame*>(frame);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_frame_is_valid(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return 0;
    return static_cast<lldb::SBFrame*>(frame)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_frame_get_function_name(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;
    return static_cast<lldb::SBFrame*>(frame)->GetFunctionName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_frame_get_display_function_name(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;
    return static_cast<lldb::SBFrame*>(frame)->GetDisplayFunctionName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_frame_get_line(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return 0;
    lldb::SBLineEntry line_entry = static_cast<lldb::SBFrame*>(frame)->GetLineEntry();
    return line_entry.GetLine();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_frame_get_file_path(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBLineEntry line_entry = f->GetLineEntry();
    lldb::SBFileSpec file_spec = line_entry.GetFileSpec();

    if (!file_spec.IsValid()) return nullptr;

    if (wrapper_copy_file_spec_path(file_spec, g_temp_string)) {
        return g_temp_string.c_str();
    }

    return nullptr;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_file_spec_t lldb_frame_get_file_spec(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;

    lldb::SBFileSpec file_spec = static_cast<lldb::SBFrame*>(frame)->GetLineEntry().GetFileSpec();
    if (!file_spec.IsValid()) return nullptr;

    return static_cast<lldb_file_spec_t>(new lldb::SBFileSpec(file_spec));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_line_entry_t lldb_frame_get_line_entry(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;

    lldb::SBLineEntry line_entry = static_cast<lldb::SBFrame*>(frame)->GetLineEntry();
    if (!line_entry.IsValid()) return nullptr;

    return static_cast<lldb_line_entry_t>(new lldb::SBLineEntry(line_entry));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_frame_get_column(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return 0;
    lldb::SBLineEntry line_entry = static_cast<lldb::SBFrame*>(frame)->GetLineEntry();
    return line_entry.GetColumn();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_frame_get_pc(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return LLDB_INVALID_ADDRESS;
    return static_cast<lldb::SBFrame*>(frame)->GetPC();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_frame_set_pc(lldb_frame_t frame, uint64_t new_pc)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return 0;
    return static_cast<lldb::SBFrame*>(frame)->SetPC(new_pc) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_frame_get_sp(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return LLDB_INVALID_ADDRESS;
    return static_cast<lldb::SBFrame*>(frame)->GetSP();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_frame_get_fp(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return LLDB_INVALID_ADDRESS;
    return static_cast<lldb::SBFrame*>(frame)->GetFP();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_frame_find_variable(lldb_frame_t frame, const char* name)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame || !name) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBValue value = f->FindVariable(name);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_frame_evaluate_expression(lldb_frame_t frame, const char* expr)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame || !expr) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBValue value = f->EvaluateExpression(expr);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_frame_evaluate_expression_with_options(lldb_frame_t frame,
                                                         const char* expr,
                                                         lldb_expression_options_t options)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame || !expr || !options) return nullptr;

    lldb::SBValue value = static_cast<lldb::SBFrame*>(frame)->EvaluateExpression(
        expr, *static_cast<lldb::SBExpressionOptions*>(options));
    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_frame_get_value_for_variable_path(lldb_frame_t frame, const char* path)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame || !path) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBValue value = f->GetValueForVariablePath(path);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_frame_get_frame_id(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return 0;
    return static_cast<lldb::SBFrame*>(frame)->GetFrameID();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_thread_t lldb_frame_get_thread(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBThread thread = f->GetThread();

    if (!thread.IsValid()) return nullptr;

    return static_cast<lldb_thread_t>(new lldb::SBThread(thread));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_function_t lldb_frame_get_function(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;
    lldb::SBFunction function = static_cast<lldb::SBFrame*>(frame)->GetFunction();
    if (!function.IsValid()) return nullptr;
    return static_cast<lldb_function_t>(new lldb::SBFunction(function));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_symbol_t lldb_frame_get_symbol(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;
    lldb::SBSymbol symbol = static_cast<lldb::SBFrame*>(frame)->GetSymbol();
    if (!symbol.IsValid()) return nullptr;
    return static_cast<lldb_symbol_t>(new lldb::SBSymbol(symbol));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_compile_unit_t lldb_frame_get_compile_unit(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;
    lldb::SBCompileUnit unit = static_cast<lldb::SBFrame*>(frame)->GetCompileUnit();
    if (!unit.IsValid()) return nullptr;
    return static_cast<lldb_compile_unit_t>(new lldb::SBCompileUnit(unit));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_block_t lldb_frame_get_block(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;
    lldb::SBBlock block = static_cast<lldb::SBFrame*>(frame)->GetBlock();
    if (!block.IsValid()) return nullptr;
    return static_cast<lldb_block_t>(new lldb::SBBlock(block));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_instruction_list_t lldb_frame_get_instruction_list(lldb_frame_t frame,
                                                         lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame || !target) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBTarget* t = static_cast<lldb::SBTarget*>(target);
    lldb::SBInstructionList instructions;
    lldb::SBFunction function = f->GetFunction();
    if (function.IsValid()) {
        instructions = function.GetInstructions(*t);
    } else {
        lldb::SBSymbol symbol = f->GetSymbol();
        if (symbol.IsValid()) instructions = symbol.GetInstructions(*t);
    }

    if (!instructions.IsValid()) return nullptr;
    return static_cast<lldb_instruction_list_t>(new lldb::SBInstructionList(instructions));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_symbol_context_t lldb_frame_get_symbol_context(lldb_frame_t frame, uint32_t scope)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBSymbolContext ctx = f->GetSymbolContext(scope);

    return static_cast<lldb_symbol_context_t>(new lldb::SBSymbolContext(ctx));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_list_t lldb_frame_get_variables(lldb_frame_t frame, int arguments, int locals, int statics, int in_scope_only)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBValueList list = f->GetVariables(arguments != 0, locals != 0, statics != 0, in_scope_only != 0);

    return static_cast<lldb_value_list_t>(new lldb::SBValueList(list));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_list_t lldb_frame_get_registers(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBValueList list = f->GetRegisters();

    return static_cast<lldb_value_list_t>(new lldb::SBValueList(list));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_frame_is_inlined(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return 0;
    return static_cast<lldb::SBFrame*>(frame)->IsInlined() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_frame_disassemble(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;
    return static_cast<lldb::SBFrame*>(frame)->Disassemble();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_module_t lldb_frame_get_module(lldb_frame_t frame)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!frame) return nullptr;

    lldb::SBFrame* f = static_cast<lldb::SBFrame*>(frame);
    lldb::SBModule module = f->GetModule();

    if (!module.IsValid()) return nullptr;

    return static_cast<lldb_module_t>(new lldb::SBModule(module));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBBreakpoint
// ============================================================================

void lldb_breakpoint_destroy(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (bp) {
        delete static_cast<lldb::SBBreakpoint*>(bp);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_breakpoint_is_valid(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int32_t lldb_breakpoint_get_id(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return -1;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetID();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_breakpoint_is_enabled(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->IsEnabled() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_breakpoint_set_enabled(lldb_breakpoint_t bp, int enabled)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetEnabled(enabled != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_breakpoint_is_one_shot(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->IsOneShot() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_breakpoint_set_one_shot(lldb_breakpoint_t bp, int one_shot)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetOneShot(one_shot != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint32_t lldb_breakpoint_get_hit_count(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetHitCount();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_breakpoint_get_ignore_count(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetIgnoreCount();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_breakpoint_set_ignore_count(lldb_breakpoint_t bp, uint32_t count)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetIgnoreCount(count);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

const char* lldb_breakpoint_get_condition(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return nullptr;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetCondition();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_breakpoint_set_condition(lldb_breakpoint_t bp, const char* condition)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetCondition(condition);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint32_t lldb_breakpoint_get_num_locations(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetNumLocations();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_breakpoint_location_t lldb_breakpoint_get_location_at_index(lldb_breakpoint_t bp, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return nullptr;

    lldb::SBBreakpoint* b = static_cast<lldb::SBBreakpoint*>(bp);
    lldb::SBBreakpointLocation loc = b->GetLocationAtIndex(index);

    if (!loc.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_location_t>(new lldb::SBBreakpointLocation(loc));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_breakpoint_location_t lldb_breakpoint_find_location_by_id(lldb_breakpoint_t bp, int32_t id)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return nullptr;

    lldb::SBBreakpoint* b = static_cast<lldb::SBBreakpoint*>(bp);
    lldb::SBBreakpointLocation loc = b->FindLocationByID(id);

    if (!loc.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_location_t>(new lldb::SBBreakpointLocation(loc));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_breakpoint_is_hardware(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->IsHardware() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_breakpoint_get_auto_continue(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetAutoContinue() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_breakpoint_set_auto_continue(lldb_breakpoint_t bp, int auto_continue)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetAutoContinue(auto_continue != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint64_t lldb_breakpoint_get_thread_id(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetThreadID();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_breakpoint_set_thread_id(lldb_breakpoint_t bp, uint64_t tid)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetThreadID(tid);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

const char* lldb_breakpoint_get_thread_name(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return nullptr;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetThreadName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_breakpoint_set_thread_name(lldb_breakpoint_t bp, const char* name)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetThreadName(name);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint32_t lldb_breakpoint_get_thread_index(lldb_breakpoint_t bp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return 0;
    return static_cast<lldb::SBBreakpoint*>(bp)->GetThreadIndex();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_breakpoint_set_thread_index(lldb_breakpoint_t bp, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!bp) return;
    static_cast<lldb::SBBreakpoint*>(bp)->SetThreadIndex(index);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

// ============================================================================
// SBBreakpointLocation
// ============================================================================

void lldb_breakpoint_location_destroy(lldb_breakpoint_location_t loc)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (loc) {
        delete static_cast<lldb::SBBreakpointLocation*>(loc);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_breakpoint_location_is_valid(lldb_breakpoint_location_t loc)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return 0;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int32_t lldb_breakpoint_location_get_id(lldb_breakpoint_location_t loc)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return LLDB_INVALID_BREAK_ID;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->GetID();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_breakpoint_location_get_load_address(lldb_breakpoint_location_t loc)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return LLDB_INVALID_ADDRESS;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->GetLoadAddress();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_address_t lldb_breakpoint_location_get_address(lldb_breakpoint_location_t loc)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return nullptr;

    lldb::SBAddress address = static_cast<lldb::SBBreakpointLocation*>(loc)->GetAddress();
    if (!address.IsValid()) return nullptr;

    return static_cast<lldb_address_t>(new lldb::SBAddress(address));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_breakpoint_location_is_enabled(lldb_breakpoint_location_t loc)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return 0;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->IsEnabled() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_breakpoint_location_set_enabled(lldb_breakpoint_location_t loc, int enabled)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return;
    static_cast<lldb::SBBreakpointLocation*>(loc)->SetEnabled(enabled != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint32_t lldb_breakpoint_location_get_hit_count(lldb_breakpoint_location_t loc)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return 0;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->GetHitCount();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_breakpoint_location_get_ignore_count(lldb_breakpoint_location_t loc)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return 0;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->GetIgnoreCount();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_breakpoint_location_set_ignore_count(lldb_breakpoint_location_t loc, uint32_t count)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return;
    static_cast<lldb::SBBreakpointLocation*>(loc)->SetIgnoreCount(count);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

const char* lldb_breakpoint_location_get_condition(lldb_breakpoint_location_t loc)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return nullptr;
    return static_cast<lldb::SBBreakpointLocation*>(loc)->GetCondition();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_breakpoint_location_set_condition(lldb_breakpoint_location_t loc, const char* condition)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return;
    static_cast<lldb::SBBreakpointLocation*>(loc)->SetCondition(condition);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

lldb_breakpoint_t lldb_breakpoint_location_get_breakpoint(lldb_breakpoint_location_t loc)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!loc) return nullptr;

    lldb::SBBreakpointLocation* l = static_cast<lldb::SBBreakpointLocation*>(loc);
    lldb::SBBreakpoint bp = l->GetBreakpoint();

    if (!bp.IsValid()) return nullptr;

    return static_cast<lldb_breakpoint_t>(new lldb::SBBreakpoint(bp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBValue
// ============================================================================

void lldb_value_destroy(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (value) {
        delete static_cast<lldb::SBValue*>(value);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_value_is_valid(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_value_get_name(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return nullptr;
    return static_cast<lldb::SBValue*>(value)->GetName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_value_get_value(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return nullptr;
    return static_cast<lldb::SBValue*>(value)->GetValue();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_value_get_summary(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return nullptr;
    return static_cast<lldb::SBValue*>(value)->GetSummary();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_value_get_type_name(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return nullptr;
    return static_cast<lldb::SBValue*>(value)->GetTypeName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_t lldb_value_get_type(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBType type = v->GetType();

    if (!type.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(type));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_value_get_num_children(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->GetNumChildren();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_value_get_child_at_index(lldb_value_t value, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue child = v->GetChildAtIndex(index);

    if (!child.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(child));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_value_get_child_member_with_name(lldb_value_t value, const char* name)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value || !name) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue child = v->GetChildMemberWithName(name);

    if (!child.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(child));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int64_t lldb_value_get_value_as_signed(lldb_value_t value, lldb_error_t output, int64_t fail_value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) {
        wrapper_set_invalid_argument(output ? static_cast<lldb::SBError*>(output) : nullptr,
                                     "invalid SBValue handle");
        return fail_value;
    }
    lldb::SBError local_error;
    lldb::SBError* error = output ? static_cast<lldb::SBError*>(output) : &local_error;
    return static_cast<lldb::SBValue*>(value)->GetValueAsSigned(*error, fail_value);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_value_get_value_as_unsigned(lldb_value_t value, lldb_error_t output, uint64_t fail_value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) {
        wrapper_set_invalid_argument(output ? static_cast<lldb::SBError*>(output) : nullptr,
                                     "invalid SBValue handle");
        return fail_value;
    }
    lldb::SBError local_error;
    lldb::SBError* error = output ? static_cast<lldb::SBError*>(output) : &local_error;
    return static_cast<lldb::SBValue*>(value)->GetValueAsUnsigned(*error, fail_value);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_value_get_byte_size(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->GetByteSize();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_value_might_have_children(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->MightHaveChildren() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_value_get_error(lldb_value_t value, lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return 0;
    lldb::SBError err = static_cast<lldb::SBValue*>(value)->GetError();
    if (error) {
        *static_cast<lldb::SBError*>(error) = err;
    }
    return err.Fail() ? 0 : 1;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_value_dereference(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue deref = v->Dereference();

    if (!deref.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(deref));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_value_address_of(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue addr = v->AddressOf();

    if (!addr.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(addr));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_value_cast(lldb_value_t value, lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value || !type) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBValue casted = v->Cast(*t);

    if (!casted.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(casted));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_value_get_load_address(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return LLDB_INVALID_ADDRESS;
    return static_cast<lldb::SBValue*>(value)->GetLoadAddress();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_value_get_value_type(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return static_cast<int>(lldb::eValueTypeInvalid);
    return static_cast<int>(static_cast<lldb::SBValue*>(value)->GetValueType());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_value_set_value_from_cstring(lldb_value_t value, const char* str, lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value || !str) return 0;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    return v->SetValueFromCString(str, err ? *err : local_error) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_value_create_child_at_offset(lldb_value_t value, const char* name, lldb_type_t type, uint32_t offset)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value || !type) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBValue child = v->CreateChildAtOffset(name, offset, *t);

    if (!child.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(child));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_value_create_value_from_address(lldb_value_t value, const char* name, uint64_t addr, lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value || !type) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBValue created = v->CreateValueFromAddress(name, addr, *t);

    if (!created.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(created));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_value_create_value_from_expression(lldb_value_t value, const char* name, const char* expr)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value || !name || !expr) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue created = v->CreateValueFromExpression(name, expr);

    if (!created.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(created));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_watchpoint_t lldb_value_watch(lldb_value_t value, int resolve_location, int read, int write, lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBError* err = error ? static_cast<lldb::SBError*>(error) : nullptr;
    lldb::SBError local_error;

    lldb::SBWatchpoint wp = v->Watch(resolve_location != 0, read != 0, write != 0, err ? *err : local_error);

    if (!wp.IsValid()) return nullptr;

    return static_cast<lldb_watchpoint_t>(new lldb::SBWatchpoint(wp));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_value_get_expression_path(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return nullptr;

    static thread_local std::string expr_path;
    lldb::SBStream stream;
    static_cast<lldb::SBValue*>(value)->GetExpressionPath(stream);
    expr_path = stream.GetData();
    return expr_path.c_str();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_value_is_pointer_type(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return 0;
    return static_cast<lldb::SBValue*>(value)->TypeIsPointerType() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_value_get_non_synthetic_value(lldb_value_t value)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!value) return nullptr;

    lldb::SBValue* v = static_cast<lldb::SBValue*>(value);
    lldb::SBValue non_synth = v->GetNonSyntheticValue();

    if (!non_synth.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(non_synth));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBValueList
// ============================================================================

void lldb_value_list_destroy(lldb_value_list_t list)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (list) {
        delete static_cast<lldb::SBValueList*>(list);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_value_list_is_valid(lldb_value_list_t list)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!list) return 0;
    return static_cast<lldb::SBValueList*>(list)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_value_list_get_size(lldb_value_list_t list)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!list) return 0;
    return static_cast<lldb::SBValueList*>(list)->GetSize();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_value_list_get_value_at_index(lldb_value_list_t list, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!list) return nullptr;

    lldb::SBValueList* l = static_cast<lldb::SBValueList*>(list);
    lldb::SBValue value = l->GetValueAtIndex(index);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_value_t lldb_value_list_get_first_value_by_name(lldb_value_list_t list, const char* name)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!list || !name) return nullptr;

    lldb::SBValueList* l = static_cast<lldb::SBValueList*>(list);
    lldb::SBValue value = l->GetFirstValueByName(name);

    if (!value.IsValid()) return nullptr;

    return static_cast<lldb_value_t>(new lldb::SBValue(value));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBError
// ============================================================================

lldb_error_t lldb_error_create(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return static_cast<lldb_error_t>(new lldb::SBError());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_error_destroy(lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (error) {
        delete static_cast<lldb::SBError*>(error);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_error_success(lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!error) return 0;
    return static_cast<lldb::SBError*>(error)->Success() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_error_fail(lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!error) return 1;
    return static_cast<lldb::SBError*>(error)->Fail() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_error_get_cstring(lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!error) return nullptr;
    return static_cast<lldb::SBError*>(error)->GetCString();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_error_get_error(lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!error) return 0;
    return static_cast<lldb::SBError*>(error)->GetError();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_error_get_type(lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!error) return static_cast<int>(lldb::eErrorTypeInvalid);
    return static_cast<int>(static_cast<lldb::SBError*>(error)->GetType());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_error_clear(lldb_error_t error)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!error) return;
    static_cast<lldb::SBError*>(error)->Clear();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

void lldb_error_set_error_string(lldb_error_t error, const char* str)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!error) return;
    static_cast<lldb::SBError*>(error)->SetErrorString(str);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

// ============================================================================
// SBModule
// ============================================================================

void lldb_module_destroy(lldb_module_t module)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (module) {
        delete static_cast<lldb::SBModule*>(module);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_module_is_valid(lldb_module_t module)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!module) return 0;
    return static_cast<lldb::SBModule*>(module)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_module_get_file_path(lldb_module_t module)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!module) return nullptr;

    lldb::SBModule* m = static_cast<lldb::SBModule*>(module);
    lldb::SBFileSpec spec = m->GetFileSpec();

    if (!spec.IsValid()) return nullptr;

    if (wrapper_copy_file_spec_path(spec, g_temp_string)) {
        return g_temp_string.c_str();
    }

    return nullptr;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_module_get_platform_file_path(lldb_module_t module)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!module) return nullptr;

    lldb::SBModule* m = static_cast<lldb::SBModule*>(module);
    lldb::SBFileSpec spec = m->GetPlatformFileSpec();

    if (!spec.IsValid()) return nullptr;

    if (wrapper_copy_file_spec_path(spec, g_temp_string2)) {
        return g_temp_string2.c_str();
    }

    return nullptr;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_file_spec_t lldb_module_get_file(lldb_module_t module)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!module) return nullptr;

    lldb::SBFileSpec spec = static_cast<lldb::SBModule*>(module)->GetFileSpec();
    if (!spec.IsValid()) return nullptr;

    return static_cast<lldb_file_spec_t>(new lldb::SBFileSpec(spec));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_file_spec_t lldb_module_get_platform_file(lldb_module_t module)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!module) return nullptr;

    lldb::SBFileSpec spec = static_cast<lldb::SBModule*>(module)->GetPlatformFileSpec();
    if (!spec.IsValid()) return nullptr;

    return static_cast<lldb_file_spec_t>(new lldb::SBFileSpec(spec));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_module_get_num_symbols(lldb_module_t module)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!module) return 0;
    return static_cast<lldb::SBModule*>(module)->GetNumSymbols();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_symbol_t lldb_module_get_symbol_at_index(lldb_module_t module, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!module) return nullptr;
    lldb::SBSymbol symbol = static_cast<lldb::SBModule*>(module)->GetSymbolAtIndex(index);
    if (!symbol.IsValid()) return nullptr;
    return static_cast<lldb_symbol_t>(new lldb::SBSymbol(symbol));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBSymbol
// ============================================================================

void lldb_symbol_destroy(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (symbol) delete static_cast<lldb::SBSymbol*>(symbol);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_symbol_is_valid(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return 0;
    return static_cast<lldb::SBSymbol*>(symbol)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_symbol_get_name(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return nullptr;
    return static_cast<lldb::SBSymbol*>(symbol)->GetName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_symbol_get_display_name(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return nullptr;
    return static_cast<lldb::SBSymbol*>(symbol)->GetDisplayName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_symbol_get_mangled_name(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return nullptr;
    return static_cast<lldb::SBSymbol*>(symbol)->GetMangledName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_symbol_get_base_name(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return nullptr;
#if LLDB_RUBY_HAVE_SYMBOL_GET_BASE_NAME
    return static_cast<lldb::SBSymbol*>(symbol)->GetBaseName();
#else
    return static_cast<lldb::SBSymbol*>(symbol)->GetName();
#endif

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_address_t lldb_symbol_get_start_address(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return nullptr;
    lldb::SBAddress address = static_cast<lldb::SBSymbol*>(symbol)->GetStartAddress();
    if (!address.IsValid()) return nullptr;
    return static_cast<lldb_address_t>(new lldb::SBAddress(address));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_address_t lldb_symbol_get_end_address(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return nullptr;
    lldb::SBAddress address = static_cast<lldb::SBSymbol*>(symbol)->GetEndAddress();
    if (!address.IsValid()) return nullptr;
    return static_cast<lldb_address_t>(new lldb::SBAddress(address));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_symbol_get_value(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return LLDB_INVALID_ADDRESS;
#if LLDB_RUBY_HAVE_SYMBOL_GET_VALUE
    return static_cast<lldb::SBSymbol*>(symbol)->GetValue();
#else
    return LLDB_INVALID_ADDRESS;
#endif

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_symbol_get_size(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return 0;
#if LLDB_RUBY_HAVE_SYMBOL_GET_SIZE
    return static_cast<lldb::SBSymbol*>(symbol)->GetSize();
#else
    lldb::SBSymbol* native_symbol = static_cast<lldb::SBSymbol*>(symbol);
    lldb::SBAddress start = native_symbol->GetStartAddress();
    lldb::SBAddress end = native_symbol->GetEndAddress();
    if (!start.IsValid() || !end.IsValid()) return 0;
    lldb::addr_t start_address = start.GetFileAddress();
    lldb::addr_t end_address = end.GetFileAddress();
    if (start_address == LLDB_INVALID_ADDRESS || end_address < start_address) return 0;
    return static_cast<uint64_t>(end_address - start_address);
#endif

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_symbol_get_prologue_byte_size(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return 0;
    return static_cast<lldb::SBSymbol*>(symbol)->GetPrologueByteSize();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_symbol_get_type(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return static_cast<int>(lldb::eSymbolTypeInvalid);
    return static_cast<int>(static_cast<lldb::SBSymbol*>(symbol)->GetType());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_symbol_get_id(lldb_symbol_t symbol)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!symbol) return 0;
#if LLDB_RUBY_HAVE_SYMBOL_GET_ID
    return static_cast<lldb::SBSymbol*>(symbol)->GetID();
#else
    return 0;
#endif

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBFunction
// ============================================================================

void lldb_function_destroy(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (function) delete static_cast<lldb::SBFunction*>(function);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_function_is_valid(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return 0;
    return static_cast<lldb::SBFunction*>(function)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_function_get_name(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return nullptr;
    return static_cast<lldb::SBFunction*>(function)->GetName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_function_get_display_name(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return nullptr;
    return static_cast<lldb::SBFunction*>(function)->GetDisplayName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_function_get_mangled_name(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return nullptr;
    return static_cast<lldb::SBFunction*>(function)->GetMangledName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_function_get_base_name(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return nullptr;
#if LLDB_RUBY_HAVE_FUNCTION_GET_BASE_NAME
    return static_cast<lldb::SBFunction*>(function)->GetBaseName();
#else
    return static_cast<lldb::SBFunction*>(function)->GetName();
#endif

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_address_t lldb_function_get_start_address(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return nullptr;
    lldb::SBAddress address = static_cast<lldb::SBFunction*>(function)->GetStartAddress();
    if (!address.IsValid()) return nullptr;
    return static_cast<lldb_address_t>(new lldb::SBAddress(address));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_address_t lldb_function_get_end_address(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return nullptr;
    lldb::SBAddress address = static_cast<lldb::SBFunction*>(function)->GetEndAddress();
    if (!address.IsValid()) return nullptr;
    return static_cast<lldb_address_t>(new lldb::SBAddress(address));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_function_get_prologue_byte_size(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return 0;
    return static_cast<lldb::SBFunction*>(function)->GetPrologueByteSize();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_t lldb_function_get_type(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return nullptr;
    lldb::SBType type = static_cast<lldb::SBFunction*>(function)->GetType();
    if (!type.IsValid()) return nullptr;
    return static_cast<lldb_type_t>(new lldb::SBType(type));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_block_t lldb_function_get_block(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return nullptr;
    lldb::SBBlock block = static_cast<lldb::SBFunction*>(function)->GetBlock();
    if (!block.IsValid()) return nullptr;
    return static_cast<lldb_block_t>(new lldb::SBBlock(block));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_function_is_optimized(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return 0;
    return static_cast<lldb::SBFunction*>(function)->GetIsOptimized() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_function_get_language(lldb_function_t function)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!function) return static_cast<int>(lldb::eLanguageTypeUnknown);
    return static_cast<int>(static_cast<lldb::SBFunction*>(function)->GetLanguage());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBCompileUnit
// ============================================================================

void lldb_compile_unit_destroy(lldb_compile_unit_t unit)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (unit) delete static_cast<lldb::SBCompileUnit*>(unit);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_compile_unit_is_valid(lldb_compile_unit_t unit)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!unit) return 0;
    return static_cast<lldb::SBCompileUnit*>(unit)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_file_spec_t lldb_compile_unit_get_file_spec(lldb_compile_unit_t unit)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!unit) return nullptr;
    lldb::SBFileSpec file_spec = static_cast<lldb::SBCompileUnit*>(unit)->GetFileSpec();
    if (!file_spec.IsValid()) return nullptr;
    return static_cast<lldb_file_spec_t>(new lldb::SBFileSpec(file_spec));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_compile_unit_get_num_line_entries(lldb_compile_unit_t unit)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!unit) return 0;
    return static_cast<lldb::SBCompileUnit*>(unit)->GetNumLineEntries();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_line_entry_t lldb_compile_unit_get_line_entry_at_index(lldb_compile_unit_t unit,
                                                             uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!unit) return nullptr;
    lldb::SBLineEntry entry = static_cast<lldb::SBCompileUnit*>(unit)->GetLineEntryAtIndex(index);
    if (!entry.IsValid()) return nullptr;
    return static_cast<lldb_line_entry_t>(new lldb::SBLineEntry(entry));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBBlock
// ============================================================================

void lldb_block_destroy(lldb_block_t block)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (block) delete static_cast<lldb::SBBlock*>(block);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_block_is_valid(lldb_block_t block)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!block) return 0;
    return static_cast<lldb::SBBlock*>(block)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_block_get_inlined_name(lldb_block_t block)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!block) return nullptr;
    return static_cast<lldb::SBBlock*>(block)->GetInlinedName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_file_spec_t lldb_block_get_inlined_call_site_file(lldb_block_t block)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!block) return nullptr;
    lldb::SBFileSpec file_spec = static_cast<lldb::SBBlock*>(block)->GetInlinedCallSiteFile();
    if (!file_spec.IsValid()) return nullptr;
    return static_cast<lldb_file_spec_t>(new lldb::SBFileSpec(file_spec));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_block_get_inlined_call_site_line(lldb_block_t block)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!block) return LLDB_INVALID_LINE_NUMBER;
    return static_cast<lldb::SBBlock*>(block)->GetInlinedCallSiteLine();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_block_get_inlined_call_site_column(lldb_block_t block)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!block) return 0;
    return static_cast<lldb::SBBlock*>(block)->GetInlinedCallSiteColumn();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_block_t lldb_block_get_parent(lldb_block_t block)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!block) return nullptr;
    lldb::SBBlock parent = static_cast<lldb::SBBlock*>(block)->GetParent();
    if (!parent.IsValid()) return nullptr;
    return static_cast<lldb_block_t>(new lldb::SBBlock(parent));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_block_t lldb_block_get_sibling(lldb_block_t block)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!block) return nullptr;
    lldb::SBBlock sibling = static_cast<lldb::SBBlock*>(block)->GetSibling();
    if (!sibling.IsValid()) return nullptr;
    return static_cast<lldb_block_t>(new lldb::SBBlock(sibling));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_block_t lldb_block_get_first_child(lldb_block_t block)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!block) return nullptr;
    lldb::SBBlock child = static_cast<lldb::SBBlock*>(block)->GetFirstChild();
    if (!child.IsValid()) return nullptr;
    return static_cast<lldb_block_t>(new lldb::SBBlock(child));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_block_get_num_ranges(lldb_block_t block)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!block) return 0;
    return static_cast<lldb::SBBlock*>(block)->GetNumRanges();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_address_t lldb_block_get_range_start_address(lldb_block_t block, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!block) return nullptr;
    lldb::SBAddress address = static_cast<lldb::SBBlock*>(block)->GetRangeStartAddress(index);
    if (!address.IsValid()) return nullptr;
    return static_cast<lldb_address_t>(new lldb::SBAddress(address));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_address_t lldb_block_get_range_end_address(lldb_block_t block, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!block) return nullptr;
    lldb::SBAddress address = static_cast<lldb::SBBlock*>(block)->GetRangeEndAddress(index);
    if (!address.IsValid()) return nullptr;
    return static_cast<lldb_address_t>(new lldb::SBAddress(address));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBInstructionList
// ============================================================================

void lldb_instruction_list_destroy(lldb_instruction_list_t list)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (list) delete static_cast<lldb::SBInstructionList*>(list);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_instruction_list_is_valid(lldb_instruction_list_t list)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!list) return 0;
    return static_cast<lldb::SBInstructionList*>(list)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_instruction_list_get_size(lldb_instruction_list_t list)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!list) return 0;
    return static_cast<lldb::SBInstructionList*>(list)->GetSize();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_instruction_t lldb_instruction_list_get_instruction_at_index(lldb_instruction_list_t list,
                                                                   uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!list) return nullptr;
    lldb::SBInstruction instruction =
        static_cast<lldb::SBInstructionList*>(list)->GetInstructionAtIndex(index);
    if (!instruction.IsValid()) return nullptr;
    return static_cast<lldb_instruction_t>(new lldb::SBInstruction(instruction));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBInstruction
// ============================================================================

void lldb_instruction_destroy(lldb_instruction_t instruction)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (instruction) delete static_cast<lldb::SBInstruction*>(instruction);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_instruction_is_valid(lldb_instruction_t instruction)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!instruction) return 0;
    return static_cast<lldb::SBInstruction*>(instruction)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_address_t lldb_instruction_get_address(lldb_instruction_t instruction)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!instruction) return nullptr;
    lldb::SBAddress address = static_cast<lldb::SBInstruction*>(instruction)->GetAddress();
    if (!address.IsValid()) return nullptr;
    return static_cast<lldb_address_t>(new lldb::SBAddress(address));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_instruction_get_mnemonic(lldb_instruction_t instruction, lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!instruction || !target) return nullptr;
    return static_cast<lldb::SBInstruction*>(instruction)->GetMnemonic(
        *static_cast<lldb::SBTarget*>(target));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_instruction_get_operands(lldb_instruction_t instruction, lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!instruction || !target) return nullptr;
    return static_cast<lldb::SBInstruction*>(instruction)->GetOperands(
        *static_cast<lldb::SBTarget*>(target));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_instruction_get_comment(lldb_instruction_t instruction, lldb_target_t target)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!instruction || !target) return nullptr;
    return static_cast<lldb::SBInstruction*>(instruction)->GetComment(
        *static_cast<lldb::SBTarget*>(target));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_instruction_get_byte_size(lldb_instruction_t instruction)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!instruction) return 0;
    return static_cast<lldb::SBInstruction*>(instruction)->GetByteSize();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_instruction_get_bytes(lldb_instruction_t instruction,
                                    lldb_target_t target,
                                    uint8_t* buffer,
                                    uint32_t length)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!instruction || !target) return 0;

    lldb::SBData data = static_cast<lldb::SBInstruction*>(instruction)->GetData(
        *static_cast<lldb::SBTarget*>(target));
    if (!data.IsValid()) return 0;

    size_t size = data.GetByteSize();
    if (!buffer || length == 0) return static_cast<uint32_t>(std::min<size_t>(size, UINT32_MAX));

    lldb::SBError error;
    size_t written = data.ReadRawData(error, 0, buffer, std::min<size_t>(size, length));
    return static_cast<uint32_t>(std::min<size_t>(written, UINT32_MAX));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBSymbolContext
// ============================================================================

void lldb_symbol_context_destroy(lldb_symbol_context_t ctx)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (ctx) {
        delete static_cast<lldb::SBSymbolContext*>(ctx);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_symbol_context_is_valid(lldb_symbol_context_t ctx)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!ctx) return 0;
    lldb::SBSymbolContext* c = static_cast<lldb::SBSymbolContext*>(ctx);
    return (c->GetModule().IsValid() || c->GetFunction().IsValid()) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_module_t lldb_symbol_context_get_module(lldb_symbol_context_t ctx)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!ctx) return nullptr;

    lldb::SBSymbolContext* c = static_cast<lldb::SBSymbolContext*>(ctx);
    lldb::SBModule module = c->GetModule();

    if (!module.IsValid()) return nullptr;

    return static_cast<lldb_module_t>(new lldb::SBModule(module));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_symbol_context_get_function_name(lldb_symbol_context_t ctx)  LLDB_WRAPPER_NOEXCEPT {
    try {
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

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBType
// ============================================================================

void lldb_type_destroy(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (type) {
        delete static_cast<lldb::SBType*>(type);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_type_is_valid(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_type_get_name(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;
    return static_cast<lldb::SBType*>(type)->GetName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_type_get_display_type_name(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;
    return static_cast<lldb::SBType*>(type)->GetDisplayTypeName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_type_get_byte_size(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->GetByteSize();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_type_is_pointer_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsPointerType() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_type_is_reference_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsReferenceType() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_type_is_array_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsArrayType() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_type_is_vector_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsVectorType() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_type_is_typedef_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsTypedefType() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_type_is_function_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsFunctionType() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_type_is_polymorphic_class(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->IsPolymorphicClass() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_t lldb_type_get_pointer_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType ptr = t->GetPointerType();

    if (!ptr.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(ptr));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_t lldb_type_get_pointee_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType pointee = t->GetPointeeType();

    if (!pointee.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(pointee));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_t lldb_type_get_reference_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType ref = t->GetReferenceType();

    if (!ref.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(ref));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_t lldb_type_get_dereferenced_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType deref = t->GetDereferencedType();

    if (!deref.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(deref));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_t lldb_type_get_unqualified_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType unqual = t->GetUnqualifiedType();

    if (!unqual.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(unqual));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_t lldb_type_get_canonical_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType canon = t->GetCanonicalType();

    if (!canon.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(canon));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_t lldb_type_get_array_element_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;

    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    lldb::SBType elem = t->GetArrayElementType();

    if (!elem.IsValid()) return nullptr;

    return static_cast<lldb_type_t>(new lldb::SBType(elem));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_type_get_array_size(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    lldb::SBType* t = static_cast<lldb::SBType*>(type);
    // GetArraySize may not be available in older LLDB versions
    // Use byte_size / element_type_byte_size as a fallback
    if (!t->IsArrayType()) return 0;
    lldb::SBType elem = t->GetArrayElementType();
    if (!elem.IsValid() || elem.GetByteSize() == 0) return 0;
    return t->GetByteSize() / elem.GetByteSize();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_type_get_num_fields(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->GetNumberOfFields();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_type_get_num_direct_base_classes(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->GetNumberOfDirectBaseClasses();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_type_get_num_virtual_base_classes(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return 0;
    return static_cast<lldb::SBType*>(type)->GetNumberOfVirtualBaseClasses();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_member_t lldb_type_get_field_at_index(lldb_type_t type, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;
    lldb::SBTypeMember member = static_cast<lldb::SBType*>(type)->GetFieldAtIndex(index);
    if (!member.IsValid()) return nullptr;
    return static_cast<lldb_type_member_t>(new lldb::SBTypeMember(member));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_member_t lldb_type_get_direct_base_class_at_index(lldb_type_t type, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;
    lldb::SBTypeMember member =
        static_cast<lldb::SBType*>(type)->GetDirectBaseClassAtIndex(index);
    if (!member.IsValid()) return nullptr;
    return static_cast<lldb_type_member_t>(new lldb::SBTypeMember(member));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_member_t lldb_type_get_virtual_base_class_at_index(lldb_type_t type, uint32_t index)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return nullptr;
    lldb::SBTypeMember member =
        static_cast<lldb::SBType*>(type)->GetVirtualBaseClassAtIndex(index);
    if (!member.IsValid()) return nullptr;
    return static_cast<lldb_type_member_t>(new lldb::SBTypeMember(member));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_type_get_basic_type(lldb_type_t type)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!type) return static_cast<int>(lldb::eBasicTypeInvalid);

    // LLDB added eBasicTypeChar8 in the middle of this enum after the
    // minimum supported LLDB release. Keep the Ruby-facing values stable so
    // a Type returned by LLDB 14 and the same Type returned by current LLDB
    // are interpreted identically by BasicType.
    switch (static_cast<lldb::SBType*>(type)->GetBasicType()) {
      case lldb::eBasicTypeInvalid: return 0;
      case lldb::eBasicTypeVoid: return 1;
      case lldb::eBasicTypeChar: return 2;
      case lldb::eBasicTypeSignedChar: return 3;
      case lldb::eBasicTypeUnsignedChar: return 4;
      case lldb::eBasicTypeWChar: return 5;
      case lldb::eBasicTypeSignedWChar: return 6;
      case lldb::eBasicTypeUnsignedWChar: return 7;
      case lldb::eBasicTypeChar16: return 8;
      case lldb::eBasicTypeChar32: return 9;
      case lldb::eBasicTypeShort: return 10;
      case lldb::eBasicTypeUnsignedShort: return 11;
      case lldb::eBasicTypeInt: return 12;
      case lldb::eBasicTypeUnsignedInt: return 13;
      case lldb::eBasicTypeLong: return 14;
      case lldb::eBasicTypeUnsignedLong: return 15;
      case lldb::eBasicTypeLongLong: return 16;
      case lldb::eBasicTypeUnsignedLongLong: return 17;
      case lldb::eBasicTypeInt128: return 18;
      case lldb::eBasicTypeUnsignedInt128: return 19;
      case lldb::eBasicTypeBool: return 20;
      case lldb::eBasicTypeHalf: return 21;
      case lldb::eBasicTypeFloat: return 22;
      case lldb::eBasicTypeDouble: return 23;
      case lldb::eBasicTypeLongDouble: return 24;
      case lldb::eBasicTypeFloatComplex: return 25;
      case lldb::eBasicTypeDoubleComplex: return 26;
      case lldb::eBasicTypeLongDoubleComplex: return 27;
      case lldb::eBasicTypeObjCID: return 28;
      case lldb::eBasicTypeObjCClass: return 29;
      case lldb::eBasicTypeObjCSel: return 30;
      case lldb::eBasicTypeNullPtr: return 31;
#if LLDB_RUBY_HAVE_BASIC_TYPE_CHAR8
      case lldb::eBasicTypeChar8: return 32;
#endif
      default:
        // Preserve unknown future enum values instead of turning them into a
        // misleading known Ruby BasicType.
        return static_cast<int>(static_cast<lldb::SBType*>(type)->GetBasicType());
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBTypeMember
// ============================================================================

void lldb_type_member_destroy(lldb_type_member_t member)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (member) delete static_cast<lldb::SBTypeMember*>(member);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_type_member_is_valid(lldb_type_member_t member)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!member) return 0;
    return static_cast<lldb::SBTypeMember*>(member)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_type_member_get_name(lldb_type_member_t member)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!member) return nullptr;
    return static_cast<lldb::SBTypeMember*>(member)->GetName();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_type_t lldb_type_member_get_type(lldb_type_member_t member)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!member) return nullptr;
    lldb::SBType type = static_cast<lldb::SBTypeMember*>(member)->GetType();
    if (!type.IsValid()) return nullptr;
    return static_cast<lldb_type_t>(new lldb::SBType(type));

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_type_member_get_offset_in_bytes(lldb_type_member_t member)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!member) return 0;
    return static_cast<lldb::SBTypeMember*>(member)->GetOffsetInBytes();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint64_t lldb_type_member_get_offset_in_bits(lldb_type_member_t member)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!member) return 0;
    return static_cast<lldb::SBTypeMember*>(member)->GetOffsetInBits();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_type_member_get_bitfield_size_in_bits(lldb_type_member_t member)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!member) return 0;
    return static_cast<lldb::SBTypeMember*>(member)->GetBitfieldSizeInBits();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBWatchpoint
// ============================================================================

void lldb_watchpoint_destroy(lldb_watchpoint_t wp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (wp) {
        delete static_cast<lldb::SBWatchpoint*>(wp);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_watchpoint_is_valid(lldb_watchpoint_t wp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!wp) return 0;
    return static_cast<lldb::SBWatchpoint*>(wp)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int32_t lldb_watchpoint_get_id(lldb_watchpoint_t wp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!wp) return -1;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetID();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_watchpoint_is_enabled(lldb_watchpoint_t wp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!wp) return 0;
    return static_cast<lldb::SBWatchpoint*>(wp)->IsEnabled() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_watchpoint_set_enabled(lldb_watchpoint_t wp, int enabled)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!wp) return;
    static_cast<lldb::SBWatchpoint*>(wp)->SetEnabled(enabled != 0);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint32_t lldb_watchpoint_get_hit_count(lldb_watchpoint_t wp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!wp) return 0;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetHitCount();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

uint32_t lldb_watchpoint_get_ignore_count(lldb_watchpoint_t wp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!wp) return 0;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetIgnoreCount();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_watchpoint_set_ignore_count(lldb_watchpoint_t wp, uint32_t count)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!wp) return;
    static_cast<lldb::SBWatchpoint*>(wp)->SetIgnoreCount(count);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

const char* lldb_watchpoint_get_condition(lldb_watchpoint_t wp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!wp) return nullptr;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetCondition();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_watchpoint_set_condition(lldb_watchpoint_t wp, const char* condition)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!wp) return;
    static_cast<lldb::SBWatchpoint*>(wp)->SetCondition(condition);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

uint64_t lldb_watchpoint_get_watch_address(lldb_watchpoint_t wp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!wp) return LLDB_INVALID_ADDRESS;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetWatchAddress();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

size_t lldb_watchpoint_get_watch_size(lldb_watchpoint_t wp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!wp) return 0;
    return static_cast<lldb::SBWatchpoint*>(wp)->GetWatchSize();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

lldb_ruby_status_t lldb_watchpoint_is_watching_reads(lldb_watchpoint_t wp, int* result)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!result) return LLDB_RUBY_STATUS_INVALID_ARGUMENT;
    if (!wp) return LLDB_RUBY_STATUS_INVALID_HANDLE;
#if LLDB_RUBY_HAVE_WATCHPOINT_ACCESS_KIND
    *result = static_cast<lldb::SBWatchpoint*>(wp)->IsWatchingReads() ? 1 : 0;
    return LLDB_RUBY_STATUS_OK;
#else
    return LLDB_RUBY_STATUS_UNSUPPORTED;
#endif

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

lldb_ruby_status_t lldb_watchpoint_is_watching_writes(lldb_watchpoint_t wp, int* result)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!result) return LLDB_RUBY_STATUS_INVALID_ARGUMENT;
    if (!wp) return LLDB_RUBY_STATUS_INVALID_HANDLE;
#if LLDB_RUBY_HAVE_WATCHPOINT_ACCESS_KIND
    *result = static_cast<lldb::SBWatchpoint*>(wp)->IsWatchingWrites() ? 1 : 0;
    return LLDB_RUBY_STATUS_OK;
#else
    return LLDB_RUBY_STATUS_UNSUPPORTED;
#endif

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return LLDB_RUBY_STATUS_INTERNAL_ERROR;
    }
}

// ============================================================================
// SBCommandInterpreter
// ============================================================================

void lldb_command_interpreter_destroy(lldb_command_interpreter_t interp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (interp) {
        delete static_cast<lldb::SBCommandInterpreter*>(interp);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_command_interpreter_is_valid(lldb_command_interpreter_t interp)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!interp) return 0;
    return static_cast<lldb::SBCommandInterpreter*>(interp)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_command_interpreter_handle_command(lldb_command_interpreter_t interp,
                                             const char* command,
                                             lldb_command_return_object_t result,
                                             int add_to_history)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!interp || !command) return 0;

    lldb::SBCommandInterpreter* i = static_cast<lldb::SBCommandInterpreter*>(interp);
    lldb::SBCommandReturnObject* r = result ? static_cast<lldb::SBCommandReturnObject*>(result) : nullptr;
    lldb::SBCommandReturnObject local_result;

    lldb::ReturnStatus status = i->HandleCommand(command, r ? *r : local_result, add_to_history != 0);
    return static_cast<int>(status);

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_command_interpreter_command_exists(lldb_command_interpreter_t interp, const char* command)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!interp || !command) return 0;
    return static_cast<lldb::SBCommandInterpreter*>(interp)->CommandExists(command) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_command_interpreter_alias_exists(lldb_command_interpreter_t interp, const char* alias)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!interp || !alias) return 0;
    return static_cast<lldb::SBCommandInterpreter*>(interp)->AliasExists(alias) ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

// ============================================================================
// SBCommandReturnObject
// ============================================================================

lldb_command_return_object_t lldb_command_return_object_create(void)  LLDB_WRAPPER_NOEXCEPT {
    try {
    return static_cast<lldb_command_return_object_t>(new lldb::SBCommandReturnObject());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_command_return_object_destroy(lldb_command_return_object_t obj)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (obj) {
        delete static_cast<lldb::SBCommandReturnObject*>(obj);
    }

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

int lldb_command_return_object_is_valid(lldb_command_return_object_t obj)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!obj) return 0;
    return static_cast<lldb::SBCommandReturnObject*>(obj)->IsValid() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_command_return_object_get_output(lldb_command_return_object_t obj)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!obj) return nullptr;
    return static_cast<lldb::SBCommandReturnObject*>(obj)->GetOutput();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

const char* lldb_command_return_object_get_error(lldb_command_return_object_t obj)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!obj) return nullptr;
    return static_cast<lldb::SBCommandReturnObject*>(obj)->GetError();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_command_return_object_get_status(lldb_command_return_object_t obj)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!obj) return static_cast<int>(lldb::eReturnStatusInvalid);
    return static_cast<int>(static_cast<lldb::SBCommandReturnObject*>(obj)->GetStatus());

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_command_return_object_succeeded(lldb_command_return_object_t obj)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!obj) return 0;
    return static_cast<lldb::SBCommandReturnObject*>(obj)->Succeeded() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

int lldb_command_return_object_has_result(lldb_command_return_object_t obj)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!obj) return 0;
    return static_cast<lldb::SBCommandReturnObject*>(obj)->HasResult() ? 1 : 0;

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
        return {};
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
        return {};
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
        return {};
    }
}

void lldb_command_return_object_clear(lldb_command_return_object_t obj)  LLDB_WRAPPER_NOEXCEPT {
    try {
    if (!obj) return;
    static_cast<lldb::SBCommandReturnObject*>(obj)->Clear();

      } catch (const std::bad_alloc&) {
        wrapper_set_error_state("native allocation failed across the C ABI");
    } catch (const std::exception& exception) {
        wrapper_set_error_state(exception.what());
    } catch (...) {
        wrapper_set_error_state("unknown native exception across the C ABI");
    }
}

} // extern "C"
