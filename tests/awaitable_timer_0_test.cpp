#include <co/awaitable_timer_0.hpp>
#include <co/task_0.hpp>

#include "test_executors.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <coroutine>
#include <cstdint>
#include <exception>
#include <latch>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace
{

using co_test::manual_executor_0;

// Timer class whose starts elapse only when the test calls elapse(); completions are posted to the executor of the start.
// Like real timers, it may be cancelled from any thread, concurrently with its completion.
class fake_timer_0 final : public co_timer_0_t
{
public:

	fake_timer_0() noexcept
	{
		co_timer_0_init(this, &vtable);
	}

	// Simulates the timer elapsing: posts the pending completion with success.
	void elapse()
	{
		std::lock_guard lock(m_mutex);
		ASSERT_TRUE(m_executor) << "no start is pending";
		post(CO_RESULT_0_SUCCESS);
	}

	// Like elapse(), but does nothing when no start is pending, for example because it has been cancelled.
	void try_elapse() noexcept
	{
		std::lock_guard lock(m_mutex);
		if (m_executor) {
			post(CO_RESULT_0_SUCCESS);
		}
	}

	int cancel_count = 0;
	co_result_0 cancel_result = CO_RESULT_0_SUCCESS;

private:

	// Requires m_mutex.
	void post(co_result_0 status) noexcept
	{
		m_status = status;
		co_executor_0 executor = std::exchange(m_executor, nullptr);
		co_result_0 result = co_executor_0_post_0(executor, {.invoke_fn = &fake_timer_0::invoke, .data = this});
		if (result != CO_RESULT_0_SUCCESS) {
			std::terminate();
		}
	}

	static void invoke(void* data) noexcept
	{
		fake_timer_0* self = static_cast<fake_timer_0*>(data);
		co_timer_0_start_0_completion_fn completion_fn;
		void* completion_data;
		co_result_0 status;
		{
			std::lock_guard lock(self->m_mutex);
			completion_fn = std::exchange(self->m_completion_fn, nullptr);
			completion_data = std::exchange(self->m_data, nullptr);
			status = self->m_status;
		}
		completion_fn(completion_data, status);
	}

	static void destroy(co_timer_0 self) noexcept
	{
		delete static_cast<fake_timer_0*>(self);
	}

	static co_result_0 start_0(co_timer_0 self_, uint64_t /* due_ns */, co_executor_0 executor, co_timer_0_start_0_completion_fn completion_fn, void* data) noexcept
	{
		fake_timer_0* self = static_cast<fake_timer_0*>(self_);
		std::lock_guard lock(self->m_mutex);
		self->m_executor = executor;
		self->m_completion_fn = completion_fn;
		self->m_data = data;
		return CO_RESULT_0_SUCCESS;
	}

	static co_result_0 cancel_0(co_timer_0 self_) noexcept
	{
		fake_timer_0* self = static_cast<fake_timer_0*>(self_);
		std::lock_guard lock(self->m_mutex);
		++self->cancel_count;
		if (self->cancel_result != CO_RESULT_0_SUCCESS) {
			return self->cancel_result;
		}
		// Nothing to cancel when no start is pending or the timer has already elapsed.
		if (self->m_executor) {
			self->post(CO_RESULT_0_CANCELLED);
		}
		return CO_RESULT_0_SUCCESS;
	}

	static constexpr co_timer_0_vtable vtable = {
		.api_version = CO_API_VERSION_0,
		.destroy = &fake_timer_0::destroy,
		.start_0 = &fake_timer_0::start_0,
		.cancel_0 = &fake_timer_0::cancel_0,
	};

	std::mutex m_mutex;
	co_executor_0 m_executor = nullptr;
	co_timer_0_start_0_completion_fn m_completion_fn = nullptr;
	void* m_data = nullptr;
	co_result_0 m_status = CO_RESULT_0_SUCCESS;
};

// Coroutine that starts eagerly and destroys itself when it finishes.
struct detached_0 final
{
	struct promise_type final
	{
		detached_0 get_return_object() noexcept { return {}; }
		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void return_void() noexcept {}
		void unhandled_exception() noexcept { std::terminate(); }
	};
};

class cpp_awaitable_timer_0 : public ::testing::Test
{
protected:

	void SetUp() override
	{
		fake = new fake_timer_0();
		executor = new manual_executor_0();
	}

	void TearDown() override
	{
		co::timer_0(fake).destroy();
		co_executor_0_destroy(executor);
	}

	co::executor_0 get_executor() const noexcept
	{
		return co::executor_0(executor);
	}

	fake_timer_0* fake = nullptr;
	manual_executor_0* executor = nullptr;
};

TEST_F(cpp_awaitable_timer_0, async_start_resumes_with_success_when_elapsed)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	co::result_0 result = co::result_0::error_system;
	bool done = false;
	[](co::awaitable_timer_0& timer, co::executor_0 executor, co::result_0& result, bool& done) -> detached_0 {
		result = co_await timer.async_start_0(1000, executor);
		done = true;
	}(timer, get_executor(), result, done);

	EXPECT_TRUE(timer.pending());
	EXPECT_FALSE(done);
	fake->elapse();
	executor->run_all();
	EXPECT_TRUE(done);
	EXPECT_EQ(result, co::result_0::success);
	EXPECT_FALSE(timer.pending());
}

