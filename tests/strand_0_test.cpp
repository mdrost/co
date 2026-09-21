#include <co/strand_0.h>
#include <co/strand_0.hpp>

#include "test_executors.hpp"
#include "test_guards.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <latch>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

namespace
{

using co_test::manual_executor_0;

class strand_test : public co_test::instance_fixture
{
protected:

	co::strand_0_executor_0 create_strand(co::executor_0 inner_executor)
	{
		co::strand_0_executor_0 strand = nullptr;
		co::result_0 result = co::strand_0_executor_0_create_0(instance, co::strand_0_executor_0_create_info_0{
			.structure_type = co::structure_type_0_strand_0_executor_0_create_info_0,
			.next_structure = nullptr,
			.inner_executor = inner_executor,
		}, &strand);
		EXPECT_EQ(result, co::result_0::success);
		return strand;
	}
};

using cpp_strand_0_executor_0 = strand_test;
using cpp_strand_0_executor_0_death = strand_test;
using c_strand_0_executor_0 = strand_test;

struct callback_log final
{
	std::vector<std::string> entries;
	std::string name;
};

void log_invoke(void* data) noexcept
{
	callback_log* log = static_cast<callback_log*>(data);
	log->entries.push_back("invoke " + log->name);
}

// Handler recording its invocation and destruction, to check where a handler is destroyed.
struct tracked_handler final
{
	explicit tracked_handler(std::vector<std::string>& entries, std::string name)
		: m_entries(&entries)
		, m_name(std::move(name))
	{}

	tracked_handler(tracked_handler&& other) noexcept
		: m_entries(std::exchange(other.m_entries, nullptr))
		, m_name(std::move(other.m_name))
	{}

	~tracked_handler()
	{
		if (m_entries) {
			m_entries->push_back("destroy " + m_name);
		}
	}

	void operator()() noexcept
	{
		m_entries->push_back("invoke " + m_name);
	}

	std::vector<std::string>* m_entries;
	std::string m_name;
};

} // namespace

static_assert(std::is_standard_layout_v<co_strand_0_executor_0_create_info_0>);
static_assert(offsetof(co_strand_0_executor_0_create_info_0, structure_type) == offsetof(co_in_structure_0, structure_type));
static_assert(offsetof(co_strand_0_executor_0_create_info_0, next_structure) == offsetof(co_in_structure_0, next_structure));
static_assert(std::is_standard_layout_v<co::strand_0_executor_0_create_info_0>);
static_assert(offsetof(co::strand_0_executor_0_create_info_0, structure_type) == offsetof(co::in_structure_0, structure_type));
static_assert(offsetof(co::strand_0_executor_0_create_info_0, next_structure) == offsetof(co::in_structure_0, next_structure));
static_assert(sizeof(co_strand_0_executor_0_create_info_0) == sizeof(co::strand_0_executor_0_create_info_0));
static_assert(offsetof(co_strand_0_executor_0_create_info_0, structure_type) == offsetof(co::strand_0_executor_0_create_info_0, structure_type));
static_assert(offsetof(co_strand_0_executor_0_create_info_0, next_structure) == offsetof(co::strand_0_executor_0_create_info_0, next_structure));
static_assert(offsetof(co_strand_0_executor_0_create_info_0, inner_executor) == offsetof(co::strand_0_executor_0_create_info_0, inner_executor));

TEST_F(cpp_strand_0_executor_0, create_fails_without_inner_executor)
{
	co::strand_0_executor_0 executor = nullptr;
	co::strand_0_executor_0_create_info_0 create_info = {
		.structure_type = co::structure_type_0_strand_0_executor_0_create_info_0,
		.next_structure = nullptr,
		.inner_executor = nullptr,
	};

	EXPECT_EQ(co::strand_0_executor_0_create_0(instance, create_info, &executor), co::result_0::error_invalid_argument);
	EXPECT_EQ(executor, nullptr);
}

