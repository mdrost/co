#pragma once

#include <co/time_watcher_0_service_0.h>

#include <co/time_watcher_0.hpp>

namespace co
{

extern CO_API
const service_type_0 service_type_0_time_watcher_0_service_0;

// A service found with service_type_0_time_watcher_0_service_0 casts to time_watcher_0_service_0 with
// time_watcher_0_service_0_cast(), and a time_watcher_0_service_0 converts to service_0. A time watcher service found in
// an event domain is owned by it and must not be destroyed by the user.
class time_watcher_0_service_0 final
{
public:

	time_watcher_0_service_0() noexcept = default;

	time_watcher_0_service_0(std::nullptr_t) noexcept
	{
	}

	explicit time_watcher_0_service_0(co_time_watcher_0_service_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_time_watcher_0_service_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const time_watcher_0_service_0&, const time_watcher_0_service_0&) noexcept = default;

	/// See co_time_watcher_0_service_0_destroy().
	/// @pre The time watcher service was not obtained from an event domain, which owns it.
	/// @pre All time watchers created by the time watcher service have been destroyed.
	void destroy() const noexcept
	{
		co_time_watcher_0_service_0_destroy(m_handle);
	}

	operator service_0() const noexcept
	{
		return service_0(reinterpret_cast<co_service_0>(m_handle));
	}

	/// Creates a time watcher. See co_time_watcher_0_service_0_create_time_watcher_0().
	result_0 create_time_watcher_0(time_watcher_0* time_watcher) const noexcept
	{
		return static_cast<result_0>(co_time_watcher_0_service_0_create_time_watcher_0(m_handle, reinterpret_cast<co_time_watcher_0*>(time_watcher)));
	}

private:
	co_time_watcher_0_service_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<time_watcher_0_service_0, co_time_watcher_0_service_0>);

template <>
struct cast_traits<time_watcher_0_service_0>
{
	using base = service_0;
	using handle = co_time_watcher_0_service_0;
};

template <>
struct cast_traits<co_time_watcher_0_service_0>
{
	using base = service_0;
	using handle = co_time_watcher_0_service_0;
};

/// See Handle casts in co.hpp.
/// @pre from is null or a time watcher service.
template <class From>
	requires handle_castable_to<time_watcher_0_service_0, From>
[[nodiscard]] time_watcher_0_service_0 time_watcher_0_service_0_cast(From from) noexcept
{
	return handle_cast<time_watcher_0_service_0>(from);
}

} // namespace co
