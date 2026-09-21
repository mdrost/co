#pragma once

#include <co.hpp>

#include <cassert>
#include <concepts>
#include <coroutine>
#include <exception>
#include <optional>
#include <type_traits>
#include <utility>

namespace co
{

// task_0 is a header-only coroutine type for use within one module (executable or shared library). Its coroutine
// frame is laid out by the compiler, and its promise layout and protocol (continuation, detaching, result) are inline
// code compiled into every user, so a task_0 must not cross module boundaries, such as between the library and a
// plugin module or between modules built with different compilers or headers. Across module boundaries, use the C API
// (executors, handlers and completion callbacks) and wrap it in awaiters on each side, as awaitable_timer_0 wraps
// timer_0. Once released, the layout and protocol of task_0 are frozen; changes go to a new task_1.

template <class T = void>
class task_0;

template <class T>
result_0 spawn(executor_0 executor, task_0<T>&& task) noexcept;

template <class T>
class task_0
{
public:

	class promise_type;

private:

	using coroutine_handle_type = std::coroutine_handle<promise_type>;

public:

	class promise_type
	{
	public:

		// The executor is not bound at construction; it is assigned by
		// spawn() for a root task, or inherited from the awaiting
		// coroutine (see await_transform) for a child task. It must be bound
		// before the coroutine is ever resumed.
		promise_type() noexcept = default;

		[[nodiscard]] executor_0 get_executor() const noexcept
		{
			return m_executor;
		}

		// Child tasks without their own executor inherit this coroutine's executor.
		template <class U>
		[[nodiscard]] task_0<U>&& await_transform(task_0<U>&& awaited) noexcept
		{
			if (!awaited.get_executor()) {
				awaited.set_executor(m_executor);
			}
			return std::move(awaited);
		}

		template <class U>
		[[nodiscard]] task_0<U>& await_transform(task_0<U>& awaited) noexcept
		{
			if (!awaited.get_executor()) {
				awaited.set_executor(m_executor);
			}
			return awaited;
		}

		template <class Awaitable>
		[[nodiscard]] Awaitable&& await_transform(Awaitable&& awaitable) noexcept
		{
			return std::forward<Awaitable>(awaitable);
		}

		[[nodiscard]] task_0 get_return_object()
		{
			return task_0(coroutine_handle_type::from_promise(*this));
		}

		[[nodiscard]] constexpr std::suspend_always initial_suspend() const noexcept
		{
			return {};
		}

		private:

			class final_awaiter
			{
			public:

				[[nodiscard]] constexpr bool await_ready() const noexcept
				{
					return false;
				}

				void await_suspend(coroutine_handle_type completed_coroutine_handle) noexcept
				{
					auto& completed_promise = completed_coroutine_handle.promise();
					// A root task started by spawn() owns its frame: nobody awaits its result, so it destroys itself.
					if (completed_promise.m_detached) {
						if (completed_promise.m_exception) {
							std::terminate();
						}
						completed_coroutine_handle.destroy();
						return;
					}
					// A task that was never awaited has no continuation to resume.
					auto continuation = std::exchange(completed_promise.m_continuation, {});
					if (!continuation) {
						return;
					}
					auto executor = completed_promise.get_executor();
					assert(executor && "a running task always has a bound executor");
					executor.post_0(continuation);
				}

				constexpr void await_resume() const noexcept
				{
				}
			};

		public:

			[[nodiscard]] constexpr final_awaiter final_suspend() const noexcept
			{
				return {};
			}

			template<class U>
				requires std::convertible_to<U, T>
			constexpr void return_value(U&& v)
			{
				m_value.emplace(std::forward<U>(v));
			}

			void unhandled_exception() noexcept
			{
				m_exception = std::current_exception();
			/*
			try {
				throw;
			}
			catch (const std::exception& ex) {
				std::cout << "Unhandled coroutine exception: " << ex.what() << '\n';
			}
			catch (...) {
				std::cout << "Unhandled coroutine exception: unknown\n";
			}
			std::terminate();
			*/
		}

	private:

		friend class task_0;

		void set_executor(executor_0 executor) noexcept
		{
			assert(executor && "a task may not be bound to a null executor");
			assert((!m_executor || m_executor == executor) && "a task executor may not be rebound");
			m_executor = executor;
		}

		executor_0 m_executor;
		std::optional<T> m_value;
		std::exception_ptr m_exception;
		std::coroutine_handle<> m_continuation;
		// Set by spawn(): the frame is owned by the coroutine itself and destroyed at its final suspension point.
		bool m_detached = false;
	};

public:

	task_0(const task_0& other) noexcept = delete;

	task_0(task_0&& other) noexcept
		: m_coroutine_handle(std::exchange(other.m_coroutine_handle, {}))
	{
	}

