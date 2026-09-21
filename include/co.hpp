#pragma once

#include <co.h>

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

namespace co
{

#pragma region Result

enum class result_0 : std::intptr_t
{
	success = 0,
	cancelled = 1,
	error_unknown = -1,
	error_invalid_argument = -2,
	error_incompatible_version = -3,
	error_not_supported = -4,
	error_out_of_memory = -5,
	error_service_not_found = -6,
	error_busy = -7,
	error_system = -8,
};

#pragma endregion

#pragma region Version

/// Four 16-bit components, from the most significant: epoch, major, minor, patch.
/// Bumping the epoch is a breaking change. Within an epoch, bumping the first nonzero component of major, minor and patch
/// is a breaking change (bumping patch when all of them are zero), and bumping any later component is not.
using version_0 = co_version_0;

[[nodiscard]] constexpr version_0 make_version_0(uint16_t epoch, uint16_t major, uint16_t minor, uint16_t patch) noexcept
{
	return CO_VERSION_0(epoch, major, minor, patch);
}

/// version_0 of the API declared by these headers.
inline constexpr version_0 api_version_0 = CO_API_VERSION_0;

#pragma endregion

#pragma region Structure type

enum class structure_type_0 : std::uintptr_t
{
};

/// See co_in_structure_0.
struct in_structure_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;
};

/// See CO_IN_STRUCTURE_0_CAST; converts a pointer to a C++ input structure into a pointer to in_structure_0, const if and
/// only if the structure is.
template <class T>
	requires detail::in_structure_of<std::remove_const_t<T>, in_structure_0>
auto in_structure_0_cast(T* structure) noexcept
{
	return detail::in_structure_cast<in_structure_0>(structure);
}

/// See CO_IN_STRUCTURE_0_CAST; converts a reference to a C++ input structure into a reference to in_structure_0, const if
/// and only if the structure is.
template <class T>
	requires detail::in_structure_of<std::remove_const_t<T>, in_structure_0>
auto& in_structure_0_cast(T& structure) noexcept
{
	return *detail::in_structure_cast<in_structure_0>(&structure);
}

/// See CO_IN_STRUCTURE_0_CAST; converts a reference to a C++ input structure, possibly a temporary, into a const
/// in_structure_0&.
template <class T>
	requires detail::in_structure_of<T, in_structure_0>
const in_structure_0& in_structure_0_cast(const T& structure) noexcept
{
	return *detail::in_structure_cast<in_structure_0>(&structure);
}

extern CO_API
const structure_type_0 structure_type_0_hint_info_0;

/// See co_hint_info_0.
struct hint_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	const in_structure_0* hint;
};

#pragma endregion

#pragma region Service type

enum class service_type_0 : std::uintptr_t
{
};

#pragma endregion

#pragma region Allocation

extern CO_API
const structure_type_0 structure_type_0_allocation_callbacks_0;

using allocation_callbacks_0_allocate_fn = void* (*)(void* data, std::size_t size, std::size_t alignment) noexcept;
using allocation_callbacks_0_deallocate_fn = void (*)(void* data, void* memory, std::size_t size, std::size_t alignment) noexcept;

/// See co_allocation_callbacks_0.
struct allocation_callbacks_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	allocation_callbacks_0_allocate_fn allocate_fn;
	allocation_callbacks_0_deallocate_fn deallocate_fn;
	void* data;
};

#pragma endregion

#pragma region Debug

enum class debug_severity_0 : std::uintptr_t
{
	info = 0,
	warning = 1,
	error = 2,
};

/// See co_debug_message_0.
struct debug_message_0 final
{
	debug_severity_0 severity;
	result_0 result;
	const char* message;
	const void* object;
	std::intptr_t system_error;
};

using debug_callback_0_callback_fn = void (*)(void* data, const debug_message_0* message) noexcept;

extern CO_API
const structure_type_0 structure_type_0_debug_callback_0;

/// See co_debug_callback_0.
struct debug_callback_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	debug_severity_0 min_severity;
	debug_callback_0_callback_fn callback_fn;
	void* data;
};

#pragma endregion

