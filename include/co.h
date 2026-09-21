#pragma once

#include <co/export.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
# include <type_traits>
#endif

// Rule: Each class region (Instance, Service, Event domain, ...) is a Doxygen group and declares its things in the following
// order, skipping the ones the class does not have:
//   1. Cast macro of the handle. The handles themselves are declared in the Handles region.
//   2. Create info: its structure type constant, then its structure; then likewise each structure of its next chain.
//   3. Other structures used by the functions, each preceded by the typedefs of its callbacks.
//   4. Functions: create, destroy, then the other functions.
//   5. Implementation subgroup (@defgroup ..._impl): method typedefs in vtable order, forward declaration of the vtable,
//      object start structure, vtable, init function, then the other declarations used by implementers only.
//   6. Next-chain structures of the create infos of other classes that depend on the implementation subgroup, for example
//      co_event_domain_group_factory_source_0.

#ifdef __cplusplus
# define CO_NOEXCEPT noexcept
# define CO_FINAL final
#else
# define CO_NOEXCEPT
# define CO_FINAL
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*co_reserved_fn)(void); // private; do not use

#pragma region Result

/// @brief Result of a function: zero on success, positive for other non-error outcomes, negative on error.
typedef enum co_result_0 : intptr_t
{
	/// @brief The operation succeeded.
	CO_RESULT_0_SUCCESS = 0,
	/// @brief The operation was cancelled before it completed.
	CO_RESULT_0_CANCELLED = 1,
	/// @brief The operation failed for an unspecified reason.
	CO_RESULT_0_ERROR_UNKNOWN = -1,
	/// @brief An argument, or an object created by a class, is invalid. Functions may return it when they detect a violated
	/// precondition, but are not required to: violating a precondition is undefined behavior.
	CO_RESULT_0_ERROR_INVALID_ARGUMENT = -2,
	/// @brief The requested API version is not compatible with the library; see co_instance_0_create_info_0.
	CO_RESULT_0_ERROR_INCOMPATIBLE_VERSION = -3,
	/// @brief The operation, or an entry of a next chain, is not supported.
	CO_RESULT_0_ERROR_NOT_SUPPORTED = -4,
	/// @brief Memory could not be allocated.
	CO_RESULT_0_ERROR_OUT_OF_MEMORY = -5,
	/// @brief An event domain does not provide a service it is required to provide.
	CO_RESULT_0_ERROR_SERVICE_NOT_FOUND = -6,
	/// @brief The object is busy with a previous operation, for example a pending start.
	CO_RESULT_0_ERROR_BUSY = -7,
	/// @brief A system call failed; the debug message carries the system error code.
	CO_RESULT_0_ERROR_SYSTEM = -8,
} co_result_0;

#pragma endregion

#pragma region Version

/// @brief Four 16-bit components, from the most significant: epoch, major, minor, patch.
/// Bumping the epoch is a breaking change. Within an epoch, bumping the first nonzero component of major, minor and patch
/// is a breaking change (bumping patch when all of them are zero), and bumping any later component is not.
typedef uint64_t co_version_0;

/// @brief Builds a co_version_0 from its components; each is truncated to 16 bits.
#define CO_VERSION_0(epoch, major, minor, patch) \
	((((co_version_0)(epoch) & 0xFFFFu) << 48) | (((co_version_0)(major) & 0xFFFFu) << 32) | (((co_version_0)(minor) & 0xFFFFu) << 16) | ((co_version_0)(patch) & 0xFFFFu))

/// @brief Epoch component of a co_version_0.
#define CO_VERSION_0_EPOCH(version) ((uint16_t)((co_version_0)(version) >> 48))
/// @brief Major component of a co_version_0.
#define CO_VERSION_0_MAJOR(version) ((uint16_t)((co_version_0)(version) >> 32))
/// @brief Minor component of a co_version_0.
#define CO_VERSION_0_MINOR(version) ((uint16_t)((co_version_0)(version) >> 16))
/// @brief Patch component of a co_version_0.
#define CO_VERSION_0_PATCH(version) ((uint16_t)(co_version_0)(version))

/// @brief co_version_0 of the API declared by these headers.
#define CO_API_VERSION_0 CO_VERSION_0(0, 0, 0, 0)

#pragma endregion

#pragma region Structure type

/// @brief Identifies the type of an input structure. Values are the CO_STRUCTURE_TYPE_0_* constants exported by the library
/// and by modules; compare them, but do not rely on their numeric values.
typedef enum co_structure_type_0 : uintptr_t
{
} co_structure_type_0;

/// @brief Header shared by the input structures (create infos, import infos, registrations and extensions): every one of them starts
/// with these members, so that the entries of a next chain may be read through this structure to find their type.
/// Link a structure into a next chain with CO_IN_STRUCTURE_0_CAST.
///
/// The order of the entries of a next chain does not matter between entries of different structure types. A structure
/// type appears at most once in a chain unless its documentation allows several; such documentation states whether the
/// order of those entries matters (for example co_event_domain_group_factory_source_0, registered in chain order). An
/// entry that modifies another one (for example co_thread_count_bounds_info_0) requires that entry anywhere in the same
/// chain, not at a particular position.
typedef struct co_in_structure_0 CO_FINAL
{
	/// @brief Structure type of the structure.
	co_structure_type_0 structure_type;
	/// @brief Next entry of the next chain, or NULL.
	const struct co_in_structure_0* next_structure;
} co_in_structure_0;

/// @brief Converts a pointer to an input structure into a pointer to co_in_structure_0, const if and only if the structure is, for
/// example to link it into a next chain:
/// @code
/// platform.next_structure = CO_IN_STRUCTURE_0_CAST(&limits);
/// @endcode
/// Fails to compile unless the structure starts with a co_structure_type_0 structure_type followed by a
/// const co_in_structure_0* next_structure, laid out as in co_in_structure_0. The argument is not evaluated more than once.
/// Requires C23 (typeof) in C.
#ifdef __cplusplus
# define CO_IN_STRUCTURE_0_CAST(structure) (::co::detail::in_structure_cast<co_in_structure_0>(structure))
#else
# define CO_IN_STRUCTURE_0_CAST(structure) \
	(_Generic(&(structure)->next_structure, \
		const co_in_structure_0* const*: (const co_in_structure_0*)(structure), \
		default: (co_in_structure_0*)(structure)) + 0 * sizeof(struct { \
		_Static_assert(_Generic((structure)->structure_type, co_structure_type_0: 1, default: 0), "not an input structure"); \
		_Static_assert(_Generic((structure)->next_structure, const co_in_structure_0*: 1, default: 0), "not an input structure"); \
		_Static_assert(offsetof(typeof(*(structure)), structure_type) == offsetof(co_in_structure_0, structure_type), "not an input structure"); \
		_Static_assert(offsetof(typeof(*(structure)), next_structure) == offsetof(co_in_structure_0, next_structure), "not an input structure"); \
		int unused; \
	}))
#endif

/// @brief Returns the first entry of the next chain starting at chain whose structure type is structure_type, or NULL.
static inline const co_in_structure_0* co_in_structure_0_find_0(const co_in_structure_0* chain, co_structure_type_0 structure_type) CO_NOEXCEPT
{
	for (; chain != NULL; chain = chain->next_structure) {
		if (chain->structure_type == structure_type) {
			return chain;
		}
	}
	return NULL;
}

/// @brief Structure type of co_hint_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_HINT_INFO_0;

/// @brief Marks an input structure as a hint: a preference that classes may ignore, instead of a requirement.
///
/// A structure put directly in a next chain is a requirement: only classes handling its structure type are chosen, and
/// they may decline its values. Wrapped in a co_hint_info_0, the same structure does not affect the choice of the class;
/// the chosen class receives the wrapper unchanged in its chain, honors the hint when it knows its structure type, and
/// otherwise ignores it. A class never fails because of a hint. Any next chain may hold hints.
/// @pre hint is not NULL; at most one hint of each structure type is in a next chain. The next_structure of the hinted
/// structure is not read.
typedef struct co_hint_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief The hinted structure.
	const co_in_structure_0* hint;
} co_hint_info_0;