TEST_F(cpp_awaitable_timer_0, async_cancel_does_not_suspend_when_idle)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	bool done = false;
	[](co::awaitable_timer_0& timer, bool& done) -> detached_0 {
		co::result_0 result = co_await timer.async_cancel_0();
		EXPECT_EQ(result, co::result_0::success);
		done = true;
	}(timer, done);

	EXPECT_TRUE(done);
	EXPECT_EQ(fake->cancel_count, 0);
}

// The start does not know whether it will be cancelled; another coroutine later cancels and waits until idle.
TEST_F(cpp_awaitable_timer_0, async_cancel_waits_for_completion_of_pending_start)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	std::vector<std::string> log;
	[](co::awaitable_timer_0& timer, co::executor_0 executor, std::vector<std::string>& log) -> detached_0 {
		co::result_0 result = co_await timer.async_start_0(1000, executor);
		log.push_back(result == co::result_0::cancelled ? "start cancelled" : "start elapsed");
	}(timer, get_executor(), log);
	[](co::awaitable_timer_0& timer, std::vector<std::string>& log) -> detached_0 {
		co::result_0 result = co_await timer.async_cancel_0();
		log.push_back(result == co::result_0::success ? "idle" : "cancel failed");
		EXPECT_FALSE(timer.pending());
	}(timer, log);

	EXPECT_TRUE(log.empty());
	EXPECT_EQ(fake->cancel_count, 1);
	executor->run_all();
	EXPECT_EQ(log, (std::vector<std::string>{"start cancelled", "idle"}));
}

// cancel_0 finds nothing to cancel because the timer already elapsed; the waiter still waits for the queued completion.
TEST_F(cpp_awaitable_timer_0, async_cancel_waits_for_already_queued_completion)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	std::vector<std::string> log;
	[](co::awaitable_timer_0& timer, co::executor_0 executor, std::vector<std::string>& log) -> detached_0 {
		co::result_0 result = co_await timer.async_start_0(1000, executor);
		log.push_back(result == co::result_0::cancelled ? "start cancelled" : "start elapsed");
	}(timer, get_executor(), log);
	fake->elapse();
	[](co::awaitable_timer_0& timer, std::vector<std::string>& log) -> detached_0 {
		co_await timer.async_cancel_0();
		log.push_back("idle");
	}(timer, log);

	EXPECT_TRUE(log.empty());
	executor->run_all();
	EXPECT_EQ(log, (std::vector<std::string>{"start elapsed", "idle"}));
}

