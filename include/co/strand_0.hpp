#pragma once

#include <co/strand_0.h>

#include <co.hpp>

namespace co
{

/// @defgroup cpp_strand_0_executor_0 Strand executor
/// @brief Executors that invoke posted handlers one at a time, in posting order, on top of another executor.
///
/// A strand is a built-in executor class. Create it with strand_0_executor_0_create_0(), which yields a
/// strand_0_executor_0 handle, post to it with strand_0_executor_0::post_0() and destroy it with
/// strand_0_executor_0::destroy().
///
/// - Handlers are invoked one at a time and in the order in which they were posted. A handler has finished its
///   invocation, which also releases what it owns, before the next one starts, so state accessed only from handlers
///   of the same strand needs no locking.
/// - Each handler is invoked by its own handler (job) of the inner executor, so it runs exactly in the context the inner
///   executor provides. Consecutive handlers may run on different threads of the inner executor.
/// - While it has handlers, the strand has exactly one job posted to, or running on, the inner executor. The job posts
///   the job for the next handler once its own handler has been invoked.
/// - post_0() never invokes the handler inline; it may be called from any thread, including from a handler of the strand.
/// - If the strand has no job and the inner executor rejects the new one, post_0() returns that error and the handler
///   stays with the caller.
/// - If the inner executor rejects the job for the next queued handler, std::terminate() is called.
/// - The inner executor must not invoke handlers inline from within its post_0().
///
/// The strand is idle when it has no queued handler and the inner executor holds or runs no job of the strand, for
/// example after the inner executor has invoked all of its handlers. The strand may be destroyed when it is idle, or
/// from within the invocation of one of its handlers when no other handler is queued, for example by the last handler
/// of an object owning the strand. The job then returns without accessing the strand. No post_0() call on the strand
/// may be in progress when it is destroyed.
/// @{

/// @brief Structure type of strand_0_executor_0_create_info_0.
extern CO_API
const structure_type_0 structure_type_0_strand_0_executor_0_create_info_0;

/// @brief Parameters of a strand executor.
struct strand_0_executor_0_create_info_0 final
{
	/// @brief Identifies the structure; set to structure_type_0_strand_0_executor_0_create_info_0.
	structure_type_0 structure_type;

	/// @brief Extension chain; may hold an allocation_callbacks_0.
	const in_structure_0* next_structure;

	/// @brief Executor on which the handlers are invoked. Must stay valid while the strand is not idle.
	/// @pre Not null.
	executor_0 inner_executor;
};

class strand_0_executor_0;

/// @cond
template <>
struct cast_traits<strand_0_executor_0>
{
	using base = executor_0;
	using handle = co_strand_0_executor_0;
};

template <>
struct cast_traits<co_strand_0_executor_0>
{
	using base = executor_0;
	using handle = co_strand_0_executor_0;
};
/// @endcond

/// @brief Handle of a strand executor; converts to executor_0 and is destroyed with destroy().
class strand_0_executor_0 final
{
public:

	using handler_0 = executor_handler_0;

	strand_0_executor_0() noexcept = default;

	strand_0_executor_0(std::nullptr_t) noexcept
	{
	}

	explicit strand_0_executor_0(co_strand_0_executor_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_strand_0_executor_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const strand_0_executor_0&, const strand_0_executor_0&) noexcept = default;

	operator executor_0() const noexcept
	{
		return executor_0_cast(m_handle);
	}

	/// @brief See co_strand_0_executor_0_destroy().
	void destroy() const noexcept
	{
		co_strand_0_executor_0_destroy(m_handle);
	}

	/// @brief See co_strand_0_executor_0_post_0().
	result_0 post_0(handler_0 handler) const noexcept
	{
		return static_cast<result_0>(co_strand_0_executor_0_post_0(m_handle, std::bit_cast<co_executor_handler_0>(handler)));
	}

	/// @brief See executor_0::post_0().
	template <class Handler>
		requires std::invocable<std::decay_t<Handler>&> && std::is_nothrow_constructible_v<std::decay_t<Handler>, Handler>
	result_0 post_0(Handler&& handler) const noexcept
	{
		using handler_type = std::decay_t<Handler>;
		handler_type* handlerx = new (std::nothrow) handler_type(std::forward<Handler>(handler));
		if (handlerx == nullptr) {
			return result_0::error_out_of_memory;
		}
		result_0 result = post_0(handler_0{.invoke_fn = &strand_0_executor_0::invoke<handler_type>, .data = handlerx});
		if (result != result_0::success) {
			delete handlerx;
		}
		return result;
	}

private:

	template <class Handler>
	static void invoke(void* data) noexcept
	{
		Handler* handler = static_cast<Handler*>(data);
		(*handler)();
		delete handler;
	}

private:
	co_strand_0_executor_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<strand_0_executor_0, co_strand_0_executor_0>);

/// @brief See Handle casts in co.hpp.
/// @pre from is null or a strand executor.
template <class From>
	requires handle_castable_to<strand_0_executor_0, From>
[[nodiscard]] strand_0_executor_0 strand_0_executor_0_cast(From from) noexcept
{
	return handle_cast<strand_0_executor_0>(from);
}

/// @brief See co_strand_0_executor_0_create_0().
inline result_0 strand_0_executor_0_create_0(instance_0 instance, const strand_0_executor_0_create_info_0& create_info, strand_0_executor_0* strand) noexcept
{
	return static_cast<result_0>(co_strand_0_executor_0_create_0(static_cast<co_instance_0>(instance), reinterpret_cast<const co_strand_0_executor_0_create_info_0*>(&create_info), reinterpret_cast<co_strand_0_executor_0*>(strand)));
}

/// @}

} // namespace co
