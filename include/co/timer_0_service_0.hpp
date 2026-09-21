#pragma once

#include <co/timer_0_service_0.h>

#include <co/timer_0.hpp>

namespace co
{

extern CO_API
const service_type_0 service_type_0_timer_0_service_0;

// A service found with service_type_0_timer_0_service_0 casts to timer_0_service_0 with timer_0_service_0_cast(), and a
// timer_0_service_0 converts to service_0. A timer service found in an event domain is owned by it and must not be
// destroyed by the user.
class timer_0_service_0 final
{
public:

	timer_0_service_0() noexcept = default;

	timer_0_service_0(std::nullptr_t) noexcept
	{
	}

	explicit timer_0_service_0(co_timer_0_service_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_timer_0_service_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const timer_0_service_0&, const timer_0_service_0&) noexcept = default;

	/// See co_timer_0_service_0_destroy().
	/// @pre The timer service was not obtained from an event domain, which owns it.
	/// @pre All timers created by the timer service have been destroyed.
	void destroy() const noexcept
	{
		co_timer_0_service_0_destroy(m_handle);
	}

	operator service_0() const noexcept
	{
		return service_0(reinterpret_cast<co_service_0>(m_handle));
	}

	/// Creates a timer of one of the timer classes of the timer service class, selected by the timer service from its
	/// own state. Timers are created only through their service.
	result_0 create_timer_0(timer_0* timer) const noexcept
	{
		return static_cast<result_0>(co_timer_0_service_0_create_timer_0(m_handle, reinterpret_cast<co_timer_0*>(timer)));
	}

private:
	co_timer_0_service_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<timer_0_service_0, co_timer_0_service_0>);

template <>
struct cast_traits<timer_0_service_0>
{
	using base = service_0;
	using handle = co_timer_0_service_0;
};

template <>
struct cast_traits<co_timer_0_service_0>
{
	using base = service_0;
	using handle = co_timer_0_service_0;
};

/// See Handle casts in co.hpp.
/// @pre from is null or a timer service.
template <class From>
	requires handle_castable_to<timer_0_service_0, From>
[[nodiscard]] timer_0_service_0 timer_0_service_0_cast(From from) noexcept
{
	return handle_cast<timer_0_service_0>(from);
}

} // namespace co