// Handles
//
// Handles are cheap values referring to opaque objects owned by the library; copying a handle does not copy the object.
// Their member functions forward to the C API. Objects are destroyed with the destroy() member function of their handle.
// A handle is layout-compatible with the matching C handle, explicitly constructible from it and explicitly convertible
// to it. A default-constructed handle is null. A handle converts implicitly to the handle of its base kind.
//
// Handle casts
//
// handle_cast<To>(from) and the named cast of each kind, for example event_loop_0_cast(from), cast a handle, C or C++,
// to the handle of a base kind (upcast) or of a derived kind (downcast); null stays null. They compile only when the
// kinds of From and To are related, as described by cast_traits. A downcast requires the object to be of the target
// kind; this is not checked at runtime.

class instance_0;
class timer_0;
class timer_0_service_0;

/// Describes a handle type, C or C++, to the handle casts. A kind specializes it for its C++ handle and for its C handle
/// with:
/// - base: the C++ handle of the direct base kind, or void;
/// - handle: the C handle of the kind, identifying the kind.
///     template <> struct co::cast_traits<my_event_loop_0>
///     {
///         using base = co::event_loop_0;
///         using handle = my_c_event_loop_0;
///     };
///     template <> struct co::cast_traits<my_c_event_loop_0>
///     {
///         using base = co::event_loop_0;
///         using handle = my_c_event_loop_0;
///     };
/// A kind only names its direct base: the casts between it and any kind of its chain then follow.
template <class Handle>
struct cast_traits
{
};

namespace detail
{

template <class Handle, class CHandle>
inline constexpr bool is_handle_layout_compatible = std::is_standard_layout_v<Handle> && std::is_trivially_copyable_v<Handle> && sizeof(Handle) == sizeof(CHandle) && alignof(Handle) == alignof(CHandle);

template <class Handle>
concept castable_handle = requires {
	typename cast_traits<Handle>::base;
	typename cast_traits<Handle>::handle;
};

// Whether the kind of Kind is the kind of Of or one of its base kinds.
template <class Kind, class Of>
consteval bool is_kind_or_base_kind() noexcept
{
	if constexpr (std::is_same_v<typename cast_traits<Kind>::handle, typename cast_traits<Of>::handle>) {
		return true;
	}
	else if constexpr (std::is_void_v<typename cast_traits<Of>::base>) {
		return false;
	}
	else {
		return is_kind_or_base_kind<Kind, typename cast_traits<Of>::base>();
	}
}

} // namespace detail

/// Whether handle_cast<To>() accepts a From: null, or a handle of a kind related to the kind of To.
template <class To, class From>
concept handle_castable_to = detail::castable_handle<To> && (std::is_same_v<From, std::nullptr_t> || (detail::castable_handle<From> && (detail::is_kind_or_base_kind<To, From>() || detail::is_kind_or_base_kind<From, To>())));

/// Casts a handle, C or C++, to the handle To, C or C++, of a related kind; see Handle casts.
/// @pre from is null, or a handle of an object of the kind of To.
template <class To, class From>
	requires handle_castable_to<To, From>
[[nodiscard]] To handle_cast(From from) noexcept
{
	if constexpr (std::is_same_v<From, std::nullptr_t>) {
		return To(nullptr);
	}
	else {
		return To(reinterpret_cast<typename cast_traits<To>::handle>(static_cast<typename cast_traits<From>::handle>(from)));
	}
}

/// Passes a handle as the output parameter of a function writing a handle of a related kind, C or C++, cast to the
/// handle of target with handle_cast when the full expression ends:
///     co::event_loop_0 event_loop;
///     instance.create_event_domain_0(co::event_domain_0_create_info_0_cast(create_info), co::out_handle(event_loop));
/// target is left unchanged when the function does not write its output.
/// @pre The handle written by the function is null or a handle of an object of the kind of Target.
template <detail::castable_handle Target>
class out_handle final
{
public:

	explicit out_handle(Target& target) noexcept
		: m_target(&target)
	{
	}

	out_handle(const out_handle&) = delete;
	out_handle& operator=(const out_handle&) = delete;

	~out_handle()
	{
		if (m_commit != nullptr) {
			m_commit(m_storage, *m_target);
		}
	}

