#pragma once

#include <co/platform_thread_pool_0.h>

#include <co.hpp>

namespace co
{

/// @defgroup cpp_platform_thread_pool Platform thread pool
/// @brief Event facilities adapting the thread pool of the platform, the runtime the application runs on.
///
/// A platform thread pool is an abstract kind: there is no platform_thread_pool_0 type. Creating one with a
/// platform_thread_pool_0_create_info_0 yields an event_facility_0 of whichever class was chosen, which behaves as
/// described here.
/// @{

/// @brief Structure type of platform_thread_pool_0_create_info_0.
extern CO_API
const structure_type_0 structure_type_0_platform_thread_pool_0_create_info_0;

/// @brief Create info asking for a platform thread pool, in the next chain of an event_facility_0_create_info_0.
///
/// The platform is the operating system or an application framework such as Qt or GTK, whose thread pool the
/// application does not control (for example the default process thread pool on Windows). An event domain group class
/// declares it by listing structure_type_0_platform_thread_pool_0_create_info_0 among the handled types of its factory. On
/// Windows, the Win32 platform thread pool adapter event facility (co/win32/platform_thread_pool_adapter_0.hpp) is a
/// built-in class declaring it, with rank 0. See co_platform_thread_pool_0_create_info_0.
struct platform_thread_pool_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;
};

/// @}

} // namespace co
