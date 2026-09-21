# Class registration and selection (decision record)

Design decisions for registering event domain classes in an instance and resolving generic create infos into concrete
classes. Everything below is **decided** unless listed under *Deferred*. Names follow the versioning scheme of
`docs/concepts.rst` but are not final until implemented.

This file is not part of the Sphinx docs. It supersedes the class-entry / `convert_0` / `next_types` parts of
`extensibility.md` for event domains.

## Unit of registration: event domain group classes

- The registered class is an **event domain group class**. A group class creates all N members of a group at once,
  so it can share resources between them: e.g. the `win32_iocp_0` group class creates one completion port and N event
  loops waiting on it.
- The group object also creates the executors that target the group (`create_executor_0` in its vtable, e.g. an
  executor posting with `PostQueuedCompletionStatus`). A group object without that method rejects executors with
  `CO_RESULT_0_ERROR_NOT_SUPPORTED`.
- `create_event_domain_group_0` of a factory is all-or-nothing: on failure it has created nothing. The runtime
  destroys the groups it already created in reverse order and fails.

### Entry points, factories, groups and members

Four levels, each an object or function with a clear owner:

1. **Entry point** (function): the only way factories come into existence. It describes nothing about the classes; it
   only creates factories, so its signature rarely needs to change.
2. **Factory** (`co_event_domain_group_factory_0`): one per group class. An object like an executor: it starts with a
   pointer to an append-only vtable, whose first member is `api_version`. It answers the class-level questions through getters and creates
   groups. It may hold state shared by all its groups (e.g. the resolved kernel32 functions of the Win32 thread pool
   class). New class-level questions are appended methods with documented defaults for older factories; no new struct
   versions are needed.
3. **Group object** (`co_event_domain_group_0`), created by a factory; same header model (append-only vtable starting
   with `api_version`: `get_event_domain_0`, `create_executor_0`, `destroy`).
4. **Members** (event loops, event facilities), created by their group; own vtables (`run_0`, `stop_0`,
   `get_service_0`, ...).

Executors need no factory: users create them directly; group classes need one only because the library selects and
creates them.

Illustrative shape:

```c
// Everything the entry point may use; it gets no co_instance_0 (the instance is still being created).
typedef struct co_event_domain_group_factory_0_entry_point_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	// Holds exactly one co_allocation_callbacks_0 (those of the instance) and the co_debug_callback_0 entries of
	// the instance; factories may keep copies, valid until they are destroyed.
	const co_in_structure_0* next_structure;

	co_version_0 api_version;                        // API version of the instance (the application)
} co_event_domain_group_factory_0_entry_point_info_0;

// Creates the factories of a source. Called once per source during instance creation. Two-call pattern: with
// factories == NULL it stores the count in *factory_count; otherwise it creates up to *factory_count factories.
typedef co_result_0 (* co_event_domain_group_factory_0_entry_point_0_fn)(void* data,
	const co_event_domain_group_factory_0_entry_point_info_0* info,
	co_event_domain_group_factory_0* factories, size_t* factory_count) CO_NOEXCEPT;

struct co_event_domain_group_factory_0_t
{
	const co_event_domain_group_factory_0_vtable* vtable;
};

// Factory vtable (append-only), illustrative:
//   destroy
//   api_version                             API version of the headers the class is compiled against
//   get_member_create_info_structure_types_0 structure types of the create infos of its members (generic and concrete)
//   get_handled_structure_types_0           next-chain create infos, import infos, extensions it handles on this machine (every version)
//   get_provided_service_types_0            services every member provides on this machine
//   get_member_count_range_0                e.g. a platform thread pool: 1..1
//   get_rank_0                              size_t
//   create_event_domain_group_0(self, create_info, group)   returns co_result_0; the getters return void

struct co_event_domain_group_0_t
{
	const co_event_domain_group_0_vtable* vtable;    // api_version, get_event_domain_0, create_executor_0, destroy
};
```

- **Getters are called once**, right after the entry point, and the library stores the answers. Selection uses the
  stored data, is predictable and needs no locking; the set of classes and their capabilities are fixed for the life of
  the instance. Getters report what the machine actually supports (replacing a separate probe): e.g. a timer service only
  on newer Windows, or an extension honored only by newer thread pool APIs. Returned arrays stay valid until the factory
  is destroyed.
- **Getters cannot fail** (they return `void`), as Vulkan queries such as `vkGetPhysicalDeviceProperties`: they only
  return what the factory already holds. Work that can fail (probing the system, allocating the returned arrays) is done
  when the entry point creates the factory, which returns `co_result_0`; a factory that cannot answer is not created.
  Only methods that can fail at run time (`create_event_domain_group_0`) return `co_result_0`.
