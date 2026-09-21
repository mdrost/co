# Extensibility layer design (decision record)

Summary of the design discussion for how `co` describes classes, stores per-object state, registers and selects
classes, loads modules and layers, and stays compatible across versions. Everything below is **decided** unless
marked otherwise. Code is illustrative: names follow the `_N` versioning scheme but are not final until implemented.

This file is not part of the Sphinx docs (Sphinx only reads `.rst`). It is meant as context for future work, including
new AI conversations: paste or reference it instead of re-answering the questions.

> **Superseded for executors.** Executors no longer use registered classes, `co_class_context_0` or library-owned
> blocks. An executor object starts with a public `co_executor_0_t` (`api_version` first, then a pointer to an
> append-only, unversioned `co_executor_0_vtable`), set by `co_executor_0_init()`. The class allocates and deallocates its
> objects and provides its own create function (e.g. `co_strand_0_executor_0_create_0()`). A derived kind adds its own
> vtable pointer after the base instead of extending the base vtable. The `api_version` field is provisional. The
> sections below still describe the other kinds.
>
> **Superseded for event domain registration and selection** by `registration.md` (group classes, matching,
> allocation callbacks, plugins, standalone event domains).

## Starting point (state of the code when this was written)

- Public C API in `include/co.h`, C++ wrapper in `include/co.hpp`; `co` is a SHARED library (MSVC, C++23, CMake/Ninja).
- Public dispatch functions (`co_executor_0_post_0`, `co_event_loop_0_run_0`, ...) are stubs.
- Built-in classes use an old pattern: `create_info_type`, `construct_0(self, create_info)`, assert-only destructor.
  `construct_0` is legacy and not a design constraint.
- `src/win32_platform_thread_pool_adapter_0_event_facility_0.cpp` probes kernel32 functions in every `construct_0`;
  this moves to module registration (see Capability checks). 
- `docs/_build` and `docs/_doxygen` still describe a rejected older design (`co_X_vtable`, `X_vtable_for<T>`,
  `convert_0`, `next_types`). Rebuild the docs to drop it.

## Terminology

- **Kind**: an interface, e.g. `co_executor_0`, `co_event_loop_0`. Each kind has at most one parent kind
  (event loop -> event domain, timer service -> service).
- **Class**: an implementation of a kind. Classes are leaf classes; no implementation inherits from another. Reuse is
  by composition and queries (e.g. `co_event_domain_0_get_service_0_0`).
- **Object**: an instance of a class, exposed as an opaque handle.
- **Module**: a DLL (or the built-in part of co.dll) that registers classes and/or layers.
- **Layer**: interceptor inserted between exported functions and classes (e.g. validation).

## 1. Object memory (library-owned block)

The library allocates each object as one block; the class declares size and alignment and never allocates its own
object.

```
[ object_header | state of layer 1 | ... | class state ]
  ^ handle points here                     ^ passed to class methods as `void* state`
```

- Class entry declares `state_size` and `state_alignment` (C++ helper: `sizeof(T)`, `alignof(T)`).
- Alignment must be a power of two (checked at registration). Allocators must honor any power-of-two alignment.
- Handle casts to the base kind are free: every kind handle points to the same header.
- Layers get per-object state inside the same block (`co_layer_0_get_object_state_0(handle, layer)`).
- State never moves (good for Rust pinning, intrusive structures).

### Construction and destruction

```c
co_result_0 (* construct_fn)(void* state, co_executor_0 self, const void* create_info,
							const co_class_context_0* context) CO_NOEXCEPT;
void (*destruct_fn)(void* state) CO_NOEXCEPT;
```

- One construct function. `self` may be stored during construct but must not be passed to co functions until
  construct returns. Creating child objects through `context->instance` is allowed.
- If construct fails, the object never existed: the library deallocates the block and does not call `destruct_fn`.
- Destructors cannot fail and only assert (objects are destroyed only when they have no pending work).
- Destroy functions stay **unversioned** and take no allocator: the header records the allocator.

`co_class_context_0` gives the class: the instance, the resolved allocator (for its own internal allocations, e.g. a
queue), `class_data` (see Registration), and a debug report function.

## 2. Allocation callbacks

```c
typedef struct co_allocation_callbacks_0 {
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;
	void* data;
	void* (*allocate_fn)(void* data, size_t size, size_t alignment) CO_NOEXCEPT;
	void  (*deallocate_fn)(void* data, void* memory, size_t size, size_t alignment) CO_NOEXCEPT;
} co_allocation_callbacks_0;
```

- In the instance create info chain: instance-wide default.
- In any object create info chain: override for that object. The library consumes this entry; classes never see it and
  it does not count against strict chain handling.