TEST_F(cpp_awaitable_timer_0, async_cancel_reports_cancel_error_without_suspending)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	bool start_done = false;
	[](co::awaitable_timer_0& timer, co::executor_0 executor, bool& done) -> detached_0 {
		co_await timer.async_start_0(1000, executor);
		done = true;
	}(timer, get_executor(), start_done);
	fake->cancel_result = CO_RESULT_0_ERROR_SYSTEM;
	co::result_0 result = co::result_0::success;
	[](co::awaitable_timer_0& timer, co::result_0& result) -> detached_0 {
		result = co_await timer.async_cancel_0();
	}(timer, result);

	EXPECT_EQ(result, co::result_0::error_system);
	EXPECT_TRUE(timer.pending());
	fake->elapse();
	executor->run_all();
	EXPECT_TRUE(start_done);
}

// After async_cancel_0 the timer is idle and may be started again.
TEST_F(cpp_awaitable_timer_0, timer_can_be_restarted_after_async_cancel)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	std::vector<co::result_0> results;
	[](co::awaitable_timer_0& timer, co::executor_0 executor, std::vector<co::result_0>& results) -> detached_0 {
		results.push_back(co_await timer.async_start_0(1000, executor));
	}(timer, get_executor(), results);
	[](co::awaitable_timer_0& timer, co::executor_0 executor, std::vector<co::result_0>& results) -> detached_0 {
		co_await timer.async_cancel_0();
		results.push_back(co_await timer.async_start_0(1000, executor));
	}(timer, get_executor(), results);
	executor->run_all();
	ASSERT_EQ(results, (std::vector<co::result_0>{co::result_0::cancelled}));

	EXPECT_TRUE(timer.pending());
	fake->elapse();
	executor->run_all();
	EXPECT_EQ(results, (std::vector<co::result_0>{co::result_0::cancelled, co::result_0::success}));
}

TEST_F(cpp_awaitable_timer_0, start_invokes_plain_completion_callback)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	co::result_0 status = co::result_0::error_system;
	ASSERT_EQ(timer.start_0(1000, get_executor(), [](void* data, co::result_0 status) noexcept {
		*static_cast<co::result_0*>(data) = status;
	}, &status), co::result_0::success);
	ASSERT_EQ(timer.cancel_0(), co::result_0::success);
	executor->run_all();
	EXPECT_EQ(status, co::result_0::cancelled);
	EXPECT_FALSE(timer.pending());
}

TEST_F(cpp_awaitable_timer_0, await_timer_does_not_suspend_when_idle)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	co::result_0 result = co::result_0::error_system;
	[](co::awaitable_timer_0& timer, co::result_0& result) -> detached_0 {
		result = co_await timer;
	}(timer, result);

	EXPECT_EQ(result, co::result_0::success);
}

// Many coroutines wait for the same start; they are resumed in order, after the start, with its status.
TEST_F(cpp_awaitable_timer_0, await_timer_resumes_many_waiters_with_status_of_start)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	std::vector<std::string> log;
	[](co::awaitable_timer_0& timer, co::executor_0 executor, std::vector<std::string>& log) -> detached_0 {
		co::result_0 result = co_await timer.async_start_0(1000, executor);
		log.push_back(result == co::result_0::success ? "start elapsed" : "start cancelled");
	}(timer, get_executor(), log);
	for (int i = 0; i < 3; ++i) {
		[](co::awaitable_timer_0& timer, int i, std::vector<std::string>& log) -> detached_0 {
			co::result_0 result = co_await timer;
			log.push_back("waiter " + std::to_string(i) + (result == co::result_0::success ? " elapsed" : " cancelled"));
			EXPECT_FALSE(timer.pending());
		}(timer, i, log);
	}

	EXPECT_TRUE(log.empty());
	EXPECT_EQ(fake->cancel_count, 0);
	fake->elapse();
	executor->run_all();
	EXPECT_EQ(log, (std::vector<std::string>{"start elapsed", "waiter 0 elapsed", "waiter 1 elapsed", "waiter 2 elapsed"}));
}