- **Matching stays in the library** (see below), so rejections can be explained and ranking is consistent. An optional
  factory method that inspects the chain itself (`accepts_0`) may be appended later if a real case needs it.
- **Entry points get no `co_instance_0`**: the instance is still being created. Everything they may use is passed in
  the info structure (allocation callbacks, debug callbacks, API version). `create_event_domain_group_0` gets its
  allocation callbacks in the member create info chain (see *Library-owned entries*).

## Matching

A factory is a candidate for a generic create info when (using the stored getter answers):

1. its member create info type is the structure type of the generic create info,
2. its members provide every required service,
3. its handled types contain the structure type of every chain entry, except library-owned entries (allocation
   callbacks and hint wrappers),
4. its member-count range admits the requested count,
5. in a runtime, when executors target the group: it can create executors from each of their create infos (see
   *Executors in selection*).

Of the candidates, the highest rank wins; on a tie, the last registered factory wins (built-ins are registered first,
then plugins in the order of the create info). Built-in ranks are part of the documented API, since they decide when
nothing else does. If the winner's `create_event_domain_group_0` returns `CO_RESULT_0_ERROR_NOT_SUPPORTED`, it
declines values it cannot honor (e.g. `co_thread_count_bounds_info_0` a class cannot keep: silently running fewer
threads could deadlock handlers), and the next candidate in the same order is tried. Any other failure is returned.

- **Versions:** a factory reports every version of every structure type it accepts. There is no conversion between
  versions: a factory that only reports `..._create_info_0` is not a candidate for `..._create_info_1`. Rejections
  are reported through debug callbacks.
- **Optional hints:** wrapped in `co_hint_info_0` (see *Hints*); they do not affect matching, and a class that knows
  them may honor them.
- **Concrete create infos** (e.g. `co_win32_iocp_0_event_loop_0_create_info_0`) start with
  `co_event_domain_0_create_info_0` and are passed directly; only their class lists them as a member create info type.
- **Next-chain structures** (e.g. `co_fixed_thread_pool_0_create_info_0`, `co_win32_iocp_import_info_0`) are `co_in_structure_0`
  entries in the next chain of a generic create info, optionally carrying parameters; any class may list them as handled.

## Hints

A requirement and a preference may use the same structure (e.g. an executor priority), so whether an entry is a hint
is decided per use, not per structure type:

```c
typedef struct co_hint_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;          // CO_STRUCTURE_TYPE_0_HINT_INFO_0
	const co_in_structure_0* next_structure;

	const co_in_structure_0* hint;               // the hinted structure; its own next_structure is not read
} co_hint_info_0;
```

- A structure put directly in a chain is a requirement: every candidate must handle its type and may decline its values
  with `CO_RESULT_0_ERROR_NOT_SUPPORTED`. The same structure wrapped in `co_hint_info_0` is a hint.
- Hint wrappers are library-owned entries: classes do not list `CO_STRUCTURE_TYPE_0_HINT_INFO_0` as handled, matching
  skips them, and they reach the class unchanged in its chain. A class honors the hints whose structure types it knows
  and ignores the others; it never fails because of a hint.
- A chain may hold several hint wrappers, in any order; at most one per hinted structure type.
- Classes created directly by the user (e.g. strand executors) accept hint wrappers in their chains as well.
- Ignored hints may later be reported through debug callbacks (deferred with other diagnostics).

## Executors in selection

- In a runtime, executors target an event domain group, and the group object creates them. Whether a class can create
  the requested executors is part of selection, so a class that would reject an executor is not chosen over one that
  accepts it.
- Factories gain getters, illustrative: `get_executor_create_info_structure_types_0` (executor create infos its groups
  accept, every version; none by default, meaning the group creates no executors) and
  `get_handled_executor_structure_types_0` (next-chain entries of those create infos). Matching applies the same rules
  as for member create infos, hint wrappers included.
- `create_event_domain_group_0` receives the executor create infos of the group so that a class may decline values it
  cannot honor before anything is created; the group then creates the executors with `create_executor_0`.

## Runtime entries

- Runtime bookkeeping is split from the create infos, so that the same executor or group create info can be used
  outside a runtime (e.g. on a standalone event domain). Create infos say only what to create; runtime entries say
  where it lives in the runtime (implemented):

```c
typedef struct co_runtime_0_event_domain_group_info_0 CO_FINAL
{
	size_t id;
	const co_event_domain_group_0_create_info_0* create_info;
} co_runtime_0_event_domain_group_info_0;

typedef struct co_runtime_0_executor_info_0 CO_FINAL
{
	size_t id;
	size_t event_domain_group_id;
	const co_executor_0_create_info_0* create_info;    // concrete kinds and hints in its next chain
} co_runtime_0_executor_info_0;
```

