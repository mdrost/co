#pragma once

// Test executor classes: objects starting with a co_executor_0_t whose vtable is installed by their constructor.

#include <co.hpp>

#include <gtest/gtest.h>

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

namespace co_test
{

// Vtable of an executor class T deriving from co_executor_0_t, allocated with new, with a
// co_result_0 post(co_executor_handler_0) noexcept member function.
template <class T>
inline constexpr co_executor_0_vtable executor_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = [](co_executor_0 self) noexcept { delete static_cast<T*>(self); },
	.post_0 = [](co_executor_0 self, co_executor_handler_0 handler) noexcept { return static_cast<T*>(self)->post(handler); },
};

// Executor whose handlers run only when the test calls run_one().
class manual_executor_0 final : public co_executor_0_t
{
public:

	manual_executor_0() noexcept
	{
		co_executor_0_init(this, &executor_vtable<manual_executor_0>);
	}

	size_t pending()
	{
		std::lock_guard lock(m_mutex);
		return m_handlers.size();
	}

	bool run_one()
	{
		std::unique_lock lock(m_mutex);
		if (m_handlers.empty()) {
			return false;
		}
		co_executor_handler_0 handler = m_handlers.front();
		m_handlers.pop_front();
		lock.unlock();
		handler.invoke_fn(handler.data);
		return true;
	}

	void run_all()
	{
		while (run_one()) {
		}
	}

	co::result_0 reject = co::result_0::success;

	co_result_0 post(co_executor_handler_0 handler) noexcept
	{
		if (reject != co::result_0::success) {
			return static_cast<co_result_0>(reject);
		}
		std::lock_guard lock(m_mutex);
		m_handlers.push_back(handler);
		return CO_RESULT_0_SUCCESS;
	}

private:
	std::mutex m_mutex;
	std::deque<co_executor_handler_0> m_handlers;
};

// Executor running handlers concurrently on a fixed set of threads; destruction runs all remaining handlers.
class simple_thread_pool_0_executor_0 final : public co_executor_0_t
{
public:

	explicit simple_thread_pool_0_executor_0(size_t thread_count)
	{
		co_executor_0_init(this, &executor_vtable<simple_thread_pool_0_executor_0>);
		for (size_t i = 0; i < thread_count; ++i) {
			m_threads.emplace_back([this]() { work(); });
		}
	}

	~simple_thread_pool_0_executor_0()
	{
		{
			std::lock_guard lock(m_mutex);
			m_stop = true;
		}
		m_condition.notify_all();
		for (std::thread& thread : m_threads) {
			thread.join();
		}
	}

	co_result_0 post(co_executor_handler_0 handler) noexcept
	{
		{
			std::lock_guard lock(m_mutex);
			m_handlers.push_back(handler);
		}
		m_condition.notify_one();
		return CO_RESULT_0_SUCCESS;
	}

private:

	void work()
	{
		std::unique_lock lock(m_mutex);
		for (;;) {
			m_condition.wait(lock, [this]() { return m_stop || !m_handlers.empty(); });
			if (m_handlers.empty()) {
				return;
			}
			co_executor_handler_0 handler = m_handlers.front();
			m_handlers.pop_front();
			lock.unlock();
			handler.invoke_fn(handler.data);
			lock.lock();
		}
	}

private:
	std::mutex m_mutex;
	std::condition_variable m_condition;
	std::deque<co_executor_handler_0> m_handlers;
	bool m_stop = false;
	std::vector<std::thread> m_threads;
};

// Creates an instance and the test executors.
class instance_fixture : public ::testing::Test
{
protected:

	void SetUp() override
	{
		ASSERT_EQ(co::instance_0_create_0({
			.structure_type = co::structure_type_0_instance_0_create_info_0,
			.next_structure = nullptr,
			.api_version = co::api_version_0,
		}, &instance), co::result_0::success);
	}

	void TearDown() override
	{
		instance.destroy();
	}

	co::executor_0 create_manual(manual_executor_0** state = nullptr)
	{
		manual_executor_0* executor = new manual_executor_0();
		if (state) {
			*state = executor;
		}
		return co::executor_0(executor);
	}

	co::executor_0 create_thread_pool(size_t thread_count)
	{
		return co::executor_0(new simple_thread_pool_0_executor_0(thread_count));
	}

	co::instance_0 instance = nullptr;
};

} // namespace co_test
