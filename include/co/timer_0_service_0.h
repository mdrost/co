#pragma once

#include <co.h>
#include <co/timer_0.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern CO_API
const co_service_type_0 CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0;

// A co_service_0 found with CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0 may be cast to co_timer_0_service_0 with
// CO_TIMER_0_SERVICE_0_CAST, and a co_timer_0_service_0 to co_service_0 with CO_SERVICE_0_CAST. A timer service found in
// an event domain is owned by it and must not be destroyed by the user.
typedef struct co_timer_0_service_0_t co_timer_0_service_0_t;
typedef struct co_timer_0_service_0_t* co_timer_0_service_0;

// Casts a handle to co_timer_0_service_0, unchecked; NULL stays NULL. See "Handles and casts" in Concepts.
#define CO_TIMER_0_SERVICE_0_CAST(handle) ((co_timer_0_service_0)(handle))

/// @brief Same as co_service_0_destroy() on the timer service.
/// @pre timer_service was not obtained from an event domain, which owns it.
/// @pre All timers created by the timer service have been destroyed.
static inline void co_timer_0_service_0_destroy(co_timer_0_service_0 timer_service)
{
	co_service_0_destroy(CO_SERVICE_0_CAST(timer_service));
}

// TODO: doc
CO_API
co_result_0 co_timer_0_service_0_create_timer_0(co_timer_0_service_0 timer_service, co_timer_0* timer) CO_NOEXCEPT;

// TODO: doc
typedef co_result_0 (* co_timer_0_service_0_create_timer_0_fn)(co_timer_0_service_0 self, co_timer_0* timer) CO_NOEXCEPT;

typedef struct co_timer_0_service_0_vtable co_timer_0_service_0_vtable;

// TODO: doc
struct co_timer_0_service_0_t
{
	// TODO: doc
	co_service_0_t service;
	// TODO: doc
	const co_timer_0_service_0_vtable* vtable;
};

// TODO: doc
struct co_timer_0_service_0_vtable CO_FINAL
{
	// TODO: doc
	co_timer_0_service_0_create_timer_0_fn create_timer_0;
};

// TODO: doc
static inline void co_timer_0_service_0_init(co_timer_0_service_0 timer_service, const co_service_0_vtable* service_vtable, const co_timer_0_service_0_vtable* timer_service_vtable) CO_NOEXCEPT
{
	co_service_0_init(&timer_service->service, service_vtable);
	timer_service->vtable = timer_service_vtable;
}

#ifdef __cplusplus
}
#endif