- The callbacks are copied; only `data` must outlive the object.
- One allocator per object: the class uses the same allocator for its internal allocations
  (C++: `co::allocator<T>`).
- Entities inherit their service's allocator unless their create info overrides it.

## 3. Class description

Class entry per kind, with one method table per interface level (own kind and each base kind). Example:

```c
typedef struct co_executor_0_methods_0 {
	co_result_0 (*post_0)(void* state, co_executor_0_handler_0 handler) CO_NOEXCEPT;
} co_executor_0_methods_0;

typedef struct co_executor_0_class_0 {
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;
	co_version_0 api_version;               // API version the class is written against
	const char* name;                        // for debug messages
	size_t state_size;
	size_t state_alignment;
	co_structure_type_0 create_info_type;    // concrete create info this class is created from
	const void* class_data;                  // passed to construct via context
	uintptr_t rank;
	// provided services, handled chain structure types, platform-backed flag (kinds that need them)
	co_result_0 (* construct_fn)(void*, co_executor_0, const void*, const co_class_context_0*) CO_NOEXCEPT;
	void (*destruct_fn)(void*) CO_NOEXCEPT;
	const co_executor_0_methods_0* methods_0;
	// methods added in later minor versions are appended here, read only if api_version is new enough
} co_executor_0_class_0;

typedef struct co_event_loop_0_class_0 {
	/* same header fields */
	const co_event_domain_0_methods_0* event_domain_methods_0;   // get_service_0_0
	const co_event_loop_0_methods_0* event_loop_methods_0;       // run_0, stop_0
} co_event_loop_0_class_0;
```

