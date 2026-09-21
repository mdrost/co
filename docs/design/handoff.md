# Handoff summary (for continuing in a new AI conversation)

Read this file, `registration.md`, `../concepts.rst` and `../../.github/copilot-instructions.md` before changing code.
`extensibility.md` is a historical record; where it conflicts with this file or `registration.md`, those win.

## Project

`co`: async runtime library, C++23, MSVC, CMake/Ninja, SHARED library. Public C API in `include/co.h` (+ per-kind headers
in `include/co/`), thin C++ wrapper in `include/co.hpp` (+ `.hpp` per kind). Tests use GoogleTest (`tests/`). Docs:
Sphinx + Doxygen (`docs/`), build with `.\.venv\Scripts\sphinx-build.exe -b html docs docs/_build/html`.
Nothing is released: change existing `_N` symbols in place instead of adding new versions.

## Naming and versioning (decided, implemented)

- A class (kind) name ends in its number: `co_timer_0`, `co_strand_0_executor_0`, `co_event_loop_0`. The class name,
  with its number, is the prefix of everything belonging to it: its methods (`co_timer_0_start_0`), its create
  functions (`co_instance_0_create_0`) and its create infos (`co_instance_0_create_info_0`); each of these then ends
  in its own version. Kinds without a handle (abstract kinds such as `fixed_thread_pool_0`) are numbered the same way
  (`co_fixed_thread_pool_0_create_info_0`). Bases (base kind, header a structure starts with, e.g.
  `co_in_structure_0` or `co_event_domain_0_create_info_0`) are fixed when a version is defined and documented, not
  encoded in the name; e.g. `co_event_loop_1` could be based on `co_event_domain_0` or on a `co_event_domain_1`.
- A service is named after the kind generation it serves, then its own number: `co_timer_0_service_0` is version 0 of
  the service of timers of generation 0 (`co_timer_0`), likewise `co_file_0_service_0` and `co_socket_0_service_0`.
  Service type constants mirror the name: `CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0`.
- Functions/methods: handle name (with its number) + function name + function version: `co_timer_0_start_0`,
  `co_instance_0_create_event_domain_0`, `co_event_domain_group_factory_0_get_provided_service_types_0`. The function
  version fixes parameter types and behavior; type numbers are not repeated in function names.
- Structures are named by what they describe: `create_info` = an object to create (`co_instance_0_create_info_0`,
  `co_fixed_thread_pool_0_create_info_0`), `import_info` = an existing
  object handed over (`co_win32_iocp_import_info_0`), `info` = any other description, typically an optional
  modifier of the object a next chain asks for (`co_thread_count_bounds_info_0`). Required parameters are members of
  the create info (`co_fixed_thread_pool_0_create_info_0::thread_count`); optional ones are separate `_info` structures in
  the next chain (`co_dynamic_thread_pool_0_create_info_0` has no members, the class chooses the bounds unless a
  `co_thread_count_bounds_info_0` is in the same chain). Like any next-chain entry, an `_info` narrows the class choice to
  classes handling it.
- Next-chain order: irrelevant between different structure types. A structure type appears at most once unless its
  documentation allows several and says whether their order matters (factory sources: registration order; debug
  callbacks: call order). A modifier entry requires the entry it modifies anywhere in the same chain; never require
  adjacency or a relative position.
  Structure type constants mirror the name: `CO_STRUCTURE_TYPE_0_INSTANCE_0_CREATE_INFO_0`.
- Create functions are like constructors: named after the created type with its number, then the function version,
  as destroy functions are (`co_instance_0_create_0`, `co_strand_0_executor_0_create_0`,
  `co_win32_platform_thread_pool_adapter_0_event_facility_0_create_0`). `X_0_create_1` creates the same kind
  differently; `X_1_create_0` creates a new kind. Create infos are named after the type with its number
  (`co_instance_0_create_info_0`) and are versioned independently of create functions (no 1:1 mapping): one create
  info may be taken by several create functions with different behavior (`X_0_create_0`, `X_0_create_1`), or by none
  (e.g. used only in next chains).
- Files (headers, sources, tests, API doc pages, Doxygen groups) are named `<module>_N`: a module is one implementation
  unit (an event domain group class with its factory, group, members, executors and create infos, or a standalone
  kind), and `N` its generation. A new implementation next to an old one is a new module generation in new files
  (`win32_platform_thread_pool_adapter_1.*`, e.g. on the newer Win32 thread pool API); new versions of kinds of an
  existing implementation are added in place. `co.h` and `co.hpp` are unversioned umbrella headers.
  `win32_thread_pool_0.*` will handle `PTP_POOL` handles.