- Entries are plain records, not chain structures: they have no structure type and no next chain. Runtime-wide
  extensions go in the next chain of `co_runtime_0_create_info_0`.
- Standalone executors (`co_event_domain_0_create_executor_*`) are deferred.

## Library-owned entries: allocation callbacks

- Allocation callbacks do not take part in matching and are never passed to classes as a separate parameter.
- The library passes the member create info unchanged and gives the class exactly one `co_allocation_callbacks_0` in
  the next chain of the group create info: the one from the user's chain, or the instance callbacks when it has none.
- Classes find chain entries with a public helper, e.g. `co_in_structure_0_find_0(chain, structure_type)`, never by
  position.
- Class-owned create functions called directly by the user (e.g. `co_strand_0_executor_0_create_0`) fall back to the
  instance callbacks themselves.

## Capability checks and service verification

- Factory getters report what the machine supports; there is no separate probe. A factory whose members provide nothing
  usable and handle nothing is dropped with a debug message. A class whose required OS functions are missing is not
  created by its entry point at all (see *Registration sources*).
- At instance creation, getter answers are checked for consistency (no null or duplicate types, min <= max members).
- After `create_event_domain_group_0`, the runtime checks that every member provides every required service;
  otherwise it destroys the group, reports the class through debug callbacks and fails.

## Registration sources

Every source provides an entry point; the instance calls each once during its creation and **owns every factory** it
returns. Factories are destroyed in `co_instance_0_destroy` (through their vtable `destroy`) after all groups and before
plugin modules are unloaded. There are no application-owned (borrowed) factories.

- An entry point is all-or-nothing: on failure it has created no factory. A failing entry point fails instance
  creation; the instance destroys the factories of earlier sources in reverse order. A class that is merely
  unsupported on this machine is not a failure: the entry point does not create its factory (INFO debug message).

- **Built-ins:** an internal entry point of the library (not public), always called first. It currently registers
  nothing: the Win32 classes have their own exported entry point (`co_win32_event_domain_group_factory_0_entry_point_0`,
  `co/win32.h`) that applications and tests register with a `co_event_domain_group_factory_source_0`, and they are
  expected to move to a separate library. There is no switch to turn built-ins off; once such a library is loaded by
  default, opting out belongs to module loading, not to an instance create info entry.
- **Plugins:** an extension listing plugin paths. Plugins are loaded eagerly at instance creation; the entry point is
  exported by the module and named by its manifest, which also gives the module and its `api_version`.
  - Loading is strict: a module that fails to load, or whose `api_version` is incompatible, fails instance creation.
  - Modules stay loaded until `co_instance_0_destroy`, so structure types, service types and vtables (addresses inside
	the module) stay valid.
  - The set of classes is fixed after instance creation; lookups need no locking.

## Standalone event domains

- `co_instance_0_create_event_loop_*` and `co_instance_0_create_event_facility_*` are replaced by one generic
  `co_instance_0_create_event_domain_0(instance, const co_in_structure_0* create_info, co_event_domain_0*)`. The
  structure type of `create_info` (a generic event loop or event facility create info) selects the member kind. It
  creates a hidden group of one, destroyed by `co_event_domain_0_destroy`. C++ overloads restore type safety.
- `co_event_domain_0_create_executor_*` creates an executor on a standalone domain through the hidden group object's
  `create_executor_0`, so event loops are usable without a runtime.

## Running and stopping

- The user is responsible for running every event loop they asked for. A group class may distribute work over its
  members (e.g. a shared completion port); handlers routed to a member nobody runs are never invoked.
- `stop_0` stops only the member it is called on; a group of 4 event loops needs 4 `stop_0` calls.

## Deferred

- 1: tie-breaking between plugins of equal rank beyond registration order.
- 5/9: diagnostics for rejected chains and reporting/querying the chosen class.
- 8: rank per next-chain structure instead of one rank per class.
- 10: the same class registered twice.
- 15: registering after instance creation.
- Application entry points: an instance extension listing in-process entry points (with `data`), so an application
  can add classes without a plugin module. The instance would still own the factories they create.
- 18-20: executors on groups without `create_executor_0`, several executors per group, executors over several groups.
- 23-24: concurrent `run_0`, completions arriving on another member of a shared-port group.
- 25: process-wide singletons (member-count range, no global state in built-ins, several instances per process).
- 26-27: sharing resources between groups, retrying with fewer groups.
- 28-30: service scope (per member vs per group), binding objects to their service, service types defined by plugins.
- 34-36: re-entrancy from `create_event_domain_group_0` (e.g. creating a runtime), rank abuse.
