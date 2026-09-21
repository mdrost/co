#pragma once

#include <co/suspend_watcher_0_service_0.h>

#include <co/suspend_watcher_0.hpp>

namespace co
{

extern CO_API
const service_type_0 service_type_0_suspend_watcher_0_service_0;

// A service found with service_type_0_suspend_watcher_0_service_0 casts to suspend_watcher_0_service_0 with
// suspend_watcher_0_service_0_cast(), and a suspend_watcher_0_service_0 converts to service_0. A suspend watcher service
// found in an event domain is owned by it and must not be destroyed by the user.
class suspend_watcher_0_service_0 final
{
public:

	suspend_watcher_0_service_0() noexcept = default;

	suspend_watcher_0_service_0(std::nullptr_t) noexcept
	{
	}

	explicit suspend_watcher_0_service_0(co_suspend_watcher_0_service_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_suspend_watcher_0_service_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const suspend_watcher_0_service_0&, const suspend_watcher_0_service_0&) noexcept = default;

	/// See co_suspend_watcher_0_service_0_destroy().
	/// @pre The suspend watcher service was not obtained from an event domain, which owns it.
	/// @pre All suspend watchers created by the suspend watcher service have been destroyed.
	void destroy() const noexcept
	{
		co_suspend_watcher_0_service_0_destroy(m_handle);
	}

	operator service_0() const noexcept
	{
		return service_0(reinterpret_cast<co_service_0>(m_handle));
	}

	/// Creates a suspend watcher. See co_suspend_watcher_0_service_0_create_suspend_watcher_0().
	result_0 create_suspend_watcher_0(suspend_watcher_0* suspend_watcher) const noexcept
	{
		return static_cast<result_0>(co_suspend_watcher_0_service_0_create_suspend_watcher_0(m_handle, reinterpret_cast<co_suspend_watcher_0*>(suspend_watcher)));
	}

private:
	co_suspend_watcher_0_service_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<suspend_watcher_0_service_0, co_suspend_watcher_0_service_0>);

template <>
struct cast_traits<suspend_watcher_0_service_0>
{
	using base = service_0;
	using handle = co_suspend_watcher_0_service_0;
};

template <>
struct cast_traits<co_suspend_watcher_0_service_0>
{
	using base = service_0;
	using handle = co_suspend_watcher_0_service_0;
};

/// See Handle casts in co.hpp.
/// @pre from is null or a suspend watcher service.
template <class From>
	requires handle_castable_to<suspend_watcher_0_service_0, From>
[[nodiscard]] suspend_watcher_0_service_0 suspend_watcher_0_service_0_cast(From from) noexcept
{
	return handle_cast<suspend_watcher_0_service_0>(from);
}

} // namespace co