// Many coroutines cancel and wait, mixed with coroutines that only wait.
TEST_F(cpp_awaitable_timer_0, many_async_cancel_and_await_timer_waiters)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	std::vector<std::string> log;
	[](co::awaitable_timer_0& timer, co::executor_0 executor, std::vector<std::string>& log) -> detached_0 {
		co::result_0 result = co_await timer.async_start_0(1000, executor);
		log.push_back(result == co::result_0::cancelled ? "start cancelled" : "start elapsed");
	}(timer, get_executor(), log);
	[](co::awaitable_timer_0& timer, std::vector<std::string>& log) -> detached_0 {
		co::result_0 result = co_await timer;
		log.push_back(result == co::result_0::cancelled ? "waiter cancelled" : "waiter elapsed");
	}(timer, log);
	for (int i = 0; i < 2; ++i) {
		[](co::awaitable_timer_0& timer, int i, std::vector<std::string>& log) -> detached_0 {
			co::result_0 result = co_await timer.async_cancel_0();
			log.push_back("canceller " + std::to_string(i) + (result == co::result_0::success ? " idle" : " failed"));
		}(timer, i, log);
	}

	EXPECT_TRUE(log.empty());
	EXPECT_EQ(fake->cancel_count, 2);
	executor->run_all();
	EXPECT_EQ(log, (std::vector<std::string>{"start cancelled", "waiter cancelled", "canceller 0 idle", "canceller 1 idle"}));
}

// A failing async_cancel_0 removes only its own waiter; the other waiters still wait for the start.
TEST_F(cpp_awaitable_timer_0, failed_async_cancel_keeps_other_waiters)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	std::vector<std::string> log;
	[](co::awaitable_timer_0& timer, co::executor_0 executor, std::vector<std::string>& log) -> detached_0 {
		co_await timer.async_start_0(1000, executor);
		log.push_back("start");
	}(timer, get_executor(), log);
	auto waiter = [](co::awaitable_timer_0& timer, std::string name, std::vector<std::string>& log) -> detached_0 {
		co_await timer;
		log.push_back(name);
	};
	waiter(timer, "waiter 0", log);
	fake->cancel_result = CO_RESULT_0_ERROR_SYSTEM;
	[](co::awaitable_timer_0& timer, std::vector<std::string>& log) -> detached_0 {
		co::result_0 result = co_await timer.async_cancel_0();
		log.push_back(result == co::result_0::error_system ? "cancel failed" : "cancel idle");
	}(timer, log);
	waiter(timer, "waiter 1", log);

	EXPECT_EQ(log, (std::vector<std::string>{"cancel failed"}));
	fake->elapse();
	executor->run_all();
	EXPECT_EQ(log, (std::vector<std::string>{"cancel failed", "start", "waiter 0", "waiter 1"}));
}

// When the completion starts the timer again, the waiters of the completed start are resumed, and later waiters wait for
// the new start.
TEST_F(cpp_awaitable_timer_0, await_timer_after_restart_in_completion_waits_for_new_start)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	std::vector<std::string> log;
	[](co::awaitable_timer_0& timer, co::executor_0 executor, std::vector<std::string>& log) -> detached_0 {
		co_await timer.async_start_0(1000, executor);
		log.push_back("first start");
		co_await timer.async_start_0(1000, executor);
		log.push_back("second start");
	}(timer, get_executor(), log);
	[](co::awaitable_timer_0& timer, std::vector<std::string>& log) -> detached_0 {
		co_await timer;
		log.push_back("waiter");
		EXPECT_TRUE(timer.pending());
		co_await timer;
		log.push_back("waiter again");
	}(timer, log);

	fake->elapse();
	executor->run_all();
	EXPECT_EQ(log, (std::vector<std::string>{"first start", "waiter"}));
	fake->elapse();
	executor->run_all();
	EXPECT_EQ(log, (std::vector<std::string>{"first start", "waiter", "second start", "waiter again"}));
}