#pragma endregion

#pragma region Service type

/// @brief Identifies a service kind, for example the one of co_timer_0_service_0. Values are the CO_SERVICE_TYPE_0_* constants
/// exported by the library and by modules; compare them, but do not rely on their numeric values.
typedef enum co_service_type_0 : uintptr_t
{
} co_service_type_0;

#pragma endregion

#pragma region Handles

typedef struct co_instance_0_t co_instance_0_t; // private; do not use
/// @brief Handle of an instance.
/// @ingroup c_instance_0
typedef struct co_instance_0_t* co_instance_0;

typedef struct co_runtime_0_t co_runtime_0_t; // private; do not use
/// @brief Handle of a runtime.
/// @ingroup c_runtime_0
typedef struct co_runtime_0_t* co_runtime_0;

typedef struct co_event_domain_group_factory_0_t co_event_domain_group_factory_0_t;
/// @brief Handle of an event domain group factory.
/// @ingroup c_event_domain_group_factory_0
typedef co_event_domain_group_factory_0_t* co_event_domain_group_factory_0;

// Defined in the Event domain group section: event domain group classes start their objects and factories with them.
typedef struct co_event_domain_group_0_t co_event_domain_group_0_t;
/// @brief Handle of an event domain group.
/// @ingroup c_event_domain_group_0
typedef co_event_domain_group_0_t* co_event_domain_group_0;

// Defined in the Event domain, Event loop and Event facility sections: event domain classes start their objects with them.
typedef struct co_event_domain_0_t co_event_domain_0_t;
/// @brief Handle of an event domain. Event loops and event facilities are event domains: their handles may be cast to
/// co_event_domain_0 with CO_EVENT_DOMAIN_0_CAST, and back with CO_EVENT_LOOP_0_CAST or CO_EVENT_FACILITY_0_CAST.
/// @ingroup c_event_domain_0
typedef co_event_domain_0_t* co_event_domain_0;

typedef struct co_event_loop_0_t co_event_loop_0_t;
/// @brief Handle of an event loop.
/// @ingroup c_event_loop_0
typedef co_event_loop_0_t* co_event_loop_0;

typedef struct co_event_facility_0_t co_event_facility_0_t;
/// @brief Handle of an event facility.
/// @ingroup c_event_facility_0
typedef co_event_facility_0_t* co_event_facility_0;

// Defined in the Service section: service classes start their objects with it.
typedef struct co_service_0_t co_service_0_t;
/// @brief Handle of a service. Services are objects of classes; a service handle may be cast to the handle of its service
/// kind, for example co_timer_0_service_0.
/// @ingroup c_service_0
typedef co_service_0_t* co_service_0;

// Defined in the Executor section: executor classes start their objects with it.
typedef struct co_executor_0_t co_executor_0_t;
/// @brief Handle of an executor.
/// @ingroup c_executor_0
typedef co_executor_0_t* co_executor_0;

#pragma endregion

#pragma region Allocation

/// @brief Structure type of co_allocation_callbacks_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0;

/// @brief Returns memory of at least size bytes aligned to alignment, a power of two, or NULL on failure.
///
/// The library passes the alignment of the object it allocates, which may exceed that of malloc, for example for
/// cache-line aligned state. Forward it to an aligned allocator: `::operator new(size, std::align_val_t(alignment),
/// std::nothrow)` in C++, `_aligned_malloc()` on MSVC or `aligned_alloc()` in C11 (which needs size to be a multiple
/// of alignment).
typedef void* (* co_allocation_callbacks_0_allocate_fn)(void* data, size_t size, size_t alignment) CO_NOEXCEPT;

/// @brief Releases memory returned by the allocate_fn of the same callbacks, given the size and alignment it was allocated with.
///
/// Use the release function matching the allocator, for example `_aligned_free()` for `_aligned_malloc()`, never
/// `free()`. Size and alignment let pool and arena allocators release memory without storing a header.
typedef void (* co_allocation_callbacks_0_deallocate_fn)(void* data, void* memory, size_t size, size_t alignment) CO_NOEXCEPT;

/// @brief Allocation callbacks. In the next chain of co_instance_0_create_info_0, they allocate the instance and, unless
/// overridden, every object created in it. In the next chain of an object create info, they allocate that object only.
/// The library copies the structure; data must stay valid while memory allocated through the callbacks is in use.
/// At most one entry per next chain. Without allocation callbacks, the library uses the global `operator new` and
/// `operator delete` with the requested alignment. Classes implemented outside the library allocate with the
/// callbacks they are given; in C++, co/impl.hpp provides helpers.
typedef struct co_allocation_callbacks_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Required. Allocates memory.
	co_allocation_callbacks_0_allocate_fn allocate_fn;
	/// @brief Required. Deallocates memory returned by allocate_fn.
	co_allocation_callbacks_0_deallocate_fn deallocate_fn;
	/// @brief Passed to allocate_fn and deallocate_fn.
	void* data;
} co_allocation_callbacks_0;

#pragma endregion

#pragma region Debug

/// @brief Severity of a debug message, in increasing order.
typedef enum co_debug_severity_0 : uintptr_t
{
	/// @brief Information about the normal operation of the library.
	CO_DEBUG_SEVERITY_0_INFO = 0,
	/// @brief Something likely unintended that does not make an operation fail.
	CO_DEBUG_SEVERITY_0_WARNING = 1,
	/// @brief An operation fails.
	CO_DEBUG_SEVERITY_0_ERROR = 2,
} co_debug_severity_0;

/// @brief Debug message passed to a co_debug_callback_0.
typedef struct co_debug_message_0 CO_FINAL
{
	/// @brief Severity of the message.
	co_debug_severity_0 severity;
	/// @brief Result being returned because of what the message describes, or CO_RESULT_0_SUCCESS.
	co_result_0 result;
	/// @brief UTF-8, valid only during the callback.
	const char* message;
	/// @brief Handle of the object concerned, or NULL.
	const void* object;
	/// @brief System error code (GetLastError(), errno) behind the message, or 0.
	intptr_t system_error;
} co_debug_message_0;

/// @brief Receives a debug message; message is valid during the call only.
typedef void (* co_debug_callback_0_callback_fn)(void* data, const co_debug_message_0* message) CO_NOEXCEPT;

/// @brief Structure type of co_debug_callback_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_DEBUG_CALLBACK_0;

/// @brief Debug callback, in the next chain of co_instance_0_create_info_0; the chain may hold several. It receives the messages
/// of the instance, from its creation to its destruction, whose severity is at least min_severity. It may be called from
/// any thread, concurrently, and must not call co functions.
typedef struct co_debug_callback_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Lowest severity of the messages the callback receives.
	co_debug_severity_0 min_severity;
	/// @brief Required. Receives the messages.
	co_debug_callback_0_callback_fn callback_fn;
	/// @brief Passed to callback_fn; must stay valid until the instance is destroyed.
	void* data;
} co_debug_callback_0;

#pragma endregion

#pragma region Instance

/// @defgroup c_instance_0 Instance
/// @brief Instance of the library, from which the other objects are created.
/// @{

/// @brief Structure type of co_instance_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_INSTANCE_0_CREATE_INFO_0;

/// @brief Parameters of an instance.
///
/// The next chain may hold co_allocation_callbacks_0, co_debug_callback_0 and co_event_domain_group_factory_source_0
/// entries; instance creation fails with CO_RESULT_0_ERROR_NOT_SUPPORTED for any other entry. During its creation, the
/// instance registers the event domain group classes: the built-in ones, then those of each source in chain order (see
/// co_event_domain_group_factory_0_entry_point_0_fn).
typedef struct co_instance_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief API version the application is written against; usually CO_API_VERSION_0.
	/// The instance behaves as specified by this version. Instance creation fails with
	/// CO_RESULT_0_ERROR_INCOMPATIBLE_VERSION unless the epoch and the components up to and including the first nonzero
	/// one of major, minor and patch (all of them when they are all zero) equal the ones of the library, and the library
	/// version is not older than this version.
	co_version_0 api_version;
} co_instance_0_create_info_0;

