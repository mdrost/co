#pragma once

/// @defgroup cpp_impl Class implementation helpers
/// @brief Helpers for implementing classes (event domains, services, executors, factories) on top of the C API.
///
/// Everything here is header-only and built only on public C structures, so each module (the library, an extension
/// library, a plugin) compiles its own copy and nothing crosses a module boundary.

#include <co.h>

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

/// Allocates size bytes aligned to alignment with callbacks; returns null when the allocation fails.
/// @pre alignment is a power of two.
inline void* allocate_0(const co_allocation_callbacks_0& callbacks, std::size_t size, std::size_t alignment) noexcept
{
	void* memory = callbacks.allocate_fn(callbacks.data, size, alignment);
	assert((reinterpret_cast<std::uintptr_t>(memory) & (alignment - 1)) == 0 && "allocate_fn must return memory aligned as requested");
	return memory;
}

/// Deallocates memory returned by allocate_0() with the same callbacks, size and alignment.
inline void deallocate_0(const co_allocation_callbacks_0& callbacks, void* memory, std::size_t size, std::size_t alignment) noexcept
{
	callbacks.deallocate_fn(callbacks.data, memory, size, alignment);
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
void delete_object_0(const co_allocation_callbacks_0& allocation_callbacks, T* object) noexcept
{
	const co_allocation_callbacks_0 callbacks = allocation_callbacks;
	std::destroy_at(object);
	deallocate_0(callbacks, object, sizeof(T), alignof(T));
}

/// @}

} // namespace co::impl
