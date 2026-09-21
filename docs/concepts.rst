Concepts
========

Instance
--------

An instance is the root of every object of the library. It is created with an API version, the version of the
headers the application is compiled against (``CO_API_VERSION_0``), and behaves as specified by that version.

Kinds, classes and objects
--------------------------

Executors, event loops, event facilities, services and timers are *objects*. Each object is of a *kind*, for example
``co_event_loop_0``, and of a *class* of that kind, which implements the methods of the kind.

Classes are registered in the instance when it is created: the built-in classes of the library, then those of each
``co_event_domain_group_factory_source_0`` in the next chain of the instance create info. The Win32 classes are
registered by adding a source whose entry point is ``co_win32_event_domain_group_factory_0_entry_point_0()``
(``co/win32.h``).

Executors are the exception: an executor class needs no registration. Every executor object starts with a
``co_executor_0_t`` (in C++ the class may derive from it) holding a pointer to the ``co_executor_0_vtable`` of the
class, set by ``co_executor_0_init()``. The vtable starts with the ``api_version`` of the headers the class is
compiled against. ``co_executor_0_post_0()`` and ``co_executor_0_destroy()`` call the methods of that vtable. Methods
are only ever appended to the vtable; the library calls a method only through vtables whose ``api_version`` is recent
enough to have it. A kind derived from an executor adds its own vtable pointer after the ``co_executor_0_t`` instead of
extending ``co_executor_0_vtable``. The vtables of derived kinds have no ``api_version``: the one of the base vtable
(e.g. ``co_event_domain_0_vtable`` for ``co_event_loop_0_vtable``) versions every vtable of the object.

Creating and destroying objects
-------------------------------

Most objects are created from a create info whose first member is its structure type. The instance looks up the class
registered for that structure type, allocates the object, and constructs it with the class from the create info. Some
objects are created by other objects instead: timers are created by their timer service, which offers
one or more timer classes. Executors are created by their class, for example with ``co_strand_0_executor_0_create_0()``
for a strand; an application class allocates and initializes its executors itself.

Input structures (create infos, import infos, registrations and extensions) all start with a structure type and a ``next_structure``
pointer, the members of ``co_in_structure_0``, so that the entries of a next chain may be walked through it and told
apart by their structure type. ``next_structure`` is a ``const co_in_structure_0*`` (``const co::in_structure_0*`` in C++); link
a structure into a chain with ``CO_IN_STRUCTURE_0_CAST`` (``co::in_structure_0_cast`` in C++), which fails to compile
unless the structure starts with that header::

    platform.next_structure = CO_IN_STRUCTURE_0_CAST(&limits);

The order of the entries of a next chain does not matter between entries of different structure types. A structure
type appears at most once in a chain unless its documentation allows several, and that documentation says whether
their order matters: ``co_event_domain_group_factory_source_0`` entries, for example, are registered in chain order. An
entry that modifies another one needs that entry somewhere in the same chain, not at a particular position.

An entry of a next chain is a requirement: the class must handle it and may decline its values. To state a preference
instead, wrap the structure in a ``co_hint_info_0`` (``co::hint_info_0`` in C++). A hint does not affect which class is
chosen; the class honors it when it knows its structure type and ignores it otherwise, and never fails because of it.
Any next chain may hold hints::

    hint.hint = CO_IN_STRUCTURE_0_CAST(&preference);
    create_info.next_structure = CO_IN_STRUCTURE_0_CAST(&hint);

Every event domain create info starts with the members of ``co_event_domain_0_create_info_0``: the structure type, the
``next_structure`` pointer, and the services the event domain must provide. This works the same way as
``co_in_structure_0`` does for input structures. Pass such a create info to ``co_instance_0_create_event_domain_0()``
with ``CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST`` (``co::event_domain_0_create_info_0_cast`` in C++), which fails to
compile unless the create info starts with that sequence::

    co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&facility), &event_domain);

There are two ways to ask for an event domain:

- **Name a class.** Each class has its own create info, for example
  ``co_win32_iocp_adapter_0_event_loop_0_create_info_0``, ``co_win32_iocp_0_event_loop_0_create_info_0`` or
  ``co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0``.