/// @brief Creates in *instance an instance from create_info; destroy it with co_instance_0_destroy().
///
/// Returns CO_RESULT_0_ERROR_INCOMPATIBLE_VERSION when the API version is not compatible with the library. Fails with the
/// result of an entry point that fails. The create info and its chain are valid during the call only.
/// @pre create_info is not NULL and its structure type is CO_STRUCTURE_TYPE_0_INSTANCE_0_CREATE_INFO_0.
/// @pre The next chain holds at most one co_allocation_callbacks_0, and the required members of its entries are set.
/// @pre instance is not NULL.
CO_API
co_result_0 co_instance_0_create_0(const co_instance_0_create_info_0* create_info, co_instance_0* instance) CO_NOEXCEPT;

/// @brief Destroys an instance and the event domain group factories it owns; does nothing when instance is NULL.
/// @pre All objects created for the instance have been destroyed.
CO_API
void co_instance_0_destroy(co_instance_0 instance) CO_NOEXCEPT;

/// @}

#pragma endregion

#pragma region Service

/// @defgroup c_service_0 Service
/// @brief Services provided by event domains.
/// @{

/// @brief Casts the handle of a service of any service kind to co_service_0, unchecked; `NULL` stays `NULL`.
/// See "Handles and casts" in Concepts.
/// @ingroup c_service_0
#define CO_SERVICE_0_CAST(handle) ((co_service_0)(handle))

/// @brief Destroys a standalone service of any service kind, for example a co_timer_0_service_0.
/// @pre service was not obtained from an event domain (co_event_domain_0_get_service_0()); such a service is owned and
/// destroyed by its event domain.
/// @pre All objects created by the service have been destroyed.
CO_API
void co_service_0_destroy(co_service_0 service) CO_NOEXCEPT;

/// @defgroup c_service_0_impl Service implementation
/// @ingroup c_service_0
/// @brief Declarations used to implement service classes.
/// @{

/// @brief Type of co_service_0_vtable::destroy.
typedef void (* co_service_0_destroy_fn)(co_service_0 self) CO_NOEXCEPT;

typedef struct co_service_0_vtable co_service_0_vtable;

/// @brief Start of every service object.
struct co_service_0_t
{
	/// @brief Methods of the class of the object.
	const co_service_0_vtable* vtable;
};

/// @brief Methods of the services of a class. Must stay valid while objects of the class exist.
struct co_service_0_vtable CO_FINAL
{
	/// @brief API version of the headers the class is compiled against; usually CO_API_VERSION_0.
	co_version_0 api_version;
	/// @brief Required. Implements co_service_0_destroy(): destroys the object and deallocates its memory.
	co_service_0_destroy_fn destroy;
};

// TODO: doc
static inline void co_service_0_init(co_service_0 service, const co_service_0_vtable* service_vtable) CO_NOEXCEPT
{
	service->vtable = service_vtable;
}

/// @}

/// @}

#pragma endregion

#pragma region Event domain

/// @defgroup c_event_domain_0 Event domain
/// @brief Base kind of event loops and event facilities.
///
/// Event domains are created and owned by the objects of event domain group classes (see @ref c_event_domain_group_0).
/// An event domain object starts with the co_event_loop_0_t or co_event_facility_0_t of its kind, which starts with a
/// co_event_domain_0_t, initialized with co_event_loop_0_init() or co_event_facility_0_init(). Methods are only ever
/// appended to the vtables, as described for co_executor_0_vtable.
/// @{

/// @brief Casts the handle of an event loop or event facility to co_event_domain_0, unchecked; `NULL` stays `NULL`.
/// See "Handles and casts" in Concepts.
/// @ingroup c_event_domain_0
#define CO_EVENT_DOMAIN_0_CAST(handle) ((co_event_domain_0)(handle))

/// @brief Common initial sequence of every event domain create info: every one of them starts with these members, so that the
/// library may read the required services of any of them, as co_in_structure_0 is for the input structures.
///
/// The structure has no structure type of its own: a create info starting with it is, for example, the generic
/// co_event_loop_0_create_info_0 or co_event_facility_0_create_info_0, whose next chain may hold create infos such as
/// co_platform_thread_pool_0_create_info_0, or the create info of a class such as
/// co_win32_iocp_adapter_0_event_loop_0_create_info_0. Pass such a create info as a co_event_domain_0_create_info_0
/// with CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST.
typedef struct co_event_domain_0_create_info_0 CO_FINAL
{
	/// @brief Structure type of the create info starting with this sequence.
	co_structure_type_0 structure_type;
	/// @brief Next entry of the next chain: create infos, import infos and extensions, or NULL.
	const co_in_structure_0* next_structure;

	/// @brief Services the event domain must provide; only these may be asked for with co_event_domain_0_get_service_0().
	/// @pre Not NULL when required_service_count is not zero.
	const co_service_type_0* required_services;
	/// @brief Number of elements of required_services.
	size_t required_service_count;
} co_event_domain_0_create_info_0;

/// @brief Converts a pointer to an event domain create info into a pointer to co_event_domain_0_create_info_0, const if and
/// only if the create info is. Fails to compile unless the create info starts with the members of
/// co_event_domain_0_create_info_0, laid out as in it. The argument is not evaluated more than once. Requires C23 (typeof)
/// in C.
#ifdef __cplusplus
# define CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(create_info) (::co::detail::event_domain_create_info_cast<co_event_domain_0_create_info_0>(create_info))
#else
# define CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(create_info) \
	(_Generic(&(create_info)->next_structure, \
		const co_in_structure_0* const*: (const co_event_domain_0_create_info_0*)(create_info), \
		default: (co_event_domain_0_create_info_0*)(create_info)) + 0 * sizeof(struct { \
		_Static_assert(_Generic((create_info)->structure_type, co_structure_type_0: 1, default: 0), "not an event domain create info"); \
		_Static_assert(_Generic((create_info)->next_structure, const co_in_structure_0*: 1, default: 0), "not an event domain create info"); \
		_Static_assert(_Generic((create_info)->required_services, const co_service_type_0*: 1, default: 0), "not an event domain create info"); \
		_Static_assert(_Generic((create_info)->required_service_count, size_t: 1, default: 0), "not an event domain create info"); \
		_Static_assert(offsetof(typeof(*(create_info)), structure_type) == offsetof(co_event_domain_0_create_info_0, structure_type), "not an event domain create info"); \
		_Static_assert(offsetof(typeof(*(create_info)), next_structure) == offsetof(co_event_domain_0_create_info_0, next_structure), "not an event domain create info"); \
		_Static_assert(offsetof(typeof(*(create_info)), required_services) == offsetof(co_event_domain_0_create_info_0, required_services), "not an event domain create info"); \
		_Static_assert(offsetof(typeof(*(create_info)), required_service_count) == offsetof(co_event_domain_0_create_info_0, required_service_count), "not an event domain create info"); \
		int unused; \
	}))
#endif

