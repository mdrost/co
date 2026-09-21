#include <co/task_0.hpp>

#include "test_executors.hpp"

#include <gtest/gtest.h>

#include <coroutine>

namespace
{

using co_test::manual_executor_0;

class cpp_task_0 : public ::testing::Test
{
protected:

	void SetUp() override
	{
		executor = new manual_executor_0();
	}

	void TearDown() override
	{
		co_executor_0_destroy(executor);
	}

	co::executor_0 get_executor() const noexcept
	{
		return co::executor_0(executor);
	}

	manual_executor_0* executor = nullptr;
};

// Sets a flag when the coroutine frame holding it is destroyed.
struct frame_guard final
{
	bool* destroyed;

	~frame_guard()
	{
		*destroyed = true;
	}
};

// Awaitable that suspends until the test resumes the stored coroutine.
struct manual_awaiter final
{
	std::coroutine_handle<>* stored;

	bool await_ready() const noexcept
	{
		return false;
	}

	void await_suspend(std::coroutine_handle<> coroutine) noexcept
	{
		*stored = coroutine;
	}

	void await_resume() const noexcept
	{
	}
};

TEST_F(cpp_task_0, spawn_keeps_frame_of_suspended_task_alive)
{
	std::coroutine_handle<> suspended;
	bool frame_destroyed = false;
	bool finished = false;
	ASSERT_EQ(co::spawn(get_executor(), [](std::coroutine_handle<>& suspended, bool& frame_destroyed, bool& finished) -> co::task_0<> {
		frame_guard guard{&frame_destroyed};
		co_await manual_awaiter{&suspended};
		finished = true;
	}(suspended, frame_destroyed, finished)), co::result_0::success);

	EXPECT_EQ(executor->pending(), 1u);
	executor->run_all();
	ASSERT_TRUE(suspended);
	EXPECT_FALSE(frame_destroyed);

	suspended.resume();
	EXPECT_TRUE(finished);
	EXPECT_TRUE(frame_destroyed);
}

TEST_F(cpp_task_0, spawn_destroys_frame_of_task_finishing_without_suspending)
{
	bool frame_destroyed = false;
	ASSERT_EQ(co::spawn(get_executor(), [](bool& frame_destroyed) -> co::task_0<int> {
		frame_guard guard{&frame_destroyed};
		co_return 1;
	}(frame_destroyed)), co::result_0::success);

	EXPECT_FALSE(frame_destroyed);
	executor->run_all();
	EXPECT_TRUE(frame_destroyed);
}

TEST_F(cpp_task_0, spawn_destroys_frame_when_executor_rejects)
{
	bool frame_destroyed = false;
	bool started = false;
	executor->reject = co::result_0::error_out_of_memory;
	EXPECT_EQ(co::spawn(get_executor(), [](bool& frame_destroyed, bool& started) -> co::task_0<> {
		frame_guard guard{&frame_destroyed};
		started = true;
		co_return;
	}(frame_destroyed, started)), co::result_0::error_out_of_memory);

	EXPECT_FALSE(started);
	EXPECT_EQ(executor->pending(), 0u);
}

TEST_F(cpp_task_0, spawned_task_awaits_child_on_same_executor)
{
	int value = 0;
	struct tasks
	{
		static co::task_0<int> child()
		{
			co_return 41;
		}

		static co::task_0<> parent(int& value)
		{
			value = co_await child() + 1;
		}
	};
	ASSERT_EQ(co::spawn(get_executor(), tasks::parent(value)), co::result_0::success);
	executor->run_all();
	EXPECT_EQ(value, 42);
}

} // namespace