- Methods receive `void* state` directly (fast path), not the handle.
- At registration the library copies the class entry and builds a complete internal table (defaults filled in,
  offsets computed). Class entries do not need to outlive registration (helps C#/Rust).

### Required vs optional methods

- Methods are **required** by default; registration fails if one is missing.
- A method is optional only with a good reason, and its header documents the default: either a fixed result
  (e.g. `get_service_0_0` -> `CO_RESULT_0_ERROR_SERVICE_NOT_FOUND`) or an implementation on top of an older method.
- A method added in a minor version must be optional with a default. Methods that cannot have a default need a new
  kind generation or a breaking version.
- A class-implemented method's contract never changes; a changed contract is a new method version (`post_1`).

## 4. Versioning and compatibility

- Versions are epoch.major.minor.patch; changing epoch or major is breaking (while major is 0, the first nonzero
  component is the breaking one; see `co.h`).
- The **instance** checks compatibility, not the class: applications, classes, layers and modules each declare an
  `api_version`, and the instance accepts it under the same epoch.semver rule it applies to applications.
- Old class in new library: missing later methods get their defaults; the class is never given chain entries or enum
  values newer than its `api_version`.
- New class in old library: rejected if its `api_version` is newer than the library.
- The application's `api_version` controls only library-implemented behavior.

## 5. Dispatch and layers

Every exported function (including functions specific to built-in classes, e.g.
`co_win32_iocp_0_event_loop_0_get_iocp_0_0`) goes through a per-instance dispatch table. No exported function calls a
class directly.

```c
typedef struct co_dispatch_0 {           // one slot per function co exports; appended in minor versions
	co_result_0 (*executor_0_post_0)(const struct co_dispatch_0* next, co_executor_0, co_executor_0_handler_0);
	void        (*executor_0_destroy)(const struct co_dispatch_0* next, co_executor_0);
	co_result_0 (*win32_iocp_0_event_loop_0_get_iocp_0_0)(const struct co_dispatch_0* next, co_win32_iocp_0_event_loop_0, void**);
	// ...
} co_dispatch_0;

co_result_0 co_executor_0_post_0(co_executor_0 executor, co_executor_0_handler_0 handler)
{
	const co_dispatch_0* d = header_of(executor)->dispatch;     // top layer, or terminator when no layers
	return d->executor_0_post_0(d->next, executor, handler);
}
// terminator: the only place that reaches the class
static co_result_0 term_post_0(const co_dispatch_0*, co_executor_0 executor, co_executor_0_handler_0 handler)
{
	object_header* h = header_of(executor);
	return h->cls->methods.post_0(state_of(h), handler);
}
```

- Layers see handles (they validate what the application passed); classes see state.
- Layers fill only the slots they intercept; the rest pass through (also for slots added after the layer's
  `api_version`).
- Layers intercept, they do **not** wrap: the handle stays the real object, so casting to a concrete class handle keeps
  working.
- Built-in classes expose class-specific functions as extra method tables (generated by the C++ helper).
- Plugin classes have no class-specific functions (they are reachable only via generic selection), so the set of
  functions co knows is closed and fully interceptable.
- Classes calling each other use the public API (so layers see those calls too).
- No inline dispatch from headers.
- The layer interface is designed as public from the start, but is not published until stable.

## 6. Modules, plugins and layers (Vulkan-like loading)

Everything is a module described by a JSON manifest (parsed with **nlohmann/json**, private dependency of co.dll,
fetched with `FetchContent` + `FIND_PACKAGE_ARGS` like googletest, non-throwing parse).

```json
{
  "file_format_version": "0.0.0",
  "module": {
	"name": "co.validation",
	"library_path": ".\\co_validation.dll",
	"api_version": "0.0.0.0",
	"implementation_version": "1",
	"description": "Validation of co API usage",
	"provides": ["layer"]
  }
}
```

- Manifests are read without loading DLLs; `api_version` is checked before `LoadLibrary`.
- Module directories are given by the application only:

  ```c
  typedef struct co_plugin_directory_0 {    // instance create info chain entry; may appear several times
	  co_structure_type_0 structure_type;
	  const co_in_structure_0* next_structure;
	  const char* path;                      // UTF-8
  } co_plugin_directory_0;
  ```

  No entry means no discovery.
- Modules are loaded at instance creation and unloaded at instance destruction.
- Entry point:

  ```c
  CO_MODULE_EXPORT co_result_0 co_module_0_register_0(const co_module_0_host_0* host, co_module_0_registrar_0* registrar);
  ```

  The host table carries the library version, debug report, register functions (classes, layers) and the co functions
  a module may call. Modules do not need to link co.lib.
- Class modules: always loaded if found; their classes join generic selection only.
- Layer modules: loaded only when enabled:
  - by name in a `co_layers_0` instance create info chain entry (order = call order); a missing name fails instance
	creation with `CO_RESULT_0_ERROR_NOT_SUPPORTED`;
  - or by the `CO_ENABLE_LAYERS` environment variable (`;`-separated), which only matches layers found in the
	application's directories, can only add layers, sits closest to the application, and only warns if a name is
	missing.
  - No implicit layers. No way for the application to block `CO_ENABLE_LAYERS`. Each enabled layer is reported
	(INFO) with its source and can be queried (`co_instance_0_get_enabled_layers_0_0`).
- `co_enumerate_modules_0(...)` lists modules from manifests without loading them (e.g. enable validation only if
  installed).
- Validation is a separate module/DLL built in this repository.
- Ties between plugin classes of equal rank are broken deterministically (e.g. by module file name) and reported.

## 7. Registration and selection

- Built-in classes are registered by the instance (composition in `src/modules_0.cpp`), then application classes
  from `co_*_registration_0` entries of the instance create info chain, then module classes.
- A **concrete create info** always selects its class directly, bypassing rank and order. Registering two classes for
  the same concrete create info type fails.
- A **generic create info** (e.g. `co_event_loop_0_create_info_0`) selects among classes that provide the required
  services and handle every chain entry; then highest rank; then last registered.
- **Strict chain entries**: a class or service lists the chain structure types it handles; any other entry is
  rejected with `CO_RESULT_0_ERROR_NOT_SUPPORTED` (library-consumed entries such as allocation callbacks excepted).
  May later evolve to per-type "requirement vs hint".
- If construction fails during generic selection with `CO_RESULT_0_ERROR_NOT_SUPPORTED`, the class declined values
  it cannot honor and the next candidate is tried; any other error is returned.
- IDs (structure types, service types) stay addresses of exported variables; applications never name plugin-defined
  types, so no UUIDs are needed.

### Capability checks

- Done once, at module registration (e.g. `GetProcAddress` probes). A class whose checks fail is not registered
  (INFO debug message). Probed data is shared with objects through `class_data`.
- Construct only reports genuine resource failures.
- A per-creation `probe_fn` in selection may be added later if a real case needs it.

## 8. Services and entities

- Entities (e.g. timers) are created only through their service; their classes are private to the service and are
  listed in the service's class entry, checked at registration.
- Timer hints are passed per call (for now):

  ```c
  typedef struct co_timer_0_create_info_0 {
	  co_structure_type_0 structure_type;
	  const co_in_structure_0* next_structure;    // hints (e.g. high resolution), allocator override
  } co_timer_0_create_info_0;

  CO_API co_result_0 co_timer_0_service_0_create_timer_0_0(co_timer_0_service_0 timer_service,
	  const co_timer_0_create_info_0* create_info, co_timer_0* timer);
  ```

- The service's create-timer method picks a private timer class from the hints and asks the library (through a
  factory) to create it; the library allocates the block and calls the timer class's construct with the service's
  state, which the timer stores.