/// @brief Creates a standalone event domain: the only member of an event domain group of the class chosen for create_info.
///
/// A registered class is a candidate when its factory lists the structure type of create_info among its member create info
/// types, its members provide every required service, it handles the structure type of every entry of the next chain of
/// create_info other than co_allocation_callbacks_0 and co_hint_info_0, and it admits groups of one. The candidates are tried by highest rank
/// first, and of equal rank the last registered one first. A candidate failing with CO_RESULT_0_ERROR_NOT_SUPPORTED declines
/// the values of create_info and its next chain, and the next candidate is tried; any other failure is returned. Fails with
/// CO_RESULT_0_ERROR_NOT_SUPPORTED when there is no candidate or every candidate declines. The event domain may be cast to the handle of its kind with CO_EVENT_LOOP_0_CAST or
/// CO_EVENT_FACILITY_0_CAST and is destroyed with co_event_domain_0_destroy().
/// @pre create_info is not NULL.
/// @pre event_domain is not NULL.
CO_API
co_result_0 co_instance_0_create_event_domain_0(co_instance_0 instance, const co_event_domain_0_create_info_0* create_info, co_event_domain_0* event_domain) CO_NOEXCEPT;

/// @brief Destroys an event domain created by co_instance_0_create_event_domain_0(), of any kind; does nothing when
/// event_domain is NULL.
/// @pre event_domain is not an event domain of a runtime; those are destroyed by the runtime.
/// @pre All objects created by the event domain and its services have been destroyed.
CO_API
void co_event_domain_0_destroy(co_event_domain_0 event_domain) CO_NOEXCEPT;

/// @brief On success stores in *service the service of service_type provided by the event domain.
///
/// The service is owned by the event domain and stays valid until the event domain is destroyed; it may be cast to the
/// handle of its service kind, for example co_timer_0_service_0. Services are requested when the event domain is created:
/// asking for a service that was not required is a precondition violation, even when the class provides it.
/// @pre service_type is one of the required_services of the create info the event domain was created from.
/// @pre service is not NULL.
CO_API
co_result_0 co_event_domain_0_get_service_0(co_event_domain_0 event_domain, co_service_type_0 service_type, co_service_0* service) CO_NOEXCEPT;

/// @defgroup c_event_domain_0_impl Event domain implementation
/// @ingroup c_event_domain_0
/// @brief Declarations used to implement event domain classes.
/// @{

/// @brief Type of co_event_domain_0_vtable::destroy.
typedef void (* co_event_domain_0_destroy_fn)(co_event_domain_0 self) CO_NOEXCEPT;

/// @brief Type of co_event_domain_0_vtable::get_service_0.
typedef co_result_0 (* co_event_domain_0_get_service_0_fn)(co_event_domain_0 self, co_service_type_0 service_type, co_service_0* service) CO_NOEXCEPT;

typedef struct co_event_domain_0_vtable co_event_domain_0_vtable;

/// @brief Start of every event domain object.
struct co_event_domain_0_t
{
	/// @brief Event domain methods of the class of the object.
	const co_event_domain_0_vtable* vtable;
};

/// @brief Methods of the event domains of a class. Must stay valid while objects of the class exist.
struct co_event_domain_0_vtable CO_FINAL
{
	/// @brief API version of the headers the class is compiled against; usually CO_API_VERSION_0. Also versions the vtables
	/// of the derived kinds of the object (co_event_loop_0_vtable, co_event_facility_0_vtable), which have no api_version of
	/// their own.
	co_version_0 api_version;
	/// @brief Required. Implements co_event_domain_0_destroy() for an event domain created by
	/// co_instance_0_create_event_domain_0(): destroys the group of one owning the event domain, as
	/// co_event_domain_group_0_vtable::destroy does. Never called for the members of the groups of a runtime.
	co_event_domain_0_destroy_fn destroy;
	/// @brief Required. Implements co_event_domain_0_get_service_0(): on success stores in *service a service owned by the
	/// event domain, valid until the event domain is destroyed.
	/// @pre service_type is listed by co_event_domain_group_factory_0_get_provided_service_types_0() of the factory of the class.
	co_event_domain_0_get_service_0_fn get_service_0;
};

// TODO: doc
static inline void co_event_domain_0_init(co_event_domain_0 event_domain, const co_event_domain_0_vtable* event_domain_vtable) CO_NOEXCEPT
{
	event_domain->vtable = event_domain_vtable;
}

/// @}

/// @}

#pragma endregion

#pragma region Event loop

/// @defgroup c_event_loop_0 Event loop
/// @brief Event domains run by the application on its own threads.
/// @{

/// @brief Casts a handle to co_event_loop_0, unchecked; `NULL` stays `NULL`. A downcast requires the object to be an
/// event loop. See "Handles and casts" in Concepts.
/// @ingroup c_event_loop_0
#define CO_EVENT_LOOP_0_CAST(handle) ((co_event_loop_0)(handle))

/// @brief Structure type of co_event_loop_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0;

/// @brief Generic create info of event loops, starting with co_event_domain_0_create_info_0: asks for an event loop of any class
/// listing CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0 among its member create info types, providing the required
/// services and handling every entry of the next chain. See co_instance_0_create_event_domain_0() for how the class
/// is chosen.
typedef struct co_event_loop_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Services the event loop must provide; only these may be asked for with co_event_domain_0_get_service_0().
	/// @pre Not NULL when required_service_count is not zero.
	const co_service_type_0* required_services;
	/// @brief Number of elements of required_services.
	size_t required_service_count;
} co_event_loop_0_create_info_0;

/// @brief Same as co_event_domain_0_destroy() on the event loop.
static inline void co_event_loop_0_destroy(co_event_loop_0 event_loop) CO_NOEXCEPT
{
	return co_event_domain_0_destroy(CO_EVENT_DOMAIN_0_CAST(event_loop));
}

/// @brief Same as co_event_domain_0_get_service_0() on the event loop.
static inline co_result_0 co_event_loop_0_get_service_0(co_event_loop_0 event_loop, co_service_type_0 service_type, co_service_0* service) CO_NOEXCEPT
{
	return co_event_domain_0_get_service_0(CO_EVENT_DOMAIN_0_CAST(event_loop), service_type, service);
}

/// @brief Runs the event loop on the calling thread, processing its events until co_event_loop_0_stop_0() is called.
CO_API
co_result_0 co_event_loop_0_run_0(co_event_loop_0 event_loop) CO_NOEXCEPT;

/// @brief Requests the event loop to stop: co_event_loop_0_run_0() returns soon after. May be called from any thread.
CO_API
co_result_0 co_event_loop_0_stop_0(co_event_loop_0 event_loop) CO_NOEXCEPT;

/// @defgroup c_event_loop_0_impl Event loop implementation
/// @ingroup c_event_loop_0
/// @brief Declarations used to implement event loop classes.
/// @{

/// @brief Type of co_event_loop_0_vtable::run_0.
typedef co_result_0 (* co_event_loop_0_run_0_fn)(co_event_loop_0 self) CO_NOEXCEPT;

/// @brief Type of co_event_loop_0_vtable::stop_0.
typedef co_result_0 (* co_event_loop_0_stop_0_fn)(co_event_loop_0 self) CO_NOEXCEPT;

typedef struct co_event_loop_0_vtable co_event_loop_0_vtable;

/// @brief Start of every event loop object.
struct co_event_loop_0_t
{
	/// @brief Start of the event domain the event loop is.
	co_event_domain_0_t event_domain;
	/// @brief Event loop methods of the class of the object.
	const co_event_loop_0_vtable* vtable;
};

/// @brief Methods of the event loops of a class, besides those of co_event_domain_0_vtable. Must stay valid while objects of the
/// class exist. Versioned by the api_version of the co_event_domain_0_vtable of the object: the library calls a method
/// appended in version V only when that api_version is at least V.
struct co_event_loop_0_vtable CO_FINAL
{
	/// @brief Required. Implements co_event_loop_0_run_0().
	co_event_loop_0_run_0_fn run_0;
	/// @brief Required. Implements co_event_loop_0_stop_0(); may be called from any thread.
	co_event_loop_0_stop_0_fn stop_0;
};

/// @brief Initializes the start of an event loop object: stores its vtables.
/// @pre The vtables have every required method and stay valid while the object exists.
static inline void co_event_loop_0_init(co_event_loop_0 event_loop, const co_event_domain_0_vtable* event_domain_vtable, const co_event_loop_0_vtable* event_loop_vtable) CO_NOEXCEPT
{
	co_event_domain_0_init(&event_loop->event_domain, event_domain_vtable);
	event_loop->vtable = event_loop_vtable;
}

