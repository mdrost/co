#pragma once

#include <co/executor_priority_0.h>

#include <co.hpp>

#include <cstdint>

namespace co
{

/// @defgroup cpp_executor_priority_0 Executor priority
/// @brief Scale of executor priorities; see co_executor_priority_0.
/// @{

/// @brief See CO_EXECUTOR_PRIORITY_0_LOWEST.
inline constexpr std::intptr_t executor_priority_0_lowest = CO_EXECUTOR_PRIORITY_0_LOWEST;
/// @brief See CO_EXECUTOR_PRIORITY_0_LOW.
inline constexpr std::intptr_t executor_priority_0_low = CO_EXECUTOR_PRIORITY_0_LOW;
/// @brief See CO_EXECUTOR_PRIORITY_0_NORMAL.
inline constexpr std::intptr_t executor_priority_0_normal = CO_EXECUTOR_PRIORITY_0_NORMAL;
/// @brief See CO_EXECUTOR_PRIORITY_0_HIGH.
inline constexpr std::intptr_t executor_priority_0_high = CO_EXECUTOR_PRIORITY_0_HIGH;
/// @brief See CO_EXECUTOR_PRIORITY_0_HIGHEST.
inline constexpr std::intptr_t executor_priority_0_highest = CO_EXECUTOR_PRIORITY_0_HIGHEST;

/// @}

/// @defgroup cpp_executor_priority_info_0 Executor priority info
/// @brief Priority of the handlers posted to an executor relative to other work of the same execution context.
/// @{

/// @brief Structure type of executor_priority_info_0.
extern CO_API
const structure_type_0 structure_type_0_executor_priority_info_0;

/// @brief Priority
/// requirement or wrapped in a hint_info_0 as a preference. See co_executor_priority_info_0.
struct executor_priority_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	/// @brief Priority; executor_priority_0_normal is normal, greater is more urgent.
	/// @pre Not INTPTR_MIN.
	std::intptr_t priority;
};

/// @}

} // namespace co