TEST_F(cpp_strand_0_executor_0, casts_back_from_executor_0)
{
	co::executor_0 inner = create_manual();
	auto inner_guard = co_test::guard(inner);
	co::strand_0_executor_0 strand = nullptr;
	ASSERT_EQ(co::strand_0_executor_0_create_0(instance, co::strand_0_executor_0_create_info_0{
		.structure_type = co::structure_type_0_strand_0_executor_0_create_info_0,
		.next_structure = nullptr,
		.inner_executor = inner,
	}, &strand), co::result_0::success);
	auto strand_guard = co_test::guard(strand);

	co::executor_0 executor = strand;
	EXPECT_EQ(co::strand_0_executor_0_cast(executor), strand);
	EXPECT_EQ(co::strand_0_executor_0_cast(static_cast<co_executor_0>(executor)), strand);
	EXPECT_EQ(co::strand_0_executor_0_cast(static_cast<co_strand_0_executor_0>(strand)), strand);
	EXPECT_EQ(co::executor_0_cast(strand), executor);
	EXPECT_EQ(co::executor_0_cast(static_cast<co_strand_0_executor_0>(strand)), executor);
	EXPECT_EQ(co::strand_0_executor_0_cast(nullptr), nullptr);
	static_assert(!co::handle_castable_to<co::strand_0_executor_0, co::event_domain_0>);
	static_assert(!co::handle_castable_to<co::event_domain_0, co_strand_0_executor_0>);
}

TEST_F(cpp_strand_0_executor_0, create_fails_without_output)
{
	co::executor_0 inner = create_manual();
	auto inner_guard = co_test::guard(inner);
	co::strand_0_executor_0_create_info_0 create_info = {
		.structure_type = co::structure_type_0_strand_0_executor_0_create_info_0,
		.next_structure = nullptr,
		.inner_executor = inner,
	};

	EXPECT_EQ(co::strand_0_executor_0_create_0(instance, create_info, nullptr), co::result_0::error_invalid_argument);
}

TEST_F(cpp_strand_0_executor_0, post_0_does_not_invoke_inline)
{
	manual_executor_0* inner_state = nullptr;
	co::executor_0 inner_executor = create_manual(&inner_state);
	auto inner_executor_guard = co_test::guard(inner_executor);
	manual_executor_0& inner = *inner_state;
	co::strand_0_executor_0 strand = create_strand(inner_executor);
	auto strand_guard = co_test::guard(strand);
	bool invoked = false;

	EXPECT_EQ(strand.post_0([&invoked]() noexcept { invoked = true; }), co::result_0::success);
	EXPECT_FALSE(invoked);

	EXPECT_TRUE(inner.run_one());
	EXPECT_TRUE(invoked);
	EXPECT_EQ(inner.pending(), 0u);
}

TEST_F(cpp_strand_0_executor_0, invokes_one_handler_per_inner_job_in_posting_order)
{
	manual_executor_0* inner_state = nullptr;
	co::executor_0 inner_executor = create_manual(&inner_state);
	auto inner_executor_guard = co_test::guard(inner_executor);
	manual_executor_0& inner = *inner_state;
	co::strand_0_executor_0 strand = create_strand(inner_executor);
	auto strand_guard = co_test::guard(strand);
	std::vector<int> order;

	for (int i = 0; i < 3; ++i) {
		EXPECT_EQ(strand.post_0([&order, i]() noexcept { order.push_back(i); }), co::result_0::success);
	}
	EXPECT_EQ(inner.pending(), 1u);

	EXPECT_TRUE(inner.run_one());
	EXPECT_EQ(order, (std::vector<int>{ 0 }));
	EXPECT_EQ(inner.pending(), 1u);

	EXPECT_TRUE(inner.run_one());
	EXPECT_EQ(order, (std::vector<int>{ 0, 1 }));
	EXPECT_EQ(inner.pending(), 1u);

	EXPECT_TRUE(inner.run_one());
	EXPECT_EQ(order, (std::vector<int>{ 0, 1, 2 }));
	EXPECT_EQ(inner.pending(), 0u);
}

TEST_F(cpp_strand_0_executor_0, destroys_handler_within_its_invocation_before_invoking_next)
{
	manual_executor_0* inner_state = nullptr;
	co::executor_0 inner_executor = create_manual(&inner_state);
	auto inner_executor_guard = co_test::guard(inner_executor);
	manual_executor_0& inner = *inner_state;
	co::strand_0_executor_0 strand = create_strand(inner_executor);
	auto strand_guard = co_test::guard(strand);
	std::vector<std::string> entries;

	EXPECT_EQ(strand.post_0(tracked_handler(entries, "first")), co::result_0::success);
	EXPECT_EQ(strand.post_0(tracked_handler(entries, "second")), co::result_0::success);
	EXPECT_TRUE(entries.empty());

	EXPECT_TRUE(inner.run_one());
	EXPECT_EQ(entries, (std::vector<std::string>{"invoke first", "destroy first"}));

	EXPECT_TRUE(inner.run_one());
	EXPECT_EQ(entries, (std::vector<std::string>{"invoke first", "destroy first", "invoke second", "destroy second"}));
}