	template <class Param>
		requires (!std::is_same_v<Param, std::nullptr_t>) && handle_castable_to<Target, Param>
	operator Param*() && noexcept
	{
		static_assert(sizeof(Param) <= sizeof(m_storage) && alignof(Param) <= alignof(void*));
		Param* param = ::new (static_cast<void*>(m_storage)) Param(handle_cast<Param>(*m_target));
		m_commit = &out_handle::commit<Param>;
		return param;
	}

private:

	template <class Param>
	static void commit(std::byte* storage, Target& target) noexcept
	{
		target = handle_cast<Target>(*std::launder(reinterpret_cast<Param*>(storage)));
	}

private:
	Target* m_target;
	void (*m_commit)(std::byte*, Target&) noexcept = nullptr;
	alignas(void*) std::byte m_storage[sizeof(void*)];
};

#pragma region Service

class service_0;

template <>
struct cast_traits<service_0>
{
	using base = void;
	using handle = co_service_0;
};

template <>
struct cast_traits<co_service_0>
{
	using base = void;
	using handle = co_service_0;
};

class service_0 final
{
public:

	service_0() noexcept = default;

	service_0(std::nullptr_t) noexcept
	{
	}

	explicit service_0(co_service_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_service_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const service_0&, const service_0&) noexcept = default;

	/// Destroys a standalone service of any service kind, for example a timer_0_service_0; see co_service_0_destroy().
	/// @pre The service was not obtained from an event domain, which owns it.
	/// @pre All objects created by the service have been destroyed.
	void destroy() const noexcept
	{
		co_service_0_destroy(m_handle);
	}

private:
	co_service_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<service_0, co_service_0>);

/// See Handle casts.
template <class From>
	requires handle_castable_to<service_0, From>
[[nodiscard]] service_0 service_0_cast(From from) noexcept
{
	return handle_cast<service_0>(from);
}

#pragma endregion

#pragma region Event domain

/// See co_event_domain_0_create_info_0: common initial sequence of every event domain create info.
struct event_domain_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	/// Services the event domain must provide.
	const service_type_0* required_services;
	size_t required_service_count;
};

/// See CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST; converts a pointer to a C++ event domain create info into a pointer to
/// event_domain_0_create_info_0, const if and only if the create info is.
template <class T>
	requires detail::event_domain_create_info_of<std::remove_const_t<T>, event_domain_0_create_info_0>
auto event_domain_0_create_info_0_cast(T* create_info) noexcept
{
	return detail::event_domain_create_info_cast<event_domain_0_create_info_0>(create_info);
}

/// See CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST; converts a reference to a C++ event domain create info, possibly a
/// temporary, into a const event_domain_0_create_info_0&.
template <class T>
	requires detail::event_domain_create_info_of<T, event_domain_0_create_info_0>
const event_domain_0_create_info_0& event_domain_0_create_info_0_cast(const T& create_info) noexcept
{
	return *detail::event_domain_create_info_cast<event_domain_0_create_info_0>(&create_info);
}

class event_domain_0;

template <>
struct cast_traits<event_domain_0>
{
	using base = void;
	using handle = co_event_domain_0;
};

template <>
struct cast_traits<co_event_domain_0>
{
	using base = void;
	using handle = co_event_domain_0;
};

/// Event loops and event facilities are event domains; their handles convert to event_domain_0.
class event_domain_0 final
{
public:

	event_domain_0() noexcept = default;

	event_domain_0(std::nullptr_t) noexcept
	{
	}

	explicit event_domain_0(co_event_domain_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_event_domain_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const event_domain_0&, const event_domain_0&) noexcept = default;

	/// Destroys an event domain created by instance_0::create_event_domain_0(), of any kind; see
	/// co_event_domain_0_destroy().
	void destroy() const noexcept
	{
		co_event_domain_0_destroy(m_handle);
	}

	result_0 get_service_0(service_type_0 service_type, service_0* service) const noexcept
	{
		return static_cast<result_0>(co_event_domain_0_get_service_0(m_handle, static_cast<co_service_type_0>(service_type), reinterpret_cast<co_service_0*>(service)));
	}

private:
	co_event_domain_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<event_domain_0, co_event_domain_0>);