/// @}

/// @}

#pragma endregion

#pragma region Event facility

/// @defgroup c_event_facility_0 Event facility
/// @brief Event domains whose handlers run on threads the application does not run through co, for example thread pools
/// managed by the facility itself, the operating system or an application framework.
/// @{

/// @brief Casts a handle to co_event_facility_0, unchecked; `NULL` stays `NULL`. A downcast requires the object to be an
/// event facility. See "Handles and casts" in Concepts.
/// @ingroup c_event_facility_0
#define CO_EVENT_FACILITY_0_CAST(handle) ((co_event_facility_0)(handle))

/// @brief Structure type of co_event_facility_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0;

/// @brief Generic create info of event facilities, starting with co_event_domain_0_create_info_0: asks for an event facility of
/// any class listing CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0 among its member create info types, providing
/// the required services and handling every entry of the next chain. To ask for a kind of event facility, put its create info in
/// the next chain, for example co_platform_thread_pool_0_create_info_0 or
/// co_dynamic_thread_pool_0_create_info_0. See co_instance_0_create_event_domain_0() for how the class is
/// chosen.
typedef struct co_event_facility_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Services the event facility must provide; only these may be asked for with co_event_domain_0_get_service_0().
	/// @pre Not NULL when required_service_count is not zero.
	const co_service_type_0* required_services;
	/// @brief Number of elements of required_services.
	size_t required_service_count;
} co_event_facility_0_create_info_0;

/// @brief Same as co_event_domain_0_destroy() on the event facility.
static inline void co_event_facility_0_destroy(co_event_facility_0 event_facility) CO_NOEXCEPT
{
	return co_event_domain_0_destroy(CO_EVENT_DOMAIN_0_CAST(event_facility));
}

/// @brief Same as co_event_domain_0_get_service_0() on the event facility.
static inline co_result_0 co_event_facility_0_get_service_0(co_event_facility_0 event_facility, co_service_type_0 service_type, co_service_0* service) CO_NOEXCEPT
{
	return co_event_domain_0_get_service_0(CO_EVENT_DOMAIN_0_CAST(event_facility), service_type, service);
}

/// @defgroup c_event_facility_0_impl Event facility implementation
/// @ingroup c_event_facility_0
/// @brief Declarations used to implement event facility classes.
/// @{

typedef struct co_event_facility_0_vtable co_event_facility_0_vtable;

/// @brief Start of every event facility object.
struct co_event_facility_0_t
{
	/// @brief Start of the event domain the event facility is.
	co_event_domain_0_t event_domain;
	/// @brief Event facility methods of the class of the object.
	const co_event_facility_0_vtable* vtable;
};

/// @brief Methods of the event facilities of a class, besides those of co_event_domain_0_vtable. Must stay valid while objects of
/// the class exist. Versioned by the api_version of the co_event_domain_0_vtable of the object, like co_event_loop_0_vtable.
/// It has no methods yet; the pointer to it is part of co_event_facility_0_t so that methods can be added without
/// changing the layout of event facility objects.
struct co_event_facility_0_vtable CO_FINAL
{
	co_reserved_fn reserved; // private; do not use
};

/// @brief Initializes the start of an event facility object: stores its vtable.
/// @pre The vtable has every required method and stays valid while the object exists.
static inline void co_event_facility_0_init(co_event_facility_0 event_facility, const co_event_domain_0_vtable* event_domain_vtable, const co_event_facility_0_vtable* event_facility_vtable) CO_NOEXCEPT
{
	co_event_domain_0_init(&event_facility->event_domain, event_domain_vtable);
	event_facility->vtable = event_facility_vtable;
}

/// @}

/// @}

#pragma endregion

#pragma region Executor

/// @defgroup c_executor_0 Executor
/// @brief Objects to which handlers are posted, to be invoked in their execution context.
///
/// Executors are objects of executor classes, built into the library (for example the strand) or implemented by
/// applications and plugins. An executor object starts with a co_executor_0_t (in C++ a class may instead derive publicly
/// from co_executor_0_t), which the class initializes with co_executor_0_init() before the object is used. The class
/// allocates and deallocates its objects itself; creating them is up to the class, for example a kind-specific create
/// function. co_executor_0_post_0() and co_executor_0_destroy() call the methods of the vtable of the object.
///
/// Methods are only ever appended to co_executor_0_vtable, whose size therefore depends on the version of the headers.
/// The vtable starts with the api_version of the headers the class is compiled against. The library calls a method added
/// in version V only through vtables whose api_version is at least V, and otherwise behaves as documented for classes
/// without that method, so a class keeps working with newer libraries of a compatible version (see
/// co_instance_0_create_info_0).
///
/// A kind derived from co_executor_0, including a kind defined by an application, does not extend co_executor_0_vtable:
/// its objects hold a pointer to its own vtable after the co_executor_0_t, so that both vtables may grow independently.
/// @{

/// @brief Casts the handle of an executor of any executor kind to co_executor_0, unchecked; `NULL` stays `NULL`.
/// See "Handles and casts" in Concepts.
/// @ingroup c_executor_0
#define CO_EXECUTOR_0_CAST(handle) ((co_executor_0)(handle))

/// @brief Structure type of co_executor_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_EXECUTOR_0_CREATE_INFO_0;

/// @brief Executor of any kind; concrete kinds and hints go in the next chain.
///
/// Does not say where the executor lives: in a runtime that is given by co_runtime_0_executor_info_0.
typedef struct co_executor_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;
} co_executor_0_create_info_0;

/// @brief Invokes a handler; the invocation releases data.
typedef void (* co_executor_handler_0_invoke_fn)(void* data) CO_NOEXCEPT;

/// @brief Handler posted to an executor: invoke_fn is called with data exactly once when the handler is invoked.
typedef struct co_executor_handler_0 CO_FINAL
{
	/// @brief Required. Called with data when the handler is invoked.
	co_executor_handler_0_invoke_fn invoke_fn;
	/// @brief Passed to invoke_fn; owned by the handler.
	void* data;
} co_executor_handler_0;

/// @brief Destroys an executor of any class; does nothing when executor is NULL. See co_executor_0_post_0() for when an
/// executor may be destroyed.
CO_API
void co_executor_0_destroy(co_executor_0 executor) CO_NOEXCEPT;

/// @brief Posts a handler to an executor.
///
/// On success, the handler is invoked exactly once, in the execution context of the executor.
/// On failure, the handler is never invoked and ownership of its data stays with the caller;
/// CO_RESULT_0_ERROR_OUT_OF_MEMORY is returned when the executor cannot allocate its bookkeeping.
/// An executor may be destroyed once every handler it accepted has been invoked or is being invoked, and no
/// co_executor_0_post_0() call on it is in progress. In particular it may be destroyed from within the invocation of
/// its last accepted handler; the executor then does not access its own state after that handler returns.
CO_API
co_result_0 co_executor_0_post_0(co_executor_0 executor, co_executor_handler_0 handler) CO_NOEXCEPT;

/// @defgroup c_executor_0_impl Executor implementation
/// @ingroup c_executor_0
/// @brief Declarations used to implement executor classes.
/// @{

/// @brief Type of co_executor_0_vtable::destroy.
typedef void (* co_executor_0_destroy_fn)(co_executor_0 self) CO_NOEXCEPT;

/// @brief Type of co_executor_0_vtable::post_0.
typedef co_result_0 (* co_executor_0_post_0_fn)(co_executor_0 self, co_executor_handler_0 handler) CO_NOEXCEPT;

typedef struct co_executor_0_vtable co_executor_0_vtable;

/// @brief Start of every executor object.
struct co_executor_0_t
{
	/// @brief Methods of the class of the object.
	const co_executor_0_vtable* vtable;
};