- Kinds and structures owned by an implementation module are named `co_<module>_<N>_<kind>_<M>`: `N` is the module
  generation (as in its file name), `M` the version of the kind within the module, counted from 0 in each module:
  `co_win32_thread_pool_0_event_facility_0`/`_1` in `win32_thread_pool_0.h`, `co_win32_thread_pool_1_event_facility_0`
  in `win32_thread_pool_1.h`. The module number is a namespace, like the handle of a method, not a base. Test: if the
  structure would still make sense after replacing the implementation, it is shared and not named after a module
  (`co_event_loop_0`, `co_fixed_thread_pool_0_create_info_0`, `co_win32_iocp_import_info_0`); otherwise it is module-owned
  (`co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0`, `co_win32_iocp_0_event_loop_0`). A
  `co_strand_0_executor_0` in `strand_0.h`.
- Null `create_info` is a precondition (`@pre`), not checked.
- Destroy functions are unversioned. Vtables are unversioned and append-only (`co_executor_0_vtable`).
- Return types follow Vulkan: functions and methods that can fail at run time return `co_result_0`; those that cannot
  (destroy, getters of data the object already holds) return `void`. API misuse is a precondition, not a result.
- No `const void* create_info` parameters: create functions take typed create infos.

## Executors (decided, implemented)

- An executor object starts with public `co_executor_0_t { const co_executor_0_vtable* vtable; }`
  (C++ classes may derive from it). `co_executor_0_init(executor, &vtable)` stores the vtable.
- `co_executor_0_vtable { api_version; destroy; post_0; }`, only appended to; `api_version` (first member of the vtable
  of every base kind, no shared `co_vtable_0` base) is per class, not per object; the library calls a method only if the
  vtable's `api_version` has it. Derived kinds add their own vtable pointer after the base instead of extending it.
- Vtables of derived kinds (`co_event_loop_0_vtable`, `co_event_facility_0_vtable`) have no `api_version`: the base
  vtable's `api_version` versions every vtable of the object. A derived kind keeps its vtable pointer even while its
  vtable has no methods, since the object layout is ABI; such a vtable holds one `co_reserved_fn reserved` member
  (private, ignored by the library, any value) that the first method takes over.
- Executors are created by their class's own create function; no instance registration. `destroy` frees the object.
- Handler contract: on successful `post_0` the handler is invoked exactly once in the executor's context; on failure
  ownership stays with the caller; no destroy callbacks. Executors may be destroyed only once all accepted handlers ran
  (destructors assert). Strands behave like any executor.
- Examples: `src/strand_0.cpp` (built-in, uses `detail::new_object`/`delete_object` with allocation
  callbacks), `tests/test_executors.hpp` (`executor_vtable<T>`), `tests/executor_0_test.cpp` (plain C style).

## Other decisions already implemented

- `co_instance_0_create_timer_service_0` removed: timer services come only from event domains. The Win32 platform
  thread pool adapter event facility owns its timer service by composition (`std::optional` member).
- `co_service_0_destroy` is kept for now (do not remove).
- Tests use RAII guards: `auto X_guard = co_test::guard(X);` from `tests/test_guards.hpp`.
- In examples, do not deduplicate per-object configuration just because values coincide.

## Class registration (decided, NOT implemented yet)

Full record in `registration.md`. Key points:

- The registered unit is an **event domain group class**; it creates all members of a group (sharing resources, e.g.
  one IOCP for 4 event loops) and the executors targeting the group. Group creation is all-or-nothing.