// Threads wait for and cancel a start concurrently with each other and with the timer elapsing on a thread pool, without
// any strand: every coroutine is resumed exactly once and the completion callback is invoked exactly once.
TEST_F(cpp_awaitable_timer_0, many_threads_wait_and_cancel_concurrently)
{
	constexpr int iterations = 300;
	constexpr int thread_count = 6;
	co_test::simple_thread_pool_0_executor_0* pool = new co_test::simple_thread_pool_0_executor_0(3);
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	for (int iteration = 0; iteration < iterations; ++iteration) {
		std::atomic<int> completions = 0;
		std::atomic<int> resumed = 0;
		ASSERT_EQ(timer.start_0(1000, co::executor_0(pool), [](void* data, co::result_0) noexcept {
			static_cast<std::atomic<int>*>(data)->fetch_add(1);
		}, &completions), co::result_0::success);

		std::latch go(thread_count + 2);
		std::vector<std::thread> threads;
		for (int i = 0; i < thread_count; ++i) {
			// Every other iteration, half of the threads cancel; otherwise the timer elapses.
			bool cancel = iteration % 2 == 1 && i % 2 == 0;
			threads.emplace_back([&timer, &go, &resumed, cancel]() {
				go.arrive_and_wait();
				[](co::awaitable_timer_0& timer, bool cancel, std::atomic<int>& resumed) -> detached_0 {
					if (cancel) {
						EXPECT_EQ(co_await timer.async_cancel_0(), co::result_0::success);
					}
					else {
						co::result_0 status = co_await timer;
						EXPECT_TRUE(status == co::result_0::success || status == co::result_0::cancelled);
					}
					resumed.fetch_add(1);
				}(timer, cancel, resumed);
			});
		}
		threads.emplace_back([this, &go]() {
			go.arrive_and_wait();
			fake->try_elapse();
		});
		go.arrive_and_wait();
		for (std::thread& thread : threads) {
			thread.join();
		}

		auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
		while ((resumed.load() != thread_count || completions.load() != 1) && std::chrono::steady_clock::now() < deadline) {
			std::this_thread::yield();
		}
		ASSERT_EQ(resumed.load(), thread_count) << "iteration " << iteration;
		ASSERT_EQ(completions.load(), 1) << "iteration " << iteration;
		ASSERT_FALSE(timer.pending()) << "iteration " << iteration;
	}
	co_executor_0_destroy(pool);
}

// Tests with co::task_0: the timer is driven from spawned tasks, which own their frames while suspended.
class cpp_awaitable_timer_0_task_0 : public cpp_awaitable_timer_0
{
protected:

	void SetUp() override
	{
		cpp_awaitable_timer_0::SetUp();
		other_executor = new manual_executor_0();
	}

	void TearDown() override
	{
		co_executor_0_destroy(other_executor);
		cpp_awaitable_timer_0::TearDown();
	}

	co::executor_0 get_other_executor() const noexcept
	{
		return co::executor_0(other_executor);
	}

	manual_executor_0* other_executor = nullptr;
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

// The start uses the executor of the task: the fake timer posts its completion there.
TEST_F(cpp_awaitable_timer_0_task_0, async_start_uses_executor_of_task)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	co::result_0 result = co::result_0::error_system;
	bool frame_destroyed = false;
	ASSERT_EQ(co::spawn(get_executor(), [](co::awaitable_timer_0& timer, co::result_0& result, bool& frame_destroyed) -> co::task_0<> {
		frame_guard guard{&frame_destroyed};
		result = co_await timer.async_start_0(1000);
	}(timer, result, frame_destroyed)), co::result_0::success);

	executor->run_all();
	EXPECT_TRUE(timer.pending());
	EXPECT_FALSE(frame_destroyed) << "the suspended root task must keep its frame";