/// @brief Methods of an executor class. Must stay valid while objects of the class exist.
struct co_executor_0_vtable CO_FINAL
{
	/// @brief API version of the headers the class is compiled against; usually CO_API_VERSION_0.
	co_version_0 api_version;
	/// @brief Required. Implements co_executor_0_destroy(): destroys the object and deallocates its memory.
	co_executor_0_destroy_fn destroy;
	/// @brief Required. Implements co_executor_0_post_0().
	co_executor_0_post_0_fn post_0;
};

/// @brief Initializes the start of an executor object: stores its vtable.
/// @pre vtable has every required method and stays valid while the object exists.
static inline void co_executor_0_init(co_executor_0 executor, const co_executor_0_vtable* executor_vtable) CO_NOEXCEPT
{
	executor->vtable = executor_vtable;
}

/// @}

/// @}

#pragma endregion

#pragma region Event domain group

/// @defgroup c_event_domain_group_0 Event domain group
/// @brief Groups of identical event domains sharing resources.
///
/// Event domains are created in groups by the objects of event domain group classes, so that the members of a group may
/// share resources, for example one completion port waited on by every event loop of the group. A group object starts
/// with a co_event_domain_group_0_t initialized with co_event_domain_group_0_init(), and owns its members. Group objects
/// are created by the factory of their class and destroyed by the library. Methods are only ever appended to the
/// vtables, as described for co_executor_0_vtable.
/// @{

/// @brief Casts the handle of a group of any event domain group kind to co_event_domain_group_0,
/// unchecked; `NULL` stays `NULL`. See "Handles and casts" in Concepts.
/// @ingroup c_event_domain_group_0
#define CO_EVENT_DOMAIN_GROUP_0_CAST(handle) ((co_event_domain_group_0)(handle))

/// @brief Structure type of co_event_domain_group_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_0_CREATE_INFO_0;

/// @brief Group of event_domain_count identical event domains, all created from event_domain_create_info. A lone event domain is
/// a group of one. A group does not have to be targeted by an executor: it may exist only to provide services.
typedef struct co_event_domain_group_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	/// @brief Group-wide extensions, independent of the kind of the event domains.
	const co_in_structure_0* next_structure;

	/// @brief Create info of every event domain of the group
	/// co_event_domain_0_create_info_0.
	/// @pre Not NULL.
	const co_event_domain_0_create_info_0* event_domain_create_info;
	/// @brief Number of event domains of the group.
	/// @pre Not zero.
	size_t event_domain_count;
} co_event_domain_group_0_create_info_0;

/// @brief Destroys a group created by co_event_domain_group_factory_0_create_event_domain_group_0() and its members.
/// @pre The group is not a group of a runtime; those are destroyed by the runtime.
/// @pre All objects created by the members of the group and their services have been destroyed.
CO_API
void co_event_domain_group_0_destroy(co_event_domain_group_0 event_domain_group) CO_NOEXCEPT;

CO_API
void co_event_domain_group_0_get_event_domain_0(co_event_domain_group_0 event_domain_group, size_t event_domain_index, co_event_domain_0* event_domain) CO_NOEXCEPT;

/// @defgroup c_event_domain_group_0_impl Event domain group implementation
/// @ingroup c_event_domain_group_0
/// @brief Declarations used to implement event domain group classes.
/// @{

/// @brief Type of co_event_domain_group_0_vtable::destroy.
typedef void (* co_event_domain_group_0_destroy_fn)(co_event_domain_group_0 self) CO_NOEXCEPT;

/// @brief Type of co_event_domain_group_0_vtable::get_event_domain_0.
typedef void (* co_event_domain_group_0_get_event_domain_0_fn)(co_event_domain_group_0 self, size_t event_domain_index, co_event_domain_0* event_domain) CO_NOEXCEPT;

typedef struct co_event_domain_group_0_vtable co_event_domain_group_0_vtable;

/// @brief Start of every event domain group object.
struct co_event_domain_group_0_t
{
	/// @brief Methods of the class of the object.
	const co_event_domain_group_0_vtable* vtable;
};

/// @brief Methods of the groups of a class. Must stay valid while objects of the class exist.
struct co_event_domain_group_0_vtable CO_FINAL
{
	/// @brief API version of the headers the class is compiled against; usually CO_API_VERSION_0.
	co_version_0 api_version;
	/// @brief Required. Destroys the members and the group, and deallocates their memory.
	co_event_domain_group_0_destroy_fn destroy;
	/// @brief Required. Returns the member event_domain_index of the group.
	/// @pre event_domain_index is less than the event_domain_count of the create info of the group.
	co_event_domain_group_0_get_event_domain_0_fn get_event_domain_0;
};

/// @brief Initializes the start of an event domain group object: stores its vtable.
/// @pre vtable has every required method and stays valid while the object exists.
static inline void co_event_domain_group_0_init(co_event_domain_group_0 event_domain_group, const co_event_domain_group_0_vtable* event_domain_group_vtable) CO_NOEXCEPT
{
	event_domain_group->vtable = event_domain_group_vtable;
}

/// @}

/// @}

#pragma endregion

#pragma region Event domain group factory

/// @defgroup c_event_domain_group_factory_0 Event domain group factory
/// @brief Factories describing event domain group classes to the library and creating their groups.
///
/// A factory describes an event domain group class to the library and creates its groups. Factories are created by
/// entry points (see co_event_domain_group_factory_0_entry_point_0_fn) during the creation of an instance, which owns them
/// and destroys them in co_instance_0_destroy(). The library calls the getters once, right after the entry point, and
/// stores their answers, which describe what the class supports on this machine. Getters cannot fail: they only return
/// what the factory already holds, so work that can fail (probing the system, allocating the returned arrays) is done
/// when the factory is created, and a factory that cannot answer is not created. Arrays returned by the getters stay valid
/// until the factory is destroyed. Methods are only ever appended to the vtable, as described for co_executor_0_vtable.
/// @{

/// @brief Casts the handle of a factory of any event domain group factory kind to co_event_domain_group_factory_0,
/// unchecked; `NULL` stays `NULL`. See "Handles and casts" in Concepts.
/// @ingroup c_event_domain_group_factory_0
#define CO_EVENT_DOMAIN_GROUP_FACTORY_0_CAST(handle) ((co_event_domain_group_factory_0)(handle))

/// @brief Destroys the factory and deallocates its memory.
/// @pre The factory is not owned by an instance; those are destroyed by co_instance_0_destroy().
/// @pre Every group created by the factory has been destroyed.
CO_API
void co_event_domain_group_factory_0_destroy(co_event_domain_group_factory_0 event_domain_group_factory) CO_NOEXCEPT;

/// @brief Returns in *member_create_info_structure_types the structure types of the event domain create infos the members of the groups
/// may be created from, every version of them (for example CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0 or
/// CO_STRUCTURE_TYPE_0_WIN32_IOCP_0_EVENT_LOOP_0_CREATE_INFO_0), and stores their number, not zero,
/// in *member_create_info_structure_type_count. Each of them starts with co_event_domain_0_create_info_0. The array stays valid
/// until the factory is destroyed.
CO_API
void co_event_domain_group_factory_0_get_member_create_info_structure_types_0(co_event_domain_group_factory_0 event_domain_group_factory, const co_structure_type_0** member_create_info_structure_types, size_t* member_create_info_structure_type_count) CO_NOEXCEPT;

/// @brief Returns in *handled_structure_types the structure types of the entries of the next chains of member and group create
/// infos the class handles (create infos, import infos and extensions, every version of them), and stores their number in
/// *handled_structure_type_count. The array stays valid until the factory is destroyed. co_allocation_callbacks_0 and
/// co_hint_info_0 are handled by every class and are not listed.
CO_API
void co_event_domain_group_factory_0_get_handled_structure_types_0(co_event_domain_group_factory_0 event_domain_group_factory, const co_structure_type_0** handled_structure_types, size_t* handled_structure_type_count) CO_NOEXCEPT;