	task_0& operator=(const task_0& other) noexcept = delete;

	task_0& operator=(task_0&& other) noexcept
	{
		if (this != &other) {
			if (m_coroutine_handle) {
				m_coroutine_handle.destroy();
			}
			m_coroutine_handle = std::exchange(other.m_coroutine_handle, {});
		}
		return *this;
	}

	~task_0()
	{
		if (m_coroutine_handle) {
			m_coroutine_handle.destroy();
		}
	}

	[[nodiscard]] constexpr bool done() const noexcept
	{
		return !m_coroutine_handle || m_coroutine_handle.done();
	}

	[[nodiscard]] executor_0 get_executor() const noexcept
	{
		assert(m_coroutine_handle && "get_executor() called on a moved-from task");
		return m_coroutine_handle.promise().get_executor();
	}

	// awaiter interface:

	[[nodiscard]] constexpr bool await_ready() const noexcept
	{
		return !m_coroutine_handle || m_coroutine_handle.done();
	}

	void await_suspend(std::coroutine_handle<> continuation) noexcept
	{
		auto& promise = m_coroutine_handle.promise();
		promise.m_continuation = continuation;
		auto executor = promise.get_executor();
		assert(executor && "a task must be bound to an executor before it is awaited");
		executor.post_0(m_coroutine_handle);
	}

	[[nodiscard]] T await_resume()
	{
		auto& promise = m_coroutine_handle.promise();
		if (promise.m_exception) {
			std::rethrow_exception(promise.m_exception);
		}
		return std::move(*promise.m_value);
	}

private:

	// Promises of all tasks bind the executors of the child tasks they await.
	template <class>
	friend class task_0;

	template <class U>
	friend result_0 spawn(executor_0 executor, task_0<U>&& task) noexcept;

	explicit task_0(coroutine_handle_type coroutine_handle) noexcept
		: m_coroutine_handle(coroutine_handle)
	{
	}

	void set_executor(executor_0 executor) noexcept
	{
		assert(m_coroutine_handle && "set_executor() called on a moved-from task");
		m_coroutine_handle.promise().set_executor(executor);
	}

	// Binds the task to executor and gives up ownership of its frame, which then destroys itself when it finishes.
	[[nodiscard]] coroutine_handle_type detach(executor_0 executor) noexcept
	{
		set_executor(executor);
		m_coroutine_handle.promise().m_detached = true;
		return std::exchange(m_coroutine_handle, {});
	}

	coroutine_handle_type m_coroutine_handle;
};

template <>
class task_0<void>
{
public:

	class promise_type;

private:

	using coroutine_handle_type = std::coroutine_handle<promise_type>;

public:

	class promise_type
	{
	public:

		// The executor is not bound at construction; it is assigned by
		// spawn() for a root task, or inherited from the awaiting
		// coroutine (see await_transform) for a child task. It must be bound
		// before the coroutine is ever resumed.
		promise_type() noexcept = default;

		[[nodiscard]] executor_0 get_executor() const noexcept
		{
			return m_executor;
		}

		// Child tasks without their own executor inherit this coroutine's executor.
		template <class U>
		[[nodiscard]] task_0<U>&& await_transform(task_0<U>&& awaited) noexcept
		{
			if (!awaited.get_executor()) {
				awaited.set_executor(m_executor);
			}
			return std::move(awaited);
		}

		template <class U>
		[[nodiscard]] task_0<U>& await_transform(task_0<U>& awaited) noexcept
		{
			if (!awaited.get_executor()) {
				awaited.set_executor(m_executor);
			}
			return awaited;
		}

		template <class Awaitable>
		[[nodiscard]] Awaitable&& await_transform(Awaitable&& awaitable) noexcept
		{
			return std::forward<Awaitable>(awaitable);
		}

		[[nodiscard]] task_0 get_return_object()
		{
			return task_0(coroutine_handle_type::from_promise(*this));
		}

		[[nodiscard]] constexpr std::suspend_always initial_suspend() const noexcept
		{
			return {};
		}

		private:

			class final_awaiter
			{
			public:

				[[nodiscard]] constexpr bool await_ready() const noexcept
				{
					return false;
				}

				void await_suspend(coroutine_handle_type completed_coroutine_handle) noexcept
				{
					auto& completed_promise = completed_coroutine_handle.promise();
					// A root task started by spawn() owns its frame: nobody awaits its result, so it destroys itself.
					if (completed_promise.m_detached) {
						if (completed_promise.m_exception) {
							std::terminate();
						}
						completed_coroutine_handle.destroy();
						return;
					}
					// A task that was never awaited has no continuation to resume.
					auto continuation = std::exchange(completed_promise.m_continuation, {});
					if (!continuation) {
						return;
					}
					auto executor = completed_promise.get_executor();
					assert(executor && "a running task always has a bound executor");
					executor.post_0(continuation);
				}