	fake->elapse();
	EXPECT_EQ(executor->pending(), 1u) << "the completion must be posted to the executor of the task";
	executor->run_all();
	EXPECT_EQ(result, co::result_0::success);
	EXPECT_TRUE(frame_destroyed) << "the finished root task must destroy its frame";
}

// A child task awaited by a parent inherits its executor, and so does the timer start inside it.
TEST_F(cpp_awaitable_timer_0_task_0, async_start_in_child_task)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	int value = 0;
	struct tasks
	{
		static co::task_0<int> child(co::awaitable_timer_0& timer)
		{
			co::result_0 result = co_await timer.async_start_0(1000);
			co_return result == co::result_0::cancelled ? 2 : 1;
		}

		static co::task_0<> parent(co::awaitable_timer_0& timer, int& value)
		{
			value = co_await child(timer);
		}
	};
	ASSERT_EQ(co::spawn(get_executor(), tasks::parent(timer, value)), co::result_0::success);

	executor->run_all();
	ASSERT_TRUE(timer.pending());
	ASSERT_EQ(timer.cancel_0(), co::result_0::success);
	executor->run_all();
	EXPECT_EQ(value, 2);
}

// The task that cancels and waits is resumed on its own executor, not on the executor of the start.
TEST_F(cpp_awaitable_timer_0_task_0, async_cancel_resumes_waiter_on_its_own_executor)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	std::vector<std::string> log;
	ASSERT_EQ(co::spawn(get_executor(), [](co::awaitable_timer_0& timer, std::vector<std::string>& log) -> co::task_0<> {
		co::result_0 result = co_await timer.async_start_0(1000);
		log.push_back(result == co::result_0::cancelled ? "start cancelled" : "start elapsed");
	}(timer, log)), co::result_0::success);
	executor->run_all();
	ASSERT_TRUE(timer.pending());

	ASSERT_EQ(co::spawn(get_other_executor(), [](co::awaitable_timer_0& timer, std::vector<std::string>& log) -> co::task_0<> {
		co::result_0 result = co_await timer.async_cancel_0();
		log.push_back(result == co::result_0::success ? "idle" : "cancel failed");
	}(timer, log)), co::result_0::success);
	other_executor->run_all();
	EXPECT_TRUE(log.empty());

	executor->run_all();
	EXPECT_EQ(log, (std::vector<std::string>{"start cancelled"}));
	EXPECT_EQ(other_executor->pending(), 1u) << "the waiter must be posted to its own executor";
	other_executor->run_all();
	EXPECT_EQ(log, (std::vector<std::string>{"start cancelled", "idle"}));
}

// A task cancels a timer whose start it does not own, waits until it is idle and starts it again.
TEST_F(cpp_awaitable_timer_0_task_0, task_cancels_waits_and_restarts)
{
	co::awaitable_timer_0 timer{co::timer_0(fake)};
	co::result_0 plain_status = co::result_0::error_system;
	ASSERT_EQ(timer.start_0(1000, get_executor(), [](void* data, co::result_0 status) noexcept {
		*static_cast<co::result_0*>(data) = status;
	}, &plain_status), co::result_0::success);

	std::vector<co::result_0> results;
	ASSERT_EQ(co::spawn(get_executor(), [](co::awaitable_timer_0& timer, std::vector<co::result_0>& results) -> co::task_0<> {
		results.push_back(co_await timer.async_cancel_0());
		results.push_back(co_await timer.async_start_0(1000));
	}(timer, results)), co::result_0::success);
	executor->run_all();
	EXPECT_EQ(plain_status, co::result_0::cancelled);
	ASSERT_EQ(results, (std::vector<co::result_0>{co::result_0::success}));
	ASSERT_TRUE(timer.pending());

	fake->elapse();
	executor->run_all();
	EXPECT_EQ(results, (std::vector<co::result_0>{co::result_0::success, co::result_0::success}));
	EXPECT_FALSE(timer.pending());
}

} // namespace