TEST_F(cpp_strand_0_executor_0, invokes_handlers_posted_while_running_after_queued_ones)
{
	manual_executor_0* inner_state = nullptr;
	co::executor_0 inner_executor = create_manual(&inner_state);
	auto inner_executor_guard = co_test::guard(inner_executor);
	manual_executor_0& inner = *inner_state;
	co::strand_0_executor_0 strand = create_strand(inner_executor);
	auto strand_guard = co_test::guard(strand);
	std::vector<std::string> order;

	EXPECT_EQ(strand.post_0([&]() noexcept {
		order.push_back("a");
		EXPECT_EQ(strand.post_0([&]() noexcept { order.push_back("c"); }), co::result_0::success);
	}), co::result_0::success);
	EXPECT_EQ(strand.post_0([&]() noexcept { order.push_back("b"); }), co::result_0::success);

	inner.run_all();
	EXPECT_EQ(order, (std::vector<std::string>{"a", "b", "c"}));
}

TEST_F(cpp_strand_0_executor_0, post_0_returns_inner_error_and_leaves_handler_with_caller)
{
	manual_executor_0* inner_state = nullptr;
	co::executor_0 inner_executor = create_manual(&inner_state);
	auto inner_executor_guard = co_test::guard(inner_executor);
	manual_executor_0& inner = *inner_state;
	co::strand_0_executor_0 strand = create_strand(inner_executor);
	auto strand_guard = co_test::guard(strand);
	callback_log rejected = {.name = "rejected"};
	std::vector<std::string> entries;

	inner.reject = co::result_0::error_system;
	EXPECT_EQ(strand.post_0(co::executor_handler_0{.invoke_fn = &log_invoke, .data = &rejected}), co::result_0::error_system);
	EXPECT_TRUE(rejected.entries.empty());
	// A handler that was not accepted is destroyed by post_0() on the calling thread, never invoked.
	EXPECT_EQ(strand.post_0(tracked_handler(entries, "rejected")), co::result_0::error_system);
	EXPECT_EQ(entries, (std::vector<std::string>{"destroy rejected"}));
	EXPECT_EQ(inner.pending(), 0u);

	inner.reject = co::result_0::success;
	bool invoked = false;
	EXPECT_EQ(strand.post_0([&invoked]() noexcept { invoked = true; }), co::result_0::success);
	EXPECT_TRUE(inner.run_one());
	EXPECT_TRUE(invoked);
}

TEST_F(cpp_strand_0_executor_0, rejected_first_post_does_not_affect_queued_handlers)
{
	manual_executor_0* inner_state = nullptr;
	co::executor_0 inner_executor = create_manual(&inner_state);
	auto inner_executor_guard = co_test::guard(inner_executor);
	manual_executor_0& inner = *inner_state;
	co::strand_0_executor_0 strand = create_strand(inner_executor);
	auto strand_guard = co_test::guard(strand);
	std::vector<std::string> order;

	EXPECT_EQ(strand.post_0([&]() noexcept { order.push_back("a"); }), co::result_0::success);
	// The strand
	inner.reject = co::result_0::error_system;
	EXPECT_EQ(strand.post_0([&]() noexcept { order.push_back("b"); }), co::result_0::success);
	inner.reject = co::result_0::success;

	inner.run_all();
	EXPECT_EQ(order, (std::vector<std::string>{"a", "b"}));
}

TEST_F(cpp_strand_0_executor_0, may_be_destroyed_from_within_its_only_handler)
{
	manual_executor_0* inner_state = nullptr;
	co::executor_0 inner_executor = create_manual(&inner_state);
	auto inner_executor_guard = co_test::guard(inner_executor);
	manual_executor_0& inner = *inner_state;
	co::strand_0_executor_0 strand = create_strand(inner_executor);
	bool invoked = false;

	EXPECT_EQ(strand.post_0([strand, &invoked]() noexcept {
		invoked = true;
		strand.destroy();
	}), co::result_0::success);

	EXPECT_TRUE(inner.run_one());
	EXPECT_TRUE(invoked);
	EXPECT_EQ(inner.pending(), 0u);
}

