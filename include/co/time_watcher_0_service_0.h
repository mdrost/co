#pragma once

#include <co.h>
#include <co/time_watcher_0.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern CO_API
const co_service_type_0 CO_SERVICE_TYPE_0_TIME_WATCHER_0_SERVICE_0;

// A co_service_0 found with CO_SERVICE_TYPE_0_TIME_WATCHER_0_SERVICE_0 may be cast to co_time_watcher_0_service_0 with
// CO_TIME_WATCHER_0_SERVICE_0_CAST, and a co_time_watcher_0_service_0 to co_service_0 with CO_SERVICE_0_CAST. A time
// watcher service found in an event domain is owned by it and must not be destroyed by the user.
typedef struct co_time_watcher_0_service_0_t co_time_watcher_0_service_0_t;
typedef struct co_time_watcher_0_service_0_t* co_time_watcher_0_service_0;

// Casts a handle to co_time_watcher_0_service_0, unchecked; NULL stays NULL. See "Handles and casts" in Concepts.
#define CO_TIME_WATCHER_0_SERVICE_0_CAST(handle) ((co_time_watcher_0_service_0)(handle))

/// @brief Same as co_service_0_destroy() on the time watcher service.
/// @pre time_watcher_service was not obtained from an event domain, which owns it.
/// @pre All time watchers created by the time watcher service have been destroyed.
static inline void co_time_watcher_0_service_0_destroy(co_time_watcher_0_service_0 time_watcher_service)
{
	co_service_0_destroy(CO_SERVICE_0_CAST(time_watcher_service));
}

/// @brief Creates a time watcher.
///
/// Every time watcher has its own waits and change generation, starting at 0, and counts every change of the system time
/// that happens after its creation, independently of the other time watchers of the service. A time watcher may be
/// shared by several consumers, each keeping its own generation (see time_watcher_0.h).
///
/// @retval CO_RESULT_0_SUCCESS The time watcher was created.
/// @retval CO_RESULT_0_ERROR_OUT_OF_MEMORY The time watcher cannot be allocated.
/// @pre time_watcher is not NULL.
CO_API
co_result_0 co_time_watcher_0_service_0_create_time_watcher_0(co_time_watcher_0_service_0 time_watcher_service, co_time_watcher_0* time_watcher) CO_NOEXCEPT;

/// @brief Type of co_time_watcher_0_service_0_vtable::create_time_watcher_0.
typedef co_result_0 (* co_time_watcher_0_service_0_create_time_watcher_0_fn)(co_time_watcher_0_service_0 self, co_time_watcher_0* time_watcher) CO_NOEXCEPT;

typedef struct co_time_watcher_0_service_0_vtable co_time_watcher_0_service_0_vtable;

/// @brief Start of every time watcher service object.
struct co_time_watcher_0_service_0_t
{
	/// @brief Start of the service.
	co_service_0_t service;
	/// @brief Methods of the class of the object.
	const co_time_watcher_0_service_0_vtable* vtable;
};

/// @brief Methods of a time watcher service class. Must stay valid while objects of the class exist.
struct co_time_watcher_0_service_0_vtable CO_FINAL
{
	/// @brief Required. Implements co_time_watcher_0_service_0_create_time_watcher_0().
	co_time_watcher_0_service_0_create_time_watcher_0_fn create_time_watcher_0;
};

/// @brief Initializes the start of a time watcher service object: stores its vtables.
static inline void co_time_watcher_0_service_0_init(co_time_watcher_0_service_0 time_watcher_service, const co_service_0_vtable* service_vtable, const co_time_watcher_0_service_0_vtable* time_watcher_service_vtable) CO_NOEXCEPT
{
	co_service_0_init(&time_watcher_service->service, service_vtable);
	time_watcher_service->vtable = time_watcher_service_vtable;
}

#ifdef __cplusplus
}
#endif