- **State requirements.** The generic ``co_event_loop_0_create_info_0`` and ``co_event_facility_0_create_info_0`` ask
  for any event loop or event facility. Structures in their next chain narrow the choice:

  - ``co_platform_thread_pool_0_create_info_0`` asks for an event facility of a thread pool managed by the
    runtime the application runs on: the operating system or an application framework such as Qt or GTK, for example
    the default process thread pool on Windows.
  - ``co_fixed_thread_pool_0_create_info_0`` and ``co_dynamic_thread_pool_0_create_info_0`` ask for a thread
    pool that the event facility owns. At most one of the three thread pool create infos may be in a next chain.
  - ``co_thread_count_bounds_info_0`` in a next chain that also holds a ``co_dynamic_thread_pool_0_create_info_0``
    bounds the number of its
    threads; without it the class chooses. The bounds are a requirement: a pool running fewer threads than blocking
    handlers need deadlocks.
  - ``co_win32_iocp_import_info_0`` asks for an event domain running on an I/O completion port the application owns.

In both cases the instance chooses the class the same way. Event domain group classes are registered with factories. A
factory lists the structure types of the member create infos the class accepts and the structure types of the
next-chain entries it handles. It also gives the services its members provide and a rank. A class is a candidate when
it accepts the structure type of the create info, provides the required services, and handles every entry of the next
chain other than allocation callbacks and hints. The candidates are tried by highest rank first. Among those of equal rank, the last registered one is tried
first, so classes registered after the built-in ones take precedence. A class handles structure types, not values: when
it cannot honor the values of the create infos, for example thread count bounds, it declines with
``CO_RESULT_0_ERROR_NOT_SUPPORTED`` and the next candidate is tried. Any other failure is returned. Several classes may handle the same next-chain structure, for example
several platform thread pool adapters for different frameworks.

The library allocates each object in a single block holding its private bookkeeping and the state of the class.
Handles are opaque pointers to such blocks.

An object is destroyed with the destroy function of its kind or, for event domains and services, of its base kind:
``co_event_domain_0_destroy()`` destroys event loops and event facilities, and ``co_service_0_destroy()`` destroys
standalone services of any service kind. Services obtained from an event domain are owned by it and are destroyed with
it. The destroy function destroys the state of the class and deallocates the block.
``co_executor_0_destroy()`` calls the ``destroy`` method of the executor's vtable, which deallocates the object.

Memory allocation
-----------------

Objects are allocated with ``co_allocation_callbacks_0``. Callbacks in the next chain of the instance create info
allocate the instance and every object created in it; callbacks in the next chain of an object create info allocate
that object only. Without callbacks, the library uses the global ``operator new`` and ``operator delete``.

The callbacks receive the alignment of each allocation as well as its size. ``new`` passes the alignment of the type
implicitly, but the callbacks get raw bytes, not a type, and some objects need more alignment than ``malloc``
guarantees, for example to keep state on its own cache line. ``allocate_fn`` must return memory aligned to the
requested power of two, or ``NULL``. ``deallocate_fn`` gets back the size and alignment of the allocation, so it can
use the matching release function (``_aligned_free()`` after ``_aligned_malloc()``, never ``free()``) and pool or arena
allocators need no header per block.

Classes implemented outside the library allocate their objects with the callbacks they are given too; in C++,
``co/impl.hpp`` provides helpers (see :doc:`api/cpp/impl`).

Results and preconditions
-------------------------

Functions return a ``co_result_0`` only for failures the caller cannot rule out in advance, for example an incompatible
API version, an unsupported next-chain entry, no class matching a create info, or memory and system resources running
out. Misuse the caller can always avoid is a precondition violation, documented with ``@pre``, and its behavior is
undefined: ``NULL`` handles or output pointers, a create info of the wrong structure type, unknown ids, out-of-range
indices and missing required callbacks. A function may still report a violation it happens to detect, but callers must
not rely on it; such checks belong to debug and validation layers.

Services are requested when an event domain is created, in the ``required_services`` of its create info. Only those
services may be asked for with ``co_event_domain_0_get_service_0()``, even when the chosen class provides more.

