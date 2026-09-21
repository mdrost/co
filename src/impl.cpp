#include <co.hpp>
#include <co/impl.hpp>

#include <cstddef>
#include <cstdint>
#include <new>

// Defined in one file so that each variable is initialized after the variables it reads.

namespace co
{

const structure_type_0 structure_type_0_allocation_callbacks_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_allocation_callbacks_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_allocation_callbacks_0)); }();

namespace co::impl
{

const allocation_callbacks_0 default_allocation_callbacks_0 = {
	.structure_type = structure_type_0_allocation_callbacks_0,
	.next_structure = nullptr,
	.allocate_fn = &default_allocate_0,
	.deallocate_fn = &default_deallocate_0,
	.data = nullptr,
};

void* default_allocate_0(void* /* data */, std::size_t size, std::size_t alignment) noexcept
{
	return ::operator new(size, std::align_val_t(alignment), std::nothrow);
}

void default_deallocate_0(void* /* data */, void* memory, std::size_t size, std::size_t alignment) noexcept
{
	::operator delete(memory, size, std::align_val_t(alignment));
}

} // namespace co::impl
