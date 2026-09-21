#include "co.hpp"

// Rule: Make sure functions in this file are defined in the same order as in co.h file.

void co_service_0_destroy(co_service_0 service) noexcept
{
	service->vtable->destroy(service);
}