Handles and casts
-----------------

The handle of an object may be cast to the handle of any base kind of its kind (upcast) and back to the handle of its
kind or of any kind between them (downcast) with the cast macro of the target kind, for example
``CO_EVENT_DOMAIN_0_CAST(event_loop)`` and ``CO_EVENT_LOOP_0_CAST(event_domain)``; ``NULL`` stays ``NULL``. The macros
do not check the kinds: a downcast requires the object to be of the target kind. In C++, the casts of ``co.hpp``, for
example ``co::event_loop_0_cast()``, reject unrelated kinds at compile time.

Versions
--------

Versions have four 16-bit components: epoch, major, minor and patch. Bumping the epoch is a breaking change. Within an
epoch, bumping the first nonzero component of major, minor and patch is a breaking change (bumping patch when all of
them are zero), and bumping any later component is not.

An application written against version ``A`` can use a library of version ``L`` when both have the same epoch, the
components up to and including the first nonzero one of major, minor and patch are equal, and ``L`` is not older than
``A``. For example, an application written against ``0.0.2.1`` can use a library of version ``0.0.2.7``, but not one of
version ``0.0.3.0``.

Naming
------

Public symbols carry explicit version suffixes, so that new versions can be added next to old ones:

- a class (kind) name ends in its number: ``co_timer_0`` is generation 0 of timers, ``co_event_loop_0`` generation 0
  of event loops. The class name, with its number, is the prefix of its methods, create functions and create infos,
  which then end in their own version: ``co_timer_0_start_0``, ``co_instance_0_create_0``,
  ``co_instance_0_create_info_0``. Kinds without a handle are numbered the same way:
  ``co_fixed_thread_pool_0_create_info_0``. The bases of a version (base kind, header of a structure) are fixed when the
  version is defined and documented with it, not encoded in the name;
- an implementation module (a set of classes built and shipped together, in files named after it) has a generation
  too, and the kinds and structures it owns are named after it: ``co_win32_iocp_0_event_loop_0`` is version 0 of the
  event loop of generation 0 of the Win32 IOCP module, in ``co/win32/iocp_0.h``. A new implementation is a new module
  generation in new files, whose kinds are counted from 0 again. Kinds and structures that do not depend on an
  implementation are not named after a module: ``co_fixed_thread_pool_0_create_info_0``, ``co_win32_iocp_import_info_0``.
- a function or method takes the name of the handle it belongs to, with its number, followed by the function name and
  its version: ``co_timer_0_start_0``, ``co_instance_0_create_event_domain_0``. The version of a function fixes its
  parameter types and behavior, so the numbers of the types it uses are not repeated in its name;
- a structure is named by what it describes: a ``create_info`` describes an object to create
  (``co_instance_0_create_info_0``, ``co_fixed_thread_pool_0_create_info_0``), an ``import_info`` an existing object the
  application hands over (``co_win32_iocp_import_info_0``), an ``info`` anything else, typically an optional modifier
  of what the next chain asks for (``co_thread_count_bounds_info_0``). Required parameters are members of the create
  info; optional ones are separate ``info`` structures in the next chain. Having no members does not make a structure a
  tag. Structure type constants mirror the name: ``CO_STRUCTURE_TYPE_0_INSTANCE_0_CREATE_INFO_0``;
- create functions and create infos are versioned independently. A create function is like a constructor of the type
  it creates, so it is named after that type with its number, followed by its own version, as destroy functions are:
  ``co_instance_0_create_0`` creates a ``co_instance_0`` from a ``co_instance_0_create_info_0``; a
  ``co_instance_0_create_1`` would create the same kind with other parameters or behavior, and a ``co_instance_1_create_0``
  a new kind. A create info is named after the type with its number and may be taken by several create functions with
  different behavior (``co_instance_0_create_0`` and ``co_instance_0_create_1`` may both take
  ``co_instance_0_create_info_0``) or by none, and may be put in the next chain of other create infos. Methods creating
  another object are named after their handle: ``co_instance_0_create_event_domain_0``;
- destroy functions are not versioned: a kind generation has a single destructor.