TEST_F(cpp_strand_0_executor_0, may_be_destroyed_from_within_its_last_handler)
{
	manual_executor_0* inner_state = nullptr;
	co::executor_0 inner_executor = create_manual(&inner_state);
	auto inner_executor_guard = co_test::guard(inner_executor);
	manual_executor_0& inner = *inner_state;
	co::strand_0_executor_0 strand = create_strand(inner_executor);
	std::vector<std::string> order;

	EXPECT_EQ(strand.post_0([&]() noexcept { order.push_back("a"); }), co::result_0::success);
	EXPECT_EQ(strand.post_0([&]() noexcept { order.push_back("b"); }), co::result_0::success);
	EXPECT_EQ(strand.post_0([strand, &order]() noexcept {
		order.push_back("last");
		strand.destroy();
	}), co::result_0::success);

	inner.run_all();
	EXPECT_EQ(order, (std::vector<std::string>{"a", "b", "last"}));
	EXPECT_EQ(inner.pending(), 0u);
}

TEST_F(cpp_strand_0_executor_0, may_be_destroyed_from_within_its_last_handler_on_thread_pool)
{
	std::latch done(1);
	co::executor_0 inner = create_thread_pool(2);
	auto inner_guard = co_test::guard(inner);
	co::strand_0_executor_0 strand = create_strand(inner);
	EXPECT_EQ(strand.post_0([strand, &done]() noexcept {
		strand.destroy();
		done.count_down();
	}), co::result_0::success);
	done.wait();
}

TEST_F(cpp_strand_0_executor_0_death, terminates_when_inner_rejects_next_job)
{
	EXPECT_DEATH({
		manual_executor_0* inner_state = nullptr;
		co::executor_0 inner_executor = create_manual(&inner_state);
		manual_executor_0& inner = *inner_state;
		co::strand_0_executor_0 strand = create_strand(inner_executor);
		strand.post_0([]() noexcept {});
		strand.post_0([]() noexcept {});
		inner.reject = co::result_0::error_system;
		inner.run_one();
	}, "");
}

TEST_F(cpp_strand_0_executor_0, serializes_handlers_on_thread_pool)
{
	constexpr int producer_count = 4;
	constexpr int handlers_per_producer = 2000;

	std::atomic<int> active = 0;
	std::atomic<bool> overlapped = false;
	std::vector<int> next_sequence(producer_count, 0); // accessed only from strand handlers
	bool out_of_order = false;

	co::executor_0 inner = create_thread_pool(4);
	co::strand_0_executor_0 strand = create_strand(inner);
	// Destroying the inner executor runs all of its handlers, which leaves the strand idle, so the strand guard is
	// declared first to be destroyed last.
	auto strand_guard = co_test::guard(strand);
	auto inner_guard = co_test::guard(inner);

	std::vector<std::thread> producers;
	for (int producer = 0; producer < producer_count; ++producer) {
		producers.emplace_back([&, producer]() {
			for (int sequence = 0; sequence < handlers_per_producer; ++sequence) {
				co::result_0 result = strand.post_0([&, producer, sequence]() noexcept {
					if (active.fetch_add(1) != 0) {
						overlapped = true;
					}
					if (next_sequence[producer] != sequence) {
						out_of_order = true;
					}
					next_sequence[producer] = sequence + 1;
					active.fetch_sub(1);
				});
				EXPECT_EQ(result, co::result_0::success);
			}
		});
	}
	for (std::thread& producer : producers) {
		producer.join();
	}
	inner_guard.reset();
	strand_guard.reset();

	EXPECT_FALSE(overlapped.load());
	EXPECT_FALSE(out_of_order);
	EXPECT_EQ(next_sequence, std::vector<int>(producer_count, handlers_per_producer));
}

TEST_F(c_strand_0_executor_0, create_validates_arguments)
{
	co_instance_0 c_instance = static_cast<co_instance_0>(instance);
	co_strand_0_executor_0 strand = nullptr;
	co_strand_0_executor_0_create_info_0 create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_STRAND_0_EXECUTOR_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.inner_executor = nullptr,
	};

	EXPECT_EQ(co_strand_0_executor_0_create_0(c_instance, &create_info, &strand), CO_RESULT_0_ERROR_INVALID_ARGUMENT);

	co::executor_0 inner = create_manual();
	auto inner_guard = co_test::guard(inner);
	create_info.inner_executor = static_cast<co_executor_0>(inner);
	EXPECT_EQ(co_strand_0_executor_0_create_0(c_instance, &create_info, nullptr), CO_RESULT_0_ERROR_INVALID_ARGUMENT);
	EXPECT_EQ(strand, nullptr);
}