/// See Handle casts.
template <class From>
	requires handle_castable_to<event_domain_0, From>
[[nodiscard]] event_domain_0 event_domain_0_cast(From from) noexcept
{
	return handle_cast<event_domain_0>(from);
}

#pragma endregion

#pragma region Event loop

extern CO_API
const structure_type_0 structure_type_0_event_loop_0_create_info_0;

/// Generic create info of event loops; see co_event_loop_0_create_info_0 and
/// co_instance_0_create_event_domain_0() for how the class is chosen.
struct event_loop_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	/// Services the event loop must provide.
	const service_type_0* required_services;
	size_t required_service_count;
};

class event_loop_0;

template <>
struct cast_traits<event_loop_0>
{
	using base = event_domain_0;
	using handle = co_event_loop_0;
};

template <>
struct cast_traits<co_event_loop_0>
{
	using base = event_domain_0;
	using handle = co_event_loop_0;
};

class event_loop_0 final
{
public:

	event_loop_0() noexcept = default;

	event_loop_0(std::nullptr_t) noexcept
	{
	}

	explicit event_loop_0(co_event_loop_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_event_loop_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const event_loop_0&, const event_loop_0&) noexcept = default;

	/// See co_event_loop_0_destroy().
	void destroy() const noexcept
	{
		co_event_loop_0_destroy(m_handle);
	}

	operator event_domain_0() const noexcept
	{
		return event_domain_0_cast(m_handle);
	}

	result_0 get_service_0(service_type_0 service_type, service_0* service) const noexcept
	{
		return static_cast<result_0>(co_event_loop_0_get_service_0(m_handle, static_cast<co_service_type_0>(service_type), reinterpret_cast<co_service_0*>(service)));
	}

	result_0 run_0() const noexcept
	{
		return static_cast<result_0>(co_event_loop_0_run_0(m_handle));
	}

	result_0 stop_0() const noexcept
	{
		return static_cast<result_0>(co_event_loop_0_stop_0(m_handle));
	}

private:
	co_event_loop_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<event_loop_0, co_event_loop_0>);

/// See Handle casts.
/// @pre from is null or an event loop.
template <class From>
	requires handle_castable_to<event_loop_0, From>
[[nodiscard]] event_loop_0 event_loop_0_cast(From from) noexcept
{
	return handle_cast<event_loop_0>(from);
}

#pragma endregion

#pragma region Event facility

extern CO_API
const structure_type_0 structure_type_0_event_facility_0_create_info_0;

/// Generic create info of event facilities; see co_event_facility_0_create_info_0 and
/// co_instance_0_create_event_domain_0() for how the class is chosen.
struct event_facility_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	/// Services the event facility must provide.
	const service_type_0* required_services;
	size_t required_service_count;
};

class event_facility_0;

template <>
struct cast_traits<event_facility_0>
{
	using base = event_domain_0;
	using handle = co_event_facility_0;
};

template <>
struct cast_traits<co_event_facility_0>
{
	using base = event_domain_0;
	using handle = co_event_facility_0;
};

class event_facility_0 final
{
public:

	event_facility_0() noexcept = default;

	event_facility_0(std::nullptr_t) noexcept
	{
	}

	explicit event_facility_0(co_event_facility_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_event_facility_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const event_facility_0&, const event_facility_0&) noexcept = default;

	/// See co_event_facility_0_destroy().
	void destroy() const noexcept
	{
		co_event_facility_0_destroy(m_handle);
	}

	operator event_domain_0() const noexcept
	{
		return event_domain_0_cast(m_handle);
	}

	result_0 get_service_0(service_type_0 service_type, service_0* service) const noexcept
	{
		return static_cast<result_0>(co_event_facility_0_get_service_0(m_handle, static_cast<co_service_type_0>(service_type), reinterpret_cast<co_service_0*>(service)));
	}

private:
	co_event_facility_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<event_facility_0, co_event_facility_0>);

/// See Handle casts.
/// @pre from is null or an event facility.
template <class From>
	requires handle_castable_to<event_facility_0, From>
[[nodiscard]] event_facility_0 event_facility_0_cast(From from) noexcept
{
	return handle_cast<event_facility_0>(from);
}

#pragma endregion

