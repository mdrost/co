#pragma once

#include <co.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_executor_priority_0 Executor priority
/// @brief Scale of executor priorities.
///
/// Only the order of priorities is meaningful: CO_EXECUTOR_PRIORITY_0_NORMAL (0) is the normal priority, greater values
/// are more urgent and lower values less urgent. A class whose backend has N priority levels splits the range from
/// CO_EXECUTOR_PRIORITY_0_LOWEST to CO_EXECUTOR_PRIORITY_0_HIGHEST into N bands of equal width, in order, and maps every
/// priority of a band to its level. With an odd N the middle band is centered on CO_EXECUTOR_PRIORITY_0_NORMAL. Platform
/// headers define a constant at the center of each band, for example CO_WIN32_EXECUTOR_PRIORITY_0_HIGH
/// (co/win32/executor_priority_0.h) for the levels of TP_CALLBACK_PRIORITY.
/// @{

/// @brief Least urgent priority; the scale is symmetric around CO_EXECUTOR_PRIORITY_0_NORMAL, so INTPTR_MIN is not a
/// priority.
#define CO_EXECUTOR_PRIORITY_0_LOWEST (-INTPTR_MAX)
/// @brief Generic low priority, halfway between CO_EXECUTOR_PRIORITY_0_LOWEST and CO_EXECUTOR_PRIORITY_0_NORMAL.
#define CO_EXECUTOR_PRIORITY_0_LOW (-(INTPTR_MAX / 2))
/// @brief Normal priority, used when the next chain holds no co_executor_priority_info_0.
#define CO_EXECUTOR_PRIORITY_0_NORMAL ((intptr_t)0)
/// @brief Generic high priority, halfway between CO_EXECUTOR_PRIORITY_0_NORMAL and CO_EXECUTOR_PRIORITY_0_HIGHEST.
#define CO_EXECUTOR_PRIORITY_0_HIGH (INTPTR_MAX / 2)
/// @brief Most urgent priority.
#define CO_EXECUTOR_PRIORITY_0_HIGHEST (INTPTR_MAX)

/// @}

/// @defgroup c_executor_priority_info_0 Executor priority info
/// @brief Priority of the handlers posted to an executor relative to other work of the same execution context.
/// @{

/// @brief Structure type of co_executor_priority_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_EXECUTOR_PRIORITY_INFO_0;

/// @brief Priority of the handlers posted to the executor asked for by the next chain it is in.
///
/// Put it in the next chain of a co_executor_0_create_info_0. The priority is on the scale of co_executor_priority_0
/// (see CO_EXECUTOR_PRIORITY_0_NORMAL).
///
/// Put directly in the next chain, it is a requirement: only classes handling it are chosen, and a class declines
/// priorities it cannot honor with CO_RESULT_0_ERROR_NOT_SUPPORTED. Wrapped in a co_hint_info_0, it is a preference
/// that classes honor when they can and otherwise ignore.
/// @pre The next chain holds no other co_executor_priority_info_0, directly or as a hint.
typedef struct co_executor_priority_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Priority; CO_EXECUTOR_PRIORITY_0_NORMAL is normal, greater is more urgent.
	/// @pre Not INTPTR_MIN.
	intptr_t priority;
} co_executor_priority_info_0;

/// @}

#ifdef __cplusplus
}
#endif