- Four levels: entry point (function) -> factory (one per group class) -> group object -> members.
  - Each source (built-ins, each plugin) has one entry point. The instance calls it once during its creation and
    **owns** every factory it returns; no borrowed factories. The entry point gets **no `co_instance_0`** (no
    half-created instance): everything it may use comes in `co_event_domain_group_factory_0_entry_point_info_0`
    (allocation callbacks and debug callbacks in its chain, API version). Two-call count pattern. All-or-nothing;
    unsupported classes are simply not created.
  - Factory: `co_event_domain_group_factory_0_t { vtable }`, append-only vtable starting with `api_version`, with getters (member
    create info types, handled structure types, provided services, member-count range, rank as `size_t`),
    `create_event_domain_group_0` and `destroy`. Getters replace the probe: called once after the entry point,
    answers stored by the library. Getters return `void` and cannot fail; fallible work is done when the entry point
    creates the factory. Factories are destroyed after all groups, before plugins are unloaded.
  - Group object: `co_event_domain_group_0_t { vtable }` (vtable: `api_version`, `get_event_domain_0`, `create_executor_0`,
    `destroy`). Members have their own vtables (`run_0`, `stop_0`, `get_service_0`). Executors need no factory.
- Matching (in the library, on stored getter answers): member kind, required services, every chain entry in handled
  types (except allocation callbacks and hint wrappers), member-count range; then highest rank, then last registered.
  A class returning `CO_RESULT_0_ERROR_NOT_SUPPORTED` from `create_event_domain_group_0` declines values it cannot
  honor and the next candidate is tried; other failures are returned. Bounds such as `co_thread_count_bounds_info_0`
  are requirements (too few threads can deadlock), so classes decline rather than approximate them; fixed-pool
  classes may accept equal bounds.
  Built-in ranks are documented API.
- Factories report every structure type version they accept; no conversion between versions.
- Hints are per use: `co_hint_info_0 { ..., const co_in_structure_0* hint }` wraps a structure that would otherwise be a
  requirement. Wrappers are library-owned (skipped by matching, passed to the class unchanged); classes honor the hints
  they know and never fail because of one. Directly created classes accept them too (implemented).
- Executors take part in selection (decided, NOT implemented): factories will report executor create info types and
  their handled next-chain types; `create_event_domain_group_0` will get the group's executor create infos so it can
  decline early. Runtime bookkeeping moved out of the create infos (implemented, under review):
  `co_runtime_0_event_domain_group_info_0 { id, create_info }` and
  `co_runtime_0_executor_info_0 { id, event_domain_group_id, create_info }`; `co_executor_0_create_info_0` and
  `co_event_domain_group_0_create_info_0` no longer carry ids. Standalone executors are deferred.
- Allocation callbacks: never a separate parameter; the library guarantees exactly one entry in every chain handed to a
  class, inserting the instance callbacks by copying the head create info if needed. Public chain lookup helper
  `co_in_structure_0_find_0`. Directly called class create functions fall back to instance callbacks themselves.
- Services are verified after group creation (rollback on failure).
- Instance create info extensions: factory sources (`co_event_domain_group_factory_source_0`); plugin paths (eager,
  strict loading; modules stay loaded until `co_instance_0_destroy`). No built-ins switch: the built-in entry point is
  always called and currently registers nothing. Win32 classes are registered through the exported
  `co_win32_event_domain_group_factory_0_entry_point_0` (future separate library, likely loaded by default; any opt-out
  goes with module loading).
- Standalone domains: one generic `co_instance_0_create_event_domain_0` (hidden group of one) replaces
  `co_instance_0_create_event_loop_*`/`create_event_facility_*`; plus `co_event_domain_0_create_executor_*`.
- User runs every event loop they asked for; `stop_0` stops only its member.

## Current state

- Build succeeds. Tests: 32/35 pass. The 3 failures are event-facility creation tests: `co_instance_0_create_event_facility_0`
  in `src/event_facility_0.cpp` is a stub returning `CO_RESULT_0_ERROR_UNKNOWN`.
- Runtime (`src/runtime_0.cpp`), event loops (`src/event_loop_0.cpp`), services and timers are mostly stubs. Built-in
  event loop/facility classes still use the legacy `create_info_type` / `construct_0` shape, not called by anything.
- Docs build has a pre-existing Doxygen XML parse issue around a timer enum (unrelated).

## Next steps

1. Add to the public API: entry point info and function type, factory header and vtable, group object header and
   vtable, factory source instance extension, plugins instance extension,
   `co_in_structure_0_find_0`, `co_instance_0_create_event_domain_0`, `co_event_domain_0_create_executor_*`;
   remove the per-kind instance create functions; update C++ wrapper and docs.
2. Implement registration and matching in the instance.
3. Port the Win32 platform thread pool adapter event facility to a group class (fixes the 3 failing tests).
4. Implement the runtime on top of group classes, then the Win32 IOCP group class (shared port, N loops, executor).