#pragma region Executor

extern CO_API
const structure_type_0 structure_type_0_executor_0_create_info_0;

struct executor_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;
};

using executor_handler_0_invoke_fn = void (*)(void* data) noexcept;

/// Handler posted to an executor: @p invoke_fn is called with @p data exactly once when the handler is invoked.
/// Layout-compatible with the C `co_executor_handler_0`.
struct executor_handler_0 final
{
	executor_handler_0_invoke_fn invoke_fn;
	void* data;
};

static_assert(std::is_standard_layout_v<executor_handler_0>);
static_assert(sizeof(executor_handler_0) == sizeof(co_executor_handler_0));
static_assert(offsetof(executor_handler_0, invoke_fn) == offsetof(co_executor_handler_0, invoke_fn));
static_assert(offsetof(executor_handler_0, data) == offsetof(co_executor_handler_0, data));

class executor_0;

template <>
struct cast_traits<executor_0>
{
	using base = void;
	using handle = co_executor_0;
};

template <>
struct cast_traits<co_executor_0>
{
	using base = void;
	using handle = co_executor_0;
};

/// Handlers posted to an executor are invoked exactly once, in the execution context of the executor:
/// - If post_0() succeeds, the handler is invoked exactly once; the invocation also releases whatever the handler owns.
/// - If post_0() fails, the handler is never invoked and ownership of its data stays with the caller.
/// - An executor may be destroyed once every handler it accepted has been invoked or is being invoked, and no post_0()
///   call on it is in progress. In particular it may be destroyed from within the invocation of its last accepted
///   handler; the executor then does not access its own state after that handler returns.
class executor_0 final
{
public:

	using handler_0 = executor_handler_0;

	executor_0() noexcept = default;

	executor_0(std::nullptr_t) noexcept
	{
	}

	explicit executor_0(co_executor_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_executor_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const executor_0&, const executor_0&) noexcept = default;

	/// See executor_0 for when an executor may be destroyed.
	void destroy() const noexcept
	{
		co_executor_0_destroy(m_handle);
	}

	/// On success the executor invokes @p handler exactly once in its execution context.
	/// On failure @p handler is never invoked. Returns result_0::error_out_of_memory when the executor cannot allocate
	/// its bookkeeping.
	result_0 post_0(handler_0 handler) const noexcept
	{
		return static_cast<result_0>(co_executor_0_post_0(m_handle, std::bit_cast<co_executor_handler_0>(handler)));
	}

	/// Posts a copy of @p handler_, allocated with `new`.
	/// The closure is destroyed within its invocation, or by this call when the executor rejects it.
	/// An exception escaping the closure invocation terminates the program.
	template <class Handler>
		requires std::invocable<std::decay_t<Handler>&> && std::is_nothrow_constructible_v<std::decay_t<Handler>, Handler>
	result_0 post_0(Handler&& handler) const noexcept
	{
		using handler_type = std::decay_t<Handler>;
		handler_type* handlerx = new (std::nothrow) handler_type(std::forward<Handler>(handler));
		if (handlerx == nullptr) {
			return result_0::error_out_of_memory;
		}
		result_0 result = post_0(handler_0{.invoke_fn = &executor_0::invoke<handler_type>, .data = handlerx});
		if (result != result_0::success) {
			delete handlerx;
		}
		return result;
	}

private:

	template <class Handler>
	static void invoke(void* data) noexcept
	{
		Handler* handler = static_cast<Handler*>(data);
		(*handler)();
		delete handler;
	}

private:
	co_executor_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<executor_0, co_executor_0>);

/// See Handle casts.
template <class From>
	requires handle_castable_to<executor_0, From>
[[nodiscard]] executor_0 executor_0_cast(From from) noexcept
{
	return handle_cast<executor_0>(from);
}

#pragma endregion

#pragma region Runtime

extern CO_API
const structure_type_0 structure_type_0_event_domain_group_0_create_info_0;

/// See co_event_domain_group_0_create_info_0.
struct event_domain_group_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	const event_domain_0_create_info_0* event_domain_create_info;
	size_t event_domain_count;
};

extern CO_API
const structure_type_0 structure_type_0_runtime_0_create_info_0;