TEST_F(c_strand_0_executor_0, posts_through_strand_and_executor_handles)
{
	manual_executor_0* inner_state = nullptr;
	co::executor_0 inner_executor = create_manual(&inner_state);
	manual_executor_0& inner = *inner_state;
	co_strand_0_executor_0_create_info_0 create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_STRAND_0_EXECUTOR_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.inner_executor = static_cast<co_executor_0>(inner_executor),
	};
	co_strand_0_executor_0 strand = nullptr;
	auto inner_executor_guard = co_test::guard(inner_executor);
	ASSERT_EQ(co_strand_0_executor_0_create_0(static_cast<co_instance_0>(instance), &create_info, &strand), CO_RESULT_0_SUCCESS);
	auto strand_guard = co_test::guard(strand);
	ASSERT_NE(strand, nullptr);
	co_executor_0 executor = CO_EXECUTOR_0_CAST(strand);
	EXPECT_EQ(CO_STRAND_0_EXECUTOR_0_CAST(executor), strand);
	EXPECT_EQ(CO_STRAND_0_EXECUTOR_0_CAST(nullptr), nullptr);

	callback_log first = {.name = "first"};
	callback_log second = {.name = "second"};
	EXPECT_EQ(co_strand_0_executor_0_post_0(strand, co_executor_handler_0{.invoke_fn = &log_invoke, .data = &first}), CO_RESULT_0_SUCCESS);
	EXPECT_EQ(co_executor_0_post_0(executor, co_executor_handler_0{.invoke_fn = &log_invoke, .data = &second}), CO_RESULT_0_SUCCESS);
	EXPECT_TRUE(first.entries.empty());

	EXPECT_TRUE(inner.run_one());
	EXPECT_EQ(first.entries, (std::vector<std::string>{ "invoke first" }));
	EXPECT_TRUE(second.entries.empty());

	EXPECT_TRUE(inner.run_one());
	EXPECT_EQ(second.entries, (std::vector<std::string>{ "invoke second" }));
	EXPECT_EQ(inner.pending(), 0u);
}

TEST_F(c_strand_0_executor_0, ignores_hints)
{
	co::executor_0 inner_executor = create_manual();
	auto inner_executor_guard = co_test::guard(inner_executor);
	const int unknown_tag = 0;
	const co_in_structure_0 unknown = {.structure_type = static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&unknown_tag)), .next_structure = nullptr};
	const co_hint_info_0 hint = {.structure_type = CO_STRUCTURE_TYPE_0_HINT_INFO_0, .next_structure = nullptr, .hint = &unknown};
	co_strand_0_executor_0_create_info_0 create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_STRAND_0_EXECUTOR_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&hint),
		.inner_executor = static_cast<co_executor_0>(inner_executor),
	};
	co_strand_0_executor_0 strand = nullptr;
	ASSERT_EQ(co_strand_0_executor_0_create_0(static_cast<co_instance_0>(instance), &create_info, &strand), CO_RESULT_0_SUCCESS);
	auto strand_guard = co_test::guard(strand);

	create_info.next_structure = &unknown;
	co_strand_0_executor_0 rejected = nullptr;
	EXPECT_EQ(co_strand_0_executor_0_create_0(static_cast<co_instance_0>(instance), &create_info, &rejected), CO_RESULT_0_ERROR_NOT_SUPPORTED);
	EXPECT_EQ(rejected, nullptr);
}

namespace
{

struct destroying_handler_data final
{
	co_strand_0_executor_0 strand;
	bool invoked;
};

void destroy_strand_invoke(void* data) noexcept
{
	destroying_handler_data* handler_data = static_cast<destroying_handler_data*>(data);
	handler_data->invoked = true;
	co_strand_0_executor_0_destroy(handler_data->strand);
}

} // namespace

TEST_F(c_strand_0_executor_0, may_be_destroyed_from_within_its_last_handler)
{
	manual_executor_0* inner_state = nullptr;
	co::executor_0 inner_executor = create_manual(&inner_state);
	auto inner_executor_guard = co_test::guard(inner_executor);
	manual_executor_0& inner = *inner_state;
	co_strand_0_executor_0_create_info_0 create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_STRAND_0_EXECUTOR_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.inner_executor = static_cast<co_executor_0>(inner_executor),
	};
	co_strand_0_executor_0 strand = nullptr;
	ASSERT_EQ(co_strand_0_executor_0_create_0(static_cast<co_instance_0>(instance), &create_info, &strand), CO_RESULT_0_SUCCESS);

	destroying_handler_data data{.strand = strand, .invoked = false};
	EXPECT_EQ(co_strand_0_executor_0_post_0(strand, co_executor_handler_0{.invoke_fn = &destroy_strand_invoke, .data = &data}), CO_RESULT_0_SUCCESS);

	EXPECT_TRUE(inner.run_one());
	EXPECT_TRUE(data.invoked);
	EXPECT_EQ(inner.pending(), 0u);
}