## 9. Errors and debug callbacks

- `co_result_0` stays slim (no system error codes).
- Details go to debug callbacks:

  ```c
  typedef struct co_debug_message_0 {
	  co_debug_severity_0 severity;
	  co_result_0 result;
	  const char* message;     // UTF-8, valid during the callback
	  const void* object;      // handle concerned, or NULL
	  intptr_t system_error;    // GetLastError()/errno, 0 if none
  } co_debug_message_0;

  typedef struct co_debug_callback_0 {     // instance create info chain entry; several allowed
	  co_structure_type_0 structure_type;
	  const co_in_structure_0* next_structure;
	  co_debug_severity_0 min_severity;
	  void* data;
	  void (*callback_fn)(void* data, const co_debug_message_0* message) CO_NOEXCEPT;
  } co_debug_callback_0;
  ```

- Callbacks are given only in the instance create info (they also receive messages during instance creation and
  module loading). Adding/removing later may come in a minor version.
- Null `create_info` and similar API misuse are preconditions, checked by the validation layer, not by co.

## 10. C++ helper layer and other languages

- Header-only helper (`co/module.hpp`), depends only on the C ABI, versioned `inline namespace` to avoid ODR clashes.
- `register_X_class_0<T>(...)` builds the class entry and method tables from `T`'s members; thunks do placement new,
  call the constructor, and run the destructor.
- Exceptions are mapped to `co_result_0` (`bad_alloc` -> `OUT_OF_MEMORY`, `system_error` -> `SYSTEM`, else
  `UNKNOWN`) and reported through debug callbacks with `what()` and the error code. In `noexcept` paths (destructor,
  handler invocation) an exception terminates.
- `CO_DEFINE_MODULE_0(...)` generates the module entry point.
- Other languages: plain structs, fixed-width integers, no bitfields, explicit calling convention; Rust keeps state as
  `MaybeUninit<T>` in the library block, callbacks must not unwind; C# uses `[UnmanagedCallersOnly]` and a GC handle
  as state.

Example class with the helper:

```cpp
class my_executor final
{
public:
	using create_info_type = my_executor_create_info_0;

	my_executor(co::executor_0 self, const create_info_type& ci, const co::class_context_0& ctx)
		: m_functions(*static_cast<const my_functions*>(ctx.class_data))
		, m_queue(ctx.allocator())
	{}

	~my_executor() { assert(m_queue.empty()); }

	co::result_0 post_0(co::executor_0_handler_0 handler) noexcept;

private:
	const my_functions& m_functions;
	std::deque<co::executor_0_handler_0, co::allocator<co::executor_0_handler_0>> m_queue;
};

CO_DEFINE_MODULE_0(registrar)
{
	static my_functions functions;
	if (!probe(functions))
		return registrar.report_info("my_executor: API not available"), co::result_0::success;
	return registrar.register_executor_class_0<my_executor>({.rank = 0, .class_data = &functions});
}
```

## Suggested implementation order

1. Object header, library-owned block, allocation callbacks, debug callbacks.
2. Internal class table and registration of one built-in executor class (strand), through the C++ helper.
3. Per-instance dispatch table with terminator; implement the stub exported functions.
4. Event domain / event loop / event facility kinds with base-kind method tables; generic selection.
5. Services and private timer classes with `co_timer_0_create_info_0`.
6. Move capability probing of the Win32 thread pool classes to registration (`class_data`).
7. Manifest reading (nlohmann/json), `co_enumerate_modules_0`, module loading.
8. Layer interface and the validation module.
9. Update `docs/concepts.rst` and rebuild docs to remove the stale vtable design.

## Implementation status

- Done (executor milestone): steps 1 and 2. `co_instance_0_create_0` allocates the instance in a single block (through
  `co_allocation_callbacks_0` when given) holding copies of the debug callbacks and the executor class entries; it checks
  versions, rejects unknown chain entries and duplicate create info types. Executors are created in library-owned
  blocks (`src/instance_0.hpp`: `object_header` + `co_executor_0_t`), honour per-object allocation callbacks and the
  strict next-chain rule. Built-in classes are composed in `src/modules_0.cpp`; the strand uses
  `co::make_executor_0_class_0` from `<co/class_0.hpp>`, as do the test executors.
- Deferred: step 3. Exported executor functions call the class methods directly; the per-instance dispatch table is
  introduced with layers (step 8) without changing the public ABI.