/// See co_runtime_0_event_domain_group_info_0.
struct runtime_0_event_domain_group_info_0 final
{
	size_t id;
	const event_domain_group_0_create_info_0* create_info;
};

/// See co_runtime_0_executor_info_0.
struct runtime_0_executor_info_0 final
{
	size_t id;
	size_t event_domain_group_id;
	const executor_0_create_info_0* create_info;
};

/// See co_runtime_0_create_info_0.
struct runtime_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	const runtime_0_event_domain_group_info_0* event_domain_groups;
	size_t event_domain_group_count;

	const runtime_0_executor_info_0* executors;
	size_t executor_count;
};

class runtime_0 final
{
public:

	runtime_0() noexcept = default;

	runtime_0(std::nullptr_t) noexcept
	{
	}

	explicit runtime_0(co_runtime_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_runtime_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const runtime_0&, const runtime_0&) noexcept = default;

	/// See co_runtime_0_destroy().
	void destroy() const noexcept
	{
		co_runtime_0_destroy(m_handle);
	}

	/// See co_runtime_0_get_event_domain_0().
	void get_event_domain_0(size_t group_id, size_t domain_event_index, event_domain_0* event_domain) const noexcept
	{
		co_runtime_0_get_event_domain_0(m_handle, group_id, domain_event_index, reinterpret_cast<co_event_domain_0*>(event_domain));
	}

	/// See co_runtime_0_get_executor_0().
	void get_executor_0(size_t executor_id, executor_0* executor) const noexcept
	{
		co_runtime_0_get_executor_0(m_handle, executor_id, reinterpret_cast<co_executor_0*>(executor));
	}

private:
	co_runtime_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<runtime_0, co_runtime_0>);

#pragma endregion

#pragma region Instance

extern CO_API
const structure_type_0 structure_type_0_instance_0_create_info_0;

/// The next chain may hold allocation_callbacks_0, debug_callback_0 and event_domain_group_factory_source_0 entries;
/// see co_instance_0_create_info_0.
struct instance_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	/// API version the application is written against; usually api_version_0. See co_instance_0_create_info_0.
	version_0 api_version;
};

extern CO_API
const structure_type_0 structure_type_0_event_domain_group_factory_source_0;

/// See co_event_domain_group_factory_source_0. Factories are implemented with the C API.
struct event_domain_group_factory_source_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	co_event_domain_group_factory_0_entry_point_0_fn entry_point_fn;
	void* data;
};

class instance_0 final
{
public:

	instance_0() noexcept = default;

	instance_0(std::nullptr_t) noexcept
	{
	}

	explicit instance_0(co_instance_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_instance_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const instance_0&, const instance_0&) noexcept = default;

	/// @pre All objects created in the instance have been destroyed.
	void destroy() const noexcept
	{
		co_instance_0_destroy(m_handle);
	}

	/// See co_instance_0_create_event_domain_0(). Pass the create info with event_domain_0_create_info_0_cast(), and
	/// cast the event domain to the handle of its kind with event_loop_0_cast() or event_facility_0_cast().
	result_0 create_event_domain_0(const event_domain_0_create_info_0& create_info, event_domain_0* event_domain) const noexcept
	{
		return static_cast<result_0>(co_instance_0_create_event_domain_0(m_handle, reinterpret_cast<const co_event_domain_0_create_info_0*>(&create_info), reinterpret_cast<co_event_domain_0*>(event_domain)));
	}

	result_0 create_runtime_0(const runtime_0_create_info_0& create_info, runtime_0* runtime) const noexcept
	{
		return static_cast<result_0>(co_instance_0_create_runtime_0(m_handle, reinterpret_cast<const co_runtime_0_create_info_0*>(&create_info), reinterpret_cast<co_runtime_0*>(runtime)));
	}

private:
	co_instance_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<instance_0, co_instance_0>);

inline result_0 instance_0_create_0(const instance_0_create_info_0& create_info, instance_0* instance) noexcept
{
	return static_cast<result_0>(co_instance_0_create_0(reinterpret_cast<const co_instance_0_create_info_0*>(&create_info), reinterpret_cast<co_instance_0*>(instance)));
}

#pragma endregion

} // namespace co
