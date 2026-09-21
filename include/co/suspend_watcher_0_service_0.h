#pragma once

#include <co.h>
#include <co/suspend_watcher_0.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern CO_API
const co_service_type_0 CO_SERVICE_TYPE_0_SUSPEND_WATCHER_0_SERVICE_0;

// A co_service_0 found with CO_SERVICE_TYPE_0_SUSPEND_WATCHER_0_SERVICE_0 may be cast to co_suspend_watcher_0_service_0
// with CO_SUSPEND_WATCHER_0_SERVICE_0_CAST, and a co_suspend_watcher_0_service_0 to co_service_0 with CO_SERVICE_0_CAST.
// A suspend watcher service found in an event domain is owned by it and must not be destroyed by the user.
typedef struct co_suspend_watcher_0_service_0_t co_suspend_watcher_0_service_0_t;
typedef struct co_suspend_watcher_0_service_0_t* co_suspend_watcher_0_service_0;

// Casts a handle to co_suspend_watcher_0_service_0, unchecked; NULL stays NULL. See "Handles and casts" in Concepts.
#define CO_SUSPEND_WATCHER_0_SERVICE_0_CAST(handle) ((co_suspend_watcher_0_service_0)(handle))

/// @brief Same as co_service_0_destroy() on the suspend watcher service.
/// @pre suspend_watcher_service was not obtained from an event domain, which owns it.
/// @pre All suspend watchers created by the suspend watcher service have been destroyed.
static inline void co_suspend_watcher_0_service_0_destroy(co_suspend_watcher_0_service_0 suspend_watcher_service)
{
	co_service_0_destroy(CO_SERVICE_0_CAST(suspend_watcher_service));
}

/// @brief Creates a suspend watcher.
///
/// Every suspend watcher has its own waits and generations, both starting at 0, and counts every suspend and resume of
/// the system that happens after its creation, independently of the other suspend watchers of the service. A suspend
/// watcher may be shared by several consumers, each keeping its own generations (see suspend_watcher_0.h).
///
/// @retval CO_RESULT_0_SUCCESS The suspend watcher was created.
/// @retval CO_RESULT_0_ERROR_OUT_OF_MEMORY The suspend watcher cannot be allocated.
/// @pre suspend_watcher is not NULL.
CO_API
co_result_0 co_suspend_watcher_0_service_0_create_suspend_watcher_0(co_suspend_watcher_0_service_0 suspend_watcher_service, co_suspend_watcher_0* suspend_watcher) CO_NOEXCEPT;

/// @brief Type of co_suspend_watcher_0_service_0_vtable::create_suspend_watcher_0.
typedef co_result_0 (* co_suspend_watcher_0_service_0_create_suspend_watcher_0_fn)(co_suspend_watcher_0_service_0 self, co_suspend_watcher_0* suspend_watcher) CO_NOEXCEPT;

typedef struct co_suspend_watcher_0_service_0_vtable co_suspend_watcher_0_service_0_vtable;

/// @brief Start of every suspend watcher service object.
struct co_suspend_watcher_0_service_0_t
{
	/// @brief Start of the service.
	co_service_0_t service;
	/// @brief Methods of the class of the object.
	const co_suspend_watcher_0_service_0_vtable* vtable;
};

/// @brief Methods of a suspend watcher service class. Must stay valid while objects of the class exist.
struct co_suspend_watcher_0_service_0_vtable CO_FINAL
{
	/// @brief Required. Implements co_suspend_watcher_0_service_0_create_suspend_watcher_0().
	co_suspend_watcher_0_service_0_create_suspend_watcher_0_fn create_suspend_watcher_0;
};

/// @brief Initializes the start of a suspend watcher service object: stores its vtables.
static inline void co_suspend_watcher_0_service_0_init(co_suspend_watcher_0_service_0 suspend_watcher_service, const co_service_0_vtable* service_vtable, const co_suspend_watcher_0_service_0_vtable* suspend_watcher_service_vtable) CO_NOEXCEPT
{
	co_service_0_init(&suspend_watcher_service->service, service_vtable);
	suspend_watcher_service->vtable = suspend_watcher_service_vtable;
}

#ifdef __cplusplus
}
#endif