/// @brief Returns in *provided_service_types the services every member of the groups provides, and stores their number in
/// *provided_service_type_count. The array stays valid until the factory is destroyed.
CO_API
void co_event_domain_group_factory_0_get_provided_service_types_0(co_event_domain_group_factory_0 event_domain_group_factory, const co_service_type_0** provided_service_types, size_t* provided_service_type_count) CO_NOEXCEPT;

/// @brief Stores the range of the member counts of the groups: 1 <= *min_count <= *max_count.
CO_API
void co_event_domain_group_factory_0_get_member_count_range_0(co_event_domain_group_factory_0 event_domain_group_factory, size_t* min_count, size_t* max_count) CO_NOEXCEPT;

/// @brief Stores in *rank the rank of the class: candidate classes are tried by highest rank first.
CO_API
void co_event_domain_group_factory_0_get_rank_0(co_event_domain_group_factory_0 event_domain_group_factory, size_t* rank) CO_NOEXCEPT;

/// @brief Creates in *group a group of create_info->event_domain_count members, each from
/// create_info->event_domain_create_info. The next chain of create_info holds exactly one co_allocation_callbacks_0, which
/// allocate the group and its members; co_allocation_callbacks_0 in the next chain of the member create info are ignored.
/// All or nothing: on failure nothing is left created. Fails with CO_RESULT_0_ERROR_NOT_SUPPORTED, and only then, when the
/// class handles the structure types of the create infos but cannot honor their values; the instance then tries the next
/// candidate class. Destroy the group with co_event_domain_group_0_destroy().
CO_API
co_result_0 co_event_domain_group_factory_0_create_event_domain_group_0(co_event_domain_group_factory_0 event_domain_group_factory, const co_event_domain_group_0_create_info_0* create_info, co_event_domain_group_0* group) CO_NOEXCEPT;

/// @defgroup c_event_domain_group_factory_0_impl Event domain group factory implementation
/// @ingroup c_event_domain_group_factory_0
/// @brief Declarations used to implement event domain group classes and their factories.
/// @{

/// @brief Type of co_event_domain_group_factory_0_vtable::destroy.
typedef void (* co_event_domain_group_factory_0_destroy_fn)(co_event_domain_group_factory_0 self) CO_NOEXCEPT;

/// @brief Type of co_event_domain_group_factory_0_vtable::get_member_create_info_structure_types_0.
typedef void (* co_event_domain_group_factory_0_get_member_create_info_structure_types_0_fn)(co_event_domain_group_factory_0 self, const co_structure_type_0** member_create_info_structure_types, size_t* member_create_info_structure_type_count) CO_NOEXCEPT;

/// @brief Type of co_event_domain_group_factory_0_vtable::get_handled_structure_types_0.
typedef void (* co_event_domain_group_factory_0_get_handled_structure_types_0_fn)(co_event_domain_group_factory_0 self, const co_structure_type_0** handled_structure_types, size_t* handled_structure_type_count) CO_NOEXCEPT;

/// @brief Type of co_event_domain_group_factory_0_vtable::get_provided_service_types_0.
typedef void (* co_event_domain_group_factory_0_get_provided_service_types_0_fn)(co_event_domain_group_factory_0 self, const co_service_type_0** provided_service_types, size_t* provided_service_type_count) CO_NOEXCEPT;

/// @brief Type of co_event_domain_group_factory_0_vtable::get_member_count_range_0.
typedef void (* co_event_domain_group_factory_0_get_member_count_range_0_fn)(co_event_domain_group_factory_0 self, size_t* min_count, size_t* max_count) CO_NOEXCEPT;

/// @brief Type of co_event_domain_group_factory_0_vtable::get_rank_0.
typedef void (* co_event_domain_group_factory_0_get_rank_0_fn)(co_event_domain_group_factory_0 self, size_t* rank) CO_NOEXCEPT;

/// @brief Type of co_event_domain_group_factory_0_vtable::create_event_domain_group_0.
typedef co_result_0 (* co_event_domain_group_factory_0_create_event_domain_group_0_fn)(co_event_domain_group_factory_0 self, const co_event_domain_group_0_create_info_0* create_info, co_event_domain_group_0* group) CO_NOEXCEPT;

typedef struct co_event_domain_group_factory_0_vtable co_event_domain_group_factory_0_vtable;

/// @brief Start of every factory.
struct co_event_domain_group_factory_0_t
{
	/// @brief Methods of the factory.
	const co_event_domain_group_factory_0_vtable* vtable;
};

/// @brief Methods of a factory. Must stay valid while the factory exists.
struct co_event_domain_group_factory_0_vtable CO_FINAL
{
	/// @brief API version of the headers the class is compiled against; usually CO_API_VERSION_0.
	co_version_0 api_version;
	/// @brief Required. Destroys the factory and deallocates its memory; called once every group it created has been
	/// destroyed.
	co_event_domain_group_factory_0_destroy_fn destroy;
	/// @brief Required. Returns the structure types of the event domain create infos the members may be created from, every
	/// version of them (for example CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0), and stores their number, not zero, in
	/// *member_create_info_structure_type_count.
	co_event_domain_group_factory_0_get_member_create_info_structure_types_0_fn get_member_create_info_structure_types_0;
	/// @brief Required. Returns the structure types of the entries of the next chains of member and group create infos the
	/// class handles (create infos, import infos and extensions, every version of them), and stores their number in
	/// *handled_structure_type_count.
	/// co_allocation_callbacks_0 and co_hint_info_0 are handled by every class and are not listed.
	co_event_domain_group_factory_0_get_handled_structure_types_0_fn get_handled_structure_types_0;
	/// @brief Required. Returns the services every member provides, and stores their number in *provided_service_type_count.
	co_event_domain_group_factory_0_get_provided_service_types_0_fn get_provided_service_types_0;
	/// @brief Required. Stores the range of the member counts of the groups: 1 <= *min_count <= *max_count.
	co_event_domain_group_factory_0_get_member_count_range_0_fn get_member_count_range_0;
	/// @brief Required. Returns the rank of the class: candidate classes are tried by highest rank first.
	co_event_domain_group_factory_0_get_rank_0_fn get_rank_0;
	/// @brief Required. Creates a group of create_info->event_domain_count members, each from
	/// create_info->event_domain_create_info. The next chain of create_info holds exactly one co_allocation_callbacks_0,
	/// which allocate the group and its members; co_allocation_callbacks_0 in the next chain of the member create info are
	/// ignored. All or nothing: on failure nothing is left created. Returns CO_RESULT_0_ERROR_NOT_SUPPORTED, and only then,
	/// to decline values of the create infos the class cannot honor, so that the next candidate class is tried. The create info and its chains are valid during the call
	/// only.
	co_event_domain_group_factory_0_create_event_domain_group_0_fn create_event_domain_group_0;
};

/// @brief Initializes the start of a factory: stores its vtable.
/// @pre vtable has every required method and stays valid while the factory exists.
static inline void co_event_domain_group_factory_0_init(co_event_domain_group_factory_0 event_domain_group_factory, const co_event_domain_group_factory_0_vtable* event_domain_group_factory_vtable) CO_NOEXCEPT
{
	event_domain_group_factory->vtable = event_domain_group_factory_vtable;
}

/// @brief Structure type of co_event_domain_group_factory_0_entry_point_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_FACTORY_0_ENTRY_POINT_INFO_0;

/// @brief Everything an entry point may use; it gets no instance, which is being created.
typedef struct co_event_domain_group_factory_0_entry_point_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	/// @brief Holds exactly one co_allocation_callbacks_0 (those of the instance) and the co_debug_callback_0 entries of the
	/// instance. Factories may keep copies of the entries.
	const co_in_structure_0* next_structure;

	/// @brief API version of the instance, see co_instance_0_create_info_0.
	co_version_0 api_version;
} co_event_domain_group_factory_0_entry_point_info_0;