				constexpr void await_resume() const noexcept
				{
				}
			};

		public:

			[[nodiscard]] constexpr final_awaiter final_suspend() const noexcept
			{
				return {};
			}

			constexpr void return_void() const noexcept
			{
			}

			void unhandled_exception() noexcept
			{
				m_exception = std::current_exception();
			/*
			try {
				throw;
			}
			catch (const std::exception& ex) {
				std::cout << "Unhandled coroutine exception: " << ex.what() << '\n';
			}
			catch (...) {
				std::cout << "Unhandled coroutine exception: unknown\n";
			}
			std::terminate();
			*/
		}

	private:

		friend class task_0;

		void set_executor(executor_0 executor) noexcept
		{
			assert(executor && "a task may not be bound to a null executor");
			assert((!m_executor || m_executor == executor) && "a task executor may not be rebound");
			m_executor = executor;
		}

		executor_0 m_executor;
		std::exception_ptr m_exception;
		std::coroutine_handle<> m_continuation;
		// Set by spawn(): the frame is owned by the coroutine itself and destroyed at its final suspension point.
		bool m_detached = false;
	};

public:

	task_0(const task_0& other) noexcept = delete;

	task_0(task_0&& other) noexcept
		: m_coroutine_handle(std::exchange(other.m_coroutine_handle, {}))
	{
	}

	task_0& operator=(const task_0& other) noexcept = delete;

	task_0& operator=(task_0&& other) noexcept
	{
		if (this != &other) {
			if (m_coroutine_handle) {
				m_coroutine_handle.destroy();
			}
			m_coroutine_handle = std::exchange(other.m_coroutine_handle, {});
		}
		return *this;
	}

	~task_0()
	{
		if (m_coroutine_handle) {
			m_coroutine_handle.destroy();
		}
	}

	[[nodiscard]] constexpr bool done() const noexcept
	{
		return !m_coroutine_handle || m_coroutine_handle.done();
	}

	[[nodiscard]] executor_0 get_executor() const noexcept
	{
		assert(m_coroutine_handle && "get_executor() called on a moved-from task");
		return m_coroutine_handle.promise().get_executor();
	}

	// awaiter interface:

	[[nodiscard]] constexpr bool await_ready() const noexcept
	{
		return !m_coroutine_handle || m_coroutine_handle.done();
	}

	void await_suspend(std::coroutine_handle<> continuation) noexcept
	{
		auto& promise = m_coroutine_handle.promise();
		promise.m_continuation = continuation;
		auto executor = promise.get_executor();
		assert(executor && "a task must be bound to an executor before it is awaited");
		executor.post_0(m_coroutine_handle);
	}

	void await_resume()
	{
		auto& promise = m_coroutine_handle.promise();
		if (promise.m_exception) {
			std::rethrow_exception(promise.m_exception);
		}
	}

private:

	// Promises of all tasks bind the executors of the child tasks they await.
	template <class>
	friend class task_0;

	template <class U>
	friend result_0 spawn(executor_0 executor, task_0<U>&& task) noexcept;

	explicit task_0(coroutine_handle_type coroutine_handle) noexcept
		: m_coroutine_handle(coroutine_handle)
	{
	}

	void set_executor(executor_0 executor) noexcept
	{
		assert(m_coroutine_handle && "set_executor() called on a moved-from task");
		m_coroutine_handle.promise().set_executor(executor);
	}

	// Binds the task to executor and gives up ownership of its frame, which then destroys itself when it finishes.
	[[nodiscard]] coroutine_handle_type detach(executor_0 executor) noexcept
	{
		set_executor(executor);
		m_coroutine_handle.promise().m_detached = true;
		return std::exchange(m_coroutine_handle, {});
	}

	coroutine_handle_type m_coroutine_handle;
};

namespace detail
{

inline void resume_coroutine(void* address) noexcept
{
	std::coroutine_handle<>::from_address(address).resume();
}

} // namespace detail

// Starts a root task on the specified executor. The task owns its coroutine frame from then on and destroys it when it
// finishes; an exception escaping it terminates the program. If the executor rejects the task, the frame is destroyed
// and the error is returned.
// @pre executor is not null.
template <class T>
result_0 spawn(executor_0 executor, task_0<T>&& task) noexcept
{
	std::coroutine_handle<typename task_0<T>::promise_type> coroutine_handle = task.detach(executor);
	result_0 result = executor.post_0(executor_0::handler_0{.invoke_fn = &detail::resume_coroutine, .data = coroutine_handle.address()});
	if (result != result_0::success) {
		coroutine_handle.destroy();
	}
	return result;
}

} // namespace co
