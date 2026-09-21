#pragma once

#include <co/win32/clock_0.h>

#include <co.hpp>
#include <co/clock_0.hpp>

#include <cstddef>

namespace co
{

/// @defgroup cpp_win32_clock_0 Win32 clock
/// @brief Clock showing the system time (UTC) of Windows. See @ref c_win32_clock_0 for its contract.
/// @{

/// @brief Structure type of win32::clock_0_create_info_0.
extern CO_WIN32_API
const structure_type_0 structure_type_0_win32_clock_0_create_info_0;

namespace win32
{

/// @brief Parameters of a Win32 clock. See co_win32_clock_0_create_info_0.
struct clock_0_create_info_0 final
{
	/// @brief Identifies the structure; set to structure_type_0_win32_clock_0_create_info_0.
	structure_type_0 structure_type;

	/// @brief Extension chain; may hold an allocation_callbacks_0.
	const in_structure_0* next_structure;

	/// @brief Whether the base time advances while the system is suspended (sleep, hibernation).
	bool include_suspend_time;
};

/// @brief Handle of a Win32 clock; converts to co::clock_0 and is destroyed with destroy().
class clock_0 final
{
public:

	clock_0() noexcept = default;

	clock_0(std::nullptr_t) noexcept
	{
	}

	explicit clock_0(co_win32_clock_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_win32_clock_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const clock_0&, const clock_0&) noexcept = default;

	operator co::clock_0() const noexcept
	{
		return co::clock_0(CO_CLOCK_0_CAST(m_handle));
	}

	/// @brief See co_win32_clock_0_destroy().
	void destroy() const noexcept
	{
		co_win32_clock_0_destroy(m_handle);
	}

private:
	co_win32_clock_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<clock_0, co_win32_clock_0>);

/// @brief See co_win32_clock_0_create_0().
inline result_0 clock_0_create_0(const clock_0_create_info_0& create_info, clock_0* clock) noexcept
{
	return static_cast<result_0>(co_win32_clock_0_create_0(reinterpret_cast<const co_win32_clock_0_create_info_0*>(&create_info), reinterpret_cast<co_win32_clock_0*>(clock)));
}

} // namespace win32

/// @}

} // namespace co