/// @brief Creates the factories of a source during the creation of an instance; called twice.
///
/// With factories NULL, stores in *factory_count the number of factories it creates. Otherwise creates at most
/// *factory_count factories into factories and stores their number in *factory_count. A class not supported on this
/// machine is skipped. All or nothing: on failure no factory is left created, and the creation of the instance fails with
/// the result. info is valid during the call only.
typedef co_result_0 (* co_event_domain_group_factory_0_entry_point_0_fn)(void* data, const co_event_domain_group_factory_0_entry_point_info_0* info, size_t* factory_count, co_event_domain_group_factory_0* factories) CO_NOEXCEPT;

/// @}

/// @brief Structure type of co_event_domain_group_factory_source_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_FACTORY_SOURCE_0;

/// @brief Extension of co_instance_0_create_info_0: an entry point whose factories the instance registers. data must stay
/// valid during the creation of the instance. A next chain may hold several sources; their order matters: the instance
/// registers their factories in chain order, after the built-in ones, and among classes of equal rank the last registered
/// one is tried first.
typedef struct co_event_domain_group_factory_source_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Required. Creates the factories of the source.
	co_event_domain_group_factory_0_entry_point_0_fn entry_point_fn;
	/// @brief Passed to entry_point_fn.
	void* data;
} co_event_domain_group_factory_source_0;

/// @}

#pragma endregion

#pragma region Runtime

/// @defgroup c_runtime_0 Runtime
/// @brief Owner of the event domain groups and executors of an application.
/// @{

/// @brief Structure type of co_runtime_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_RUNTIME_0_CREATE_INFO_0;

/// @brief Event domain group of a runtime, in co_runtime_0_create_info_0: the runtime id of the group and what to create.
typedef struct co_runtime_0_event_domain_group_info_0 CO_FINAL
{
	/// @brief Id of the group within the runtime; see co_runtime_0_get_event_domain_0().
	/// @pre Unique among the event domain groups of the runtime create info.
	size_t id;
	/// @brief The group to create.
	/// @pre Not NULL.
	const co_event_domain_group_0_create_info_0* create_info;
} co_runtime_0_event_domain_group_info_0;

/// @brief Executor of a runtime, in co_runtime_0_create_info_0: the runtime id of the executor, the group it targets and
/// what to create.
typedef struct co_runtime_0_executor_info_0 CO_FINAL
{
	/// @brief Id of the executor within the runtime; see co_runtime_0_get_executor_0().
	/// @pre Unique among the executors of the runtime create info.
	size_t id;
	/// @brief Id of the event domain group whose event domains are the execution contexts of the executor.
	/// @pre The id of an event domain group of the runtime create info.
	size_t event_domain_group_id;
	/// @brief The executor to create.
	/// @pre Not NULL.
	const co_executor_0_create_info_0* create_info;
} co_runtime_0_executor_info_0;

/// @brief Parameters of a runtime.
///
/// The runtime owns the event domains, their services and the executors it creates, and destroys all of them in
/// co_runtime_0_destroy(). It never creates threads: the caller runs every event loop.
typedef struct co_runtime_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Groups are created in array order and destroyed in reverse order.
	const co_runtime_0_event_domain_group_info_0* event_domain_groups;
	/// @brief Number of elements of event_domain_groups.
	size_t event_domain_group_count;

	/// @brief Executors, each targeting one event domain group; created after and destroyed before the groups.
	const co_runtime_0_executor_info_0* executors;
	/// @brief Number of elements of executors.
	size_t executor_count;
} co_runtime_0_create_info_0;

/// @brief Creates in *runtime a runtime of the instance from create_info, with its event domain groups and executors;
/// destroy it with co_runtime_0_destroy(). All or nothing: on failure nothing is left created. The create info and its
/// chains are valid during the call only.
/// @pre create_info is not NULL.
CO_API
co_result_0 co_instance_0_create_runtime_0(co_instance_0 instance, const co_runtime_0_create_info_0* create_info, co_runtime_0* runtime) CO_NOEXCEPT;

/// @brief Destroys a runtime: its executors, then its event domain groups in reverse order; does nothing when runtime is
/// NULL.
/// @pre No event loop of the runtime is running, and every handler accepted by the executors and event facilities of the
/// runtime has been invoked.
/// @pre All objects created by the event domains of the runtime and their services have been destroyed.
CO_API
void co_runtime_0_destroy(co_runtime_0 runtime) CO_NOEXCEPT;

/// @brief On success stores in *event_domain the event domain domain_event_index of the event domain group with id group_id.
///
/// The event domain is owned by the runtime and stays valid until co_runtime_0_destroy(). It may be cast with
/// CO_EVENT_LOOP_0_CAST or CO_EVENT_FACILITY_0_CAST according to the create info of the group.
/// @pre group_id is the id of an event domain group of the runtime create info, and domain_event_index is less than its
/// event_domain_count.
/// @pre event_domain is not NULL.
CO_API
void co_runtime_0_get_event_domain_0(co_runtime_0 runtime, size_t group_id, size_t event_domain_index, co_event_domain_0* event_domain) CO_NOEXCEPT;

/// @brief On success stores in *executor the executor with id executor_id.
///
/// The executor is owned by the runtime and stays valid until co_runtime_0_destroy().
/// @pre executor_id is the id of an executor of the runtime create info.
/// @pre executor is not NULL.
CO_API
void co_runtime_0_get_executor_0(co_runtime_0 runtime, size_t executor_id, co_executor_0* executor) CO_NOEXCEPT;

/// @}

#pragma endregion

#ifdef __cplusplus
}

namespace co::detail
{

// Whether T starts with the members of the input structure header Header, laid out as in Header.
template <class T, class Header>
concept in_structure_of = std::is_standard_layout_v<T> && requires (const T& structure) {
	requires std::is_same_v<decltype(structure.structure_type), decltype(Header::structure_type)>;
	requires std::is_same_v<decltype(structure.next_structure), const Header*>;
	requires offsetof(T, structure_type) == offsetof(Header, structure_type);
	requires offsetof(T, next_structure) == offsetof(Header, next_structure);
};

// See CO_IN_STRUCTURE_0_CAST; the result is const if and only if T is.
template <class Header, class T>
	requires in_structure_of<std::remove_const_t<T>, Header>
std::conditional_t<std::is_const_v<T>, const Header, Header>* in_structure_cast(T* structure) noexcept
{
	return reinterpret_cast<std::conditional_t<std::is_const_v<T>, const Header, Header>*>(structure);
}

// Whether T starts with the members of the event domain create info sequence Sequence, laid out as in Sequence.
template <class T, class Sequence>
concept event_domain_create_info_of = in_structure_of<T, std::remove_const_t<std::remove_pointer_t<decltype(Sequence::next_structure)>>> && requires (const T& create_info) {
	requires std::is_same_v<decltype(create_info.required_services), decltype(Sequence::required_services)>;
	requires std::is_same_v<decltype(create_info.required_service_count), decltype(Sequence::required_service_count)>;
	requires offsetof(T, structure_type) == offsetof(Sequence, structure_type);
	requires offsetof(T, next_structure) == offsetof(Sequence, next_structure);
	requires offsetof(T, required_services) == offsetof(Sequence, required_services);
	requires offsetof(T, required_service_count) == offsetof(Sequence, required_service_count);
};

// See CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST; the result is const if and only if T is.
template <class Sequence, class T>
	requires event_domain_create_info_of<std::remove_const_t<T>, Sequence>
std::conditional_t<std::is_const_v<T>, const Sequence, Sequence>* event_domain_create_info_cast(T* create_info) noexcept
{
	return reinterpret_cast<std::conditional_t<std::is_const_v<T>, const Sequence, Sequence>*>(create_info);
}

} // namespace co::detail
#endif
