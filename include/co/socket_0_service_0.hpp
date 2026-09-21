#pragma once

#include <co/socket_0_service_0.h>

#include <co.hpp>

namespace co
{

extern CO_API
const service_type_0 service_type_0_socket_0_service_0;

template <>
struct cast_traits<co_socket_0_service_0>
{
	using base = service_0;
	using handle = co_socket_0_service_0;
};

} // namespace co
