#pragma once

/// @defgroup cpp_impl Class implementation helpers
/// @brief Helpers for implementing classes (event domains, services, executors, factories) on top of the C API.
///
/// Everything here is built only on public structures. Except for the default allocation callbacks, which the library
/// defines and exports, it is header-only: each module (the library, an extension library, a plugin) compiles its own
/// copy and nothing crosses a module boundary.
///
/// Each helper takes the C++ structures of co.hpp; an overload taking the matching C structure of co.h forwards to it.

#include <co/impl.h>

#include <co.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace co::impl
{

/// @addtogroup cpp_impl
/// @{

static_assert(std::is_standard_layout_v<allocation_callbacks_0>);
static_assert(sizeof(co_allocation_callbacks_0) == sizeof(allocation_callbacks_0));
static_assert(offsetof(co_allocation_callbacks_0, structure_type) == offsetof(allocation_callbacks_0, structure_type));
static_assert(offsetof(co_allocation_callbacks_0, next_structure) == offsetof(allocation_callbacks_0, next_structure));
static_assert(offsetof(co_allocation_callbacks_0, allocate_fn) == offsetof(allocation_callbacks_0, allocate_fn));
static_assert(offsetof(co_allocation_callbacks_0, deallocate_fn) == offsetof(allocation_callbacks_0, deallocate_fn));
static_assert(offsetof(co_allocation_callbacks_0, data) == offsetof(allocation_callbacks_0, data));

/// Allocation callbacks using the global operator new and delete of the library, used when a next chain holds none.
extern CO_API
const allocation_callbacks_0 default_allocation_callbacks_0;

/// Allocates with the global aligned nothrow operator new of the library; allocate_fn of default_allocation_callbacks_0.
CO_API
void* default_allocate_0(void* data, std::size_t size, std::size_t alignment) noexcept;

/// Deallocates with the global aligned sized operator delete of the library; deallocate_fn of
/// default_allocation_callbacks_0.
CO_API
void default_deallocate_0(void* data, void* memory, std::size_t size, std::size_t alignment) noexcept;

/// Allocates size bytes aligned to alignment with callbacks; returns null when the allocation fails.
/// @pre alignment is a power of two.
inline void* allocate_0(const allocation_callbacks_0& callbacks, std::size_t size, std::size_t alignment) noexcept
{
	void* memory = callbacks.allocate_fn(callbacks.data, size, alignment);
	assert((reinterpret_cast<std::uintptr_t>(memory) & (alignment - 1)) == 0 && "allocate_fn must return memory aligned as requested");
	return memory;
}

/// Allocates size bytes aligned to alignment with callbacks; returns null when the allocation fails.
/// @pre alignment is a power of two.
inline void* allocate_0(const co_allocation_callbacks_0& callbacks, std::size_t size, std::size_t alignment) noexcept
{
	void* memory = callbacks.allocate_fn(callbacks.data, size, alignment);
	assert((reinterpret_cast<std::uintptr_t>(memory) & (alignment - 1)) == 0 && "allocate_fn must return memory aligned as requested");
	return memory;
}

/// Deallocates memory returned by allocate_0() with the same callbacks, size and alignment.
inline void deallocate_0(const allocation_callbacks_0& callbacks, void* memory, std::size_t size, std::size_t alignment) noexcept
{
	callbacks.deallocate_fn(callbacks.data, memory, size, alignment);
}

/// Deallocates memory returned by allocate_0() with the same callbacks, size and alignment.
inline void deallocate_0(const co_allocation_callbacks_0& callbacks, void* memory, std::size_t size, std::size_t alignment) noexcept
{
	callbacks.deallocate_fn(callbacks.data, memory, size, alignment);
}

/// Allocates and constructs a T with allocation_callbacks; returns null when the allocation fails.
template <class T, class... Args>
T* new_object_0(const allocation_callbacks_0& allocation_callbacks, Args&&... args) noexcept
{
	static_assert(std::is_nothrow_constructible_v<T, Args...>);
	void* memory = allocate_0(allocation_callbacks, sizeof(T), alignof(T));
	if (memory == nullptr) {
		return nullptr;
	}
	return ::new (memory) T(std::forward<Args>(args)...);
}

/// Allocates and constructs a T with allocation_callbacks; returns null when the allocation fails.
template <class T, class... Args>
T* new_object_0(const co_allocation_callbacks_0& allocation_callbacks, Args&&... args) noexcept
{
	static_assert(std::is_nothrow_constructible_v<T, Args...>);
	void* memory = allocate_0(allocation_callbacks, sizeof(T), alignof(T));
	if (memory == nullptr) {
		return nullptr;
	}
	return ::new (memory) T(std::forward<Args>(args)...);
}

/// Destroys and deallocates a T allocated by new_object_0() with allocation_callbacks, which may live in the object.
template <class T>
void delete_object_0(const allocation_callbacks_0& allocation_callbacks, T* object) noexcept
{
	const allocation_callbacks_0 callbacks = allocation_callbacks;
	std::destroy_at(object);
	deallocate_0(callbacks, object, sizeof(T), alignof(T));
}

/// Destroys and deallocates a T allocated by new_object_0() with allocation_callbacks, which may live in the object.
template <class T>
void delete_object_0(const co_allocation_callbacks_0& allocation_callbacks, T* object) noexcept
{
	const co_allocation_callbacks_0 callbacks = allocation_callbacks;
	std::destroy_at(object);
	deallocate_0(callbacks, object, sizeof(T), alignof(T));
}

/// @}

} // namespace co::impl
