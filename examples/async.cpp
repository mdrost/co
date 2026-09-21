#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOIME // ?
#define NORESOURCE // ?
#define WIN32_NO_STATUS
#include <Windows.h>
//#undef WIN32_NO_STATUS
//#include <winternl.h>
//#include <ntstatus.h>

#include <co/executor_priority_0.h>
#include <co/platform_thread_pool_0.h>
#include <co/task_0.hpp>
#include <co/timer_0_service_0.hpp>

#include <algorithm>
#include <coroutine>
#include <cstdlib>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// A stand-in asynchronous operation.
//
// Every coroutine mechanism below expresses the SAME logical routine:
//
//     async_delay(10ms);
//     first = 1;
//     async_delay(20ms);
//     return first + 2;
//
// The operation is completion-callback based, which is the only shape the C ABI
// can express: it starts work and returns immediately, and the continuation is
// invoked later from a handler posted to the executor. Everything else in this
// file is just a different way of writing "what to do next".
// ---------------------------------------------------------------------------

typedef void (* async_delay_callback_fn)(void* context) CO_NOEXCEPT;

typedef struct async_delay_operation
{
	async_delay_callback_fn callback;
	void* context;
} async_delay_operation;

static void async_delay_invoke(void* data) CO_NOEXCEPT
{
	async_delay_operation operation = *(async_delay_operation*)data;
	free(data);
	operation.callback(operation.context);
}

// Starts the operation and returns immediately. The callback runs on `executor`.
static void async_delay(co_executor_0 executor, unsigned milliseconds, async_delay_callback_fn callback, void* context)
{
	(void)milliseconds; // a real implementation would arm a timer service here

	async_delay_operation* operation = (async_delay_operation*)malloc(sizeof(async_delay_operation));
	operation->callback = callback;
	operation->context = context;

	co_executor_handler_0 handler = { &async_delay_invoke, operation };
	if (co_executor_0_post_0(executor, handler) != CO_RESULT_0_SUCCESS) {
		free(operation); // not accepted: the operation is still ours
	}
}

// ---------------------------------------------------------------------------
// 1. C++20 coroutines -- co::task_0
//
// The compiler generates the context struct (the coroutine frame) and the
// resume point. Locals survive suspension untouched, control flow is ordinary.
// Only the awaiter has to be written by hand, and only once per operation.
// ---------------------------------------------------------------------------

class async_delay_awaiter
{
public:

	async_delay_awaiter(co_executor_0 executor, unsigned milliseconds) noexcept
		: m_executor(executor)
		, m_milliseconds(milliseconds)
		, m_continuation()
	{
	}

	[[nodiscard]] constexpr bool await_ready() const noexcept
	{
		return false;
	}

	void await_suspend(std::coroutine_handle<> continuation) noexcept
	{
		m_continuation = continuation;
		async_delay(m_executor, m_milliseconds, &resume, this);
	}

	constexpr void await_resume() const noexcept
	{
	}

private:

	static void resume(void* context) noexcept
	{
		static_cast<async_delay_awaiter*>(context)->m_continuation.resume();
	}

private:
	co_executor_0 m_executor;
	unsigned m_milliseconds;
	std::coroutine_handle<> m_continuation;
};

static co::task_0<int> cpp20_two_step(co_executor_0 executor)
{
	co_await async_delay_awaiter(executor, 10);
	int first = 1;                                  // a plain local, survives the suspension
	co_await async_delay_awaiter(executor, 20);
	co_return first + 2;
}

// ---------------------------------------------------------------------------
// 2. C -- continuation-passing style (no mechanism required)
//
// This is what the C ABI already supports today. The "coroutine frame" is a
// hand-written struct, and each suspension point becomes its own function.
// Zero runtime cost, but the control flow is turned inside out.
// ---------------------------------------------------------------------------

typedef void (* cps_two_step_callback_fn)(int result, void* context) CO_NOEXCEPT;

typedef struct cps_two_step_state
{
	co_executor_0 executor;
	cps_two_step_callback_fn callback;
	void* context;

	int first; // every local that outlives a suspension must be hoisted here
} cps_two_step_state;

static void cps_two_step_after_second(void* data) CO_NOEXCEPT
{
	cps_two_step_state* state = (cps_two_step_state*)data;

	int result = state->first + 2;

	cps_two_step_callback_fn callback = state->callback;
	void* context = state->context;
	free(state);

	callback(result, context);
}

static void cps_two_step_after_first(void* data) CO_NOEXCEPT
{
	cps_two_step_state* state = (cps_two_step_state*)data;

	state->first = 1;

	async_delay(state->executor, 20, &cps_two_step_after_second, state);
}

static void cps_two_step(co_executor_0 executor, cps_two_step_callback_fn callback, void* context)
{
	cps_two_step_state* state = (cps_two_step_state*)malloc(sizeof(cps_two_step_state));
	state->executor = executor;
	state->callback = callback;
	state->context = context;
	state->first = 0;

	async_delay(executor, 10, &cps_two_step_after_first, state);
}

// ---------------------------------------------------------------------------
// 3. C -- stackless state machine (Duff's device / protothread macros)
//
// One function again, at the cost of a macro vocabulary the user must adopt.
// The resume point is a saved label id; the stack is NOT preserved, so locals
// still have to live in the state struct. Suspension is only possible in the
// body of this function -- never inside a helper it calls.
//
// Note: real protothread macros use __LINE__ to generate the label implicitly,
// but that is not a constant expression under MSVC /ZI, so the ids are explicit.
// ---------------------------------------------------------------------------

#define CO_SM_BEGIN(state)      switch (state) { case 0:
#define CO_SM_YIELD(state, id)  do { (state) = (id); return; case (id):; } while (0)
#define CO_SM_END(state)        } (state) = -1;

typedef struct sm_two_step_state
{
	co_executor_0 executor;
	int resume_point;

	int first; // still hoisted: locals do not survive the return
} sm_two_step_state;

static void sm_two_step_step(void* data) CO_NOEXCEPT
{
	sm_two_step_state* state = (sm_two_step_state*)data;

	CO_SM_BEGIN(state->resume_point)

	async_delay(state->executor, 10, &sm_two_step_step, state);
	CO_SM_YIELD(state->resume_point, 1);

	state->first = 1;

	async_delay(state->executor, 20, &sm_two_step_step, state);
	CO_SM_YIELD(state->resume_point, 2);

	state->first = state->first + 2;

	CO_SM_END(state->resume_point)
}

static void sm_two_step(co_executor_0 executor)
{
	sm_two_step_state* state = (sm_two_step_state*)malloc(sizeof(sm_two_step_state));
	state->executor = executor;
	state->resume_point = 0;
	state->first = 0;

	sm_two_step_step(state); // drives the machine to its first suspension
}

// ---------------------------------------------------------------------------
// 4. C -- stackful coroutine (Win32 fibers)
//
// Straight-line code and ordinary locals, exactly like the C++20 version,
// because the "frame" is a real machine stack. The price: one stack per task,
// plus a thread-affinity problem that does not exist in any other mechanism.
//
// SwitchToFiber and GetCurrentFiber are only legal on a thread that has been
// converted to a fiber. A handler posted to an executor may run on ANY thread
// the executor owns -- a thread-pool thread has certainly not been converted.
// So a stackful provider cannot assume a fiber-capable thread: it must convert
// on entry to every posted handler and convert back before returning, because
// it does not own those threads and must leave them exactly as it found them.
//
// Note the cost this implies: conversion allocates a fiber object per thread,
// and it happens on every resume, not once per thread. A production provider
// would cache the conversion in TLS and only convert threads it has not seen.
// ---------------------------------------------------------------------------

typedef struct fiber_two_step_state
{
	co_executor_0 executor;
	LPVOID fiber;
	LPVOID resumer;
	int result;
	int finished;
} fiber_two_step_state;

static void fiber_two_step_yield(fiber_two_step_state* state)
{
	SwitchToFiber(state->resumer);
}

static void fiber_two_step_resume(void* context) CO_NOEXCEPT
{
	fiber_two_step_state* state = (fiber_two_step_state*)context;

	// IsThreadAFiber() is the only way to ask; the executor gives no guarantee
	// about which thread this handler runs on.
	BOOL converted = FALSE;
	if (!IsThreadAFiber()) {
		if (ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH) == NULL) {
			return; // a real provider would report this failure
		}
		converted = TRUE;
	}

	state->resumer = GetCurrentFiber();
	SwitchToFiber(state->fiber);

	// Control returns here once the coroutine yields or finishes. Restore the
	// thread to its original state so the executor's thread is left untouched.
	if (converted) {
		ConvertFiberToThread();
	}
}

static void CALLBACK fiber_two_step_entry(LPVOID parameter)
{
	fiber_two_step_state* state = (fiber_two_step_state*)parameter;

	async_delay(state->executor, 10, &fiber_two_step_resume, state);
	fiber_two_step_yield(state);

	int first = 1;                                  // a plain local on the fiber stack

	async_delay(state->executor, 20, &fiber_two_step_resume, state);
	fiber_two_step_yield(state);

	state->result = first + 2;
	state->finished = 1;

	SwitchToFiber(state->resumer); // a fiber entry point must never return
}

static void fiber_two_step(co_executor_0 executor)
{
	fiber_two_step_state* state = (fiber_two_step_state*)malloc(sizeof(fiber_two_step_state));
	state->executor = executor;
	state->resumer = NULL;
	state->result = 0;
	state->finished = 0;
	state->fiber = CreateFiberEx(4096, 64 * 1024, 0, &fiber_two_step_entry, state);

	fiber_two_step_resume(state); // runs until the first yield
}

// Referenced only to keep every example alive for the compiler.
void co_coroutine_examples(co_executor_0 executor)
{
	co::task_0<int> cpp20 = cpp20_two_step(executor);
	(void)cpp20;

	cps_two_step(executor, nullptr, nullptr);
	sm_two_step(executor);
	fiber_two_step(executor);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR lpCmdLine,
                     _In_ int nCmdShow)
{

	co_instance_0_create_info_0 instance_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_INSTANCE_0_CREATE_INFO_0,
		.next_structure = NULL,
		.api_version = CO_API_VERSION_0,
	};
	co_instance_0 instance;
	if (co_instance_0_create_0(&instance_create_info, &instance) != CO_RESULT_0_SUCCESS) {
		return EXIT_FAILURE;
	}
	// ----- Event domains -----------------------------------------------------------------
	// Every group has its own required services and its own event domain create info, so each
	// group can be changed independently. Members of one group share its create info.
	// No concrete event loop create info is chained, so the instance picks any class that
	// provides the required services (then rank, then registration order).
	co_service_type_0 lone_execution_event_loop_required_services[] = {
		CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0,
	};
	co_event_loop_0_create_info_0 lone_execution_event_loop_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.required_services = lone_execution_event_loop_required_services,
		.required_service_count = std::size(lone_execution_event_loop_required_services),
	};
	co_service_type_0 execution_event_loop_group_required_services[] = {
		CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0,
	};
	co_event_loop_0_create_info_0 execution_event_loop_group_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.required_services = execution_event_loop_group_required_services,
		.required_service_count = std::size(execution_event_loop_group_required_services),
	};
	co_service_type_0 lone_source_only_event_loop_required_services[] = {
		CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0,
	};
	co_event_loop_0_create_info_0 lone_source_only_event_loop_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.required_services = lone_source_only_event_loop_required_services,
		.required_service_count = std::size(lone_source_only_event_loop_required_services),
	};
	co_service_type_0 source_only_event_loop_group_required_services[] = {
		CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0,
	};
	co_event_loop_0_create_info_0 source_only_event_loop_group_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.required_services = source_only_event_loop_group_required_services,
		.required_service_count = std::size(source_only_event_loop_group_required_services),
	};
	// The platform thread pool create info in the next chain of a generic event facility create
	// info asks for an event facility of the thread pool of the platform. Each facility has its
	// own create info so that each can be changed independently.
	co_platform_thread_pool_0_create_info_0 execution_platform_thread_pool = {
		.structure_type = CO_STRUCTURE_TYPE_0_PLATFORM_THREAD_POOL_0_CREATE_INFO_0,
		.next_structure = nullptr,
	};
	co_service_type_0 execution_platform_thread_pool_event_facility_required_services[] = {
		CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0,
	};
	co_event_facility_0_create_info_0 execution_platform_thread_pool_event_facility_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&execution_platform_thread_pool),
		.required_services = execution_platform_thread_pool_event_facility_required_services,
		.required_service_count = std::size(execution_platform_thread_pool_event_facility_required_services),
	};
	co_platform_thread_pool_0_create_info_0 source_only_platform_thread_pool = {
		.structure_type = CO_STRUCTURE_TYPE_0_PLATFORM_THREAD_POOL_0_CREATE_INFO_0,
		.next_structure = nullptr,
	};
	co_service_type_0 source_only_platform_thread_pool_event_facility_required_services[] = {
		CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0,
	};
	co_event_facility_0_create_info_0 source_only_platform_thread_pool_event_facility_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&source_only_platform_thread_pool),
		.required_services = source_only_platform_thread_pool_event_facility_required_services,
		.required_service_count = std::size(source_only_platform_thread_pool_event_facility_required_services),
	};

	// ----- Event domain groups -----------------------------------------------------------
	// A group is N identical event domains created from one event loop or event facility
	// create info. A lone event domain is a group of one. Groups are not required to be
	// targeted by an executor: source-only groups exist just to provide services.
	// event_domain_create_info is the required member (head of the member's own chain);
	// next_structure is reserved for group-wide extensions independent of the member kind.
	// A group create info only says what to create; where it lives in the runtime is given
	// by the runtime entries below.
	co_event_domain_group_0_create_info_0 lone_execution_event_loop_group_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.event_domain_create_info = CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&lone_execution_event_loop_create_info),
		.event_domain_count = 1,
	};
	co_event_domain_group_0_create_info_0 execution_event_loop_group_group_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.event_domain_create_info = CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&execution_event_loop_group_create_info),
		.event_domain_count = 4,
	};
	co_event_domain_group_0_create_info_0 lone_source_only_event_loop_group_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.event_domain_create_info = CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&lone_source_only_event_loop_create_info),
		.event_domain_count = 1,
	};
	co_event_domain_group_0_create_info_0 source_only_event_loop_group_group_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.event_domain_create_info = CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&source_only_event_loop_group_create_info),
		.event_domain_count = 2,
	};
	co_event_domain_group_0_create_info_0 execution_platform_thread_pool_event_facility_group_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.event_domain_create_info = CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&execution_platform_thread_pool_event_facility_create_info),
		.event_domain_count = 1,
	};
	co_event_domain_group_0_create_info_0 source_only_platform_thread_pool_event_facility_group_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.event_domain_create_info = CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&source_only_platform_thread_pool_event_facility_create_info),
		.event_domain_count = 1,
	};

	// ----- Executors ---------------------------------------------------------------------
	// An executor create info only says what to create (concrete kinds and hints go in its
	// next chain); which group it targets is given by the runtime entries below.
	co_executor_0_create_info_0 lone_execution_event_loop_executor_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EXECUTOR_0_CREATE_INFO_0,
		.next_structure = nullptr,
	};
	co_executor_0_create_info_0 execution_event_loop_group_executor_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EXECUTOR_0_CREATE_INFO_0,
		.next_structure = nullptr,
	};
	// A priority wrapped in a hint is a preference: every class may be chosen, and one that
	// cannot honor it ignores it. Put directly in the chain, it would be a requirement that
	// narrows the choice to classes handling it.
	co_executor_priority_info_0 execution_platform_thread_pool_executor_priority = {
		.structure_type = CO_STRUCTURE_TYPE_0_EXECUTOR_PRIORITY_INFO_0,
		.next_structure = nullptr,
		.priority = CO_EXECUTOR_PRIORITY_0_HIGH,
	};
	co_hint_info_0 execution_platform_thread_pool_executor_priority_hint = {
		.structure_type = CO_STRUCTURE_TYPE_0_HINT_INFO_0,
		.next_structure = nullptr,
		.hint = CO_IN_STRUCTURE_0_CAST(&execution_platform_thread_pool_executor_priority),
	};
	co_executor_0_create_info_0 execution_platform_thread_pool_executor_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EXECUTOR_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&execution_platform_thread_pool_executor_priority_hint),
	};

	// ----- Runtime -----------------------------------------------------------------------
	// Runtime entries pair a create info with its runtime bookkeeping. Every group and
	// executor has an explicit id chosen by the user (@pre unique within the runtime create
	// info). Executors and runtime accessors refer to groups by id, so removing a group (e.g.
	// when retrying without thread pools) does not renumber the others. Array order only
	// defines creation order; the runtime destroys groups in reverse.
	enum : size_t {
		lone_execution_event_loop_group_id,
		execution_event_loop_group_id,
		lone_source_only_event_loop_group_id,
		source_only_event_loop_group_id,
		execution_platform_thread_pool_event_facility_group_id,
		source_only_platform_thread_pool_event_facility_group_id,
	};
	co_runtime_0_event_domain_group_info_0 event_domain_groups[] = {
		{
			.id = lone_execution_event_loop_group_id,
			.create_info = &lone_execution_event_loop_group_create_info,
		},
		{
			.id = execution_event_loop_group_id,
			.create_info = &execution_event_loop_group_group_create_info,
		},
		{
			.id = lone_source_only_event_loop_group_id,
			.create_info = &lone_source_only_event_loop_group_create_info,
		},
		{
			.id = source_only_event_loop_group_id,
			.create_info = &source_only_event_loop_group_group_create_info,
		},
		{
			.id = execution_platform_thread_pool_event_facility_group_id,
			.create_info = &execution_platform_thread_pool_event_facility_group_create_info,
		},
		{
			.id = source_only_platform_thread_pool_event_facility_group_id,
			.create_info = &source_only_platform_thread_pool_event_facility_group_create_info,
		},
	};
	// Each executor targets exactly one group by id; distributing handlers across the members
	// of a group is the executor's policy. Executor ids are independent of the group ids.
	enum : size_t {
		lone_execution_event_loop_executor_id,
		execution_event_loop_group_executor_id,
		execution_platform_thread_pool_executor_id,
	};
	co_runtime_0_executor_info_0 executors[] = {
		{
			.id = lone_execution_event_loop_executor_id,
			.event_domain_group_id = lone_execution_event_loop_group_id,
			.create_info = &lone_execution_event_loop_executor_create_info,
		},
		{
			.id = execution_event_loop_group_executor_id,
			.event_domain_group_id = execution_event_loop_group_id,
			.create_info = &execution_event_loop_group_executor_create_info,
		},
		{
			.id = execution_platform_thread_pool_executor_id,
			.event_domain_group_id = execution_platform_thread_pool_event_facility_group_id,
			.create_info = &execution_platform_thread_pool_executor_create_info,
		},
	};
	// The runtime only owns: it creates every group, event domain, service and executor, and
	// destroys all of them in co_runtime_0_destroy. It never creates threads.
	co_runtime_0_create_info_0 runtime_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_RUNTIME_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.event_domain_groups = event_domain_groups,
		.event_domain_group_count = std::size(event_domain_groups),
		.executors = executors,
		.executor_count = std::size(executors),
	};
	co_runtime_0 runtime;
	if (co_instance_0_create_runtime_0(instance, &runtime_create_info, &runtime) != CO_RESULT_0_SUCCESS) {
		co_instance_0_destroy(instance);
		return EXIT_FAILURE;
	}

	// ----- Running event loops -----------------------------------------------------------
	// The user runs every event loop, on threads of their choosing. Handles obtained from the
	// runtime are borrowed and stay valid until co_runtime_0_destroy. The runtime hands out
	// event domains; the user knows which groups are event loops and casts their handles.
	const size_t event_loop_group_ids[] = {
		lone_execution_event_loop_group_id,
		execution_event_loop_group_id,
		lone_source_only_event_loop_group_id,
		source_only_event_loop_group_id,
	};
	std::vector<co_event_loop_0> event_loops;
	for (const co_runtime_0_event_domain_group_info_0& group : event_domain_groups) {
		if (!std::ranges::contains(event_loop_group_ids, group.id)) {
			continue;
		}
		for (size_t event_domain_index = 0; event_domain_index < group.create_info->event_domain_count; ++event_domain_index) {
			co_event_domain_0 event_domain;
			co_runtime_0_get_event_domain_0(runtime, group.id, event_domain_index, &event_domain);
			event_loops.push_back(CO_EVENT_LOOP_0_CAST(event_domain));
		}
	}
	std::vector<std::thread> event_loop_threads;
	for (co_event_loop_0 event_loop : event_loops) {
		event_loop_threads.emplace_back([event_loop] {
			co_event_loop_0_run_0(event_loop);
		});
	}

	// Event facilities are not run by the user: the operating system calls into them. Services
	// are found on the event domain itself, so no downcast is needed.
	co_event_domain_0 source_only_platform_thread_pool_event_domain = nullptr;
	co_runtime_0_get_event_domain_0(runtime, source_only_platform_thread_pool_event_facility_group_id, 0, &source_only_platform_thread_pool_event_domain);
	(void)source_only_platform_thread_pool_event_domain; // e.g. find its timer service and arm timers

	// ----- Using executors ---------------------------------------------------------------
	co_executor_0 lone_execution_event_loop_executor = nullptr;
	co_runtime_0_get_executor_0(runtime, lone_execution_event_loop_executor_id, &lone_execution_event_loop_executor);
	co_executor_0 execution_event_loop_group_executor = nullptr;
	co_runtime_0_get_executor_0(runtime, execution_event_loop_group_executor_id, &execution_event_loop_group_executor);
	co_executor_0 execution_platform_thread_pool_executor = nullptr;
	co_runtime_0_get_executor_0(runtime, execution_platform_thread_pool_executor_id, &execution_platform_thread_pool_executor);
	co_coroutine_examples(lone_execution_event_loop_executor);
	(void)execution_event_loop_group_executor;
	(void)execution_platform_thread_pool_executor;
	// ...

	// ----- Shutdown ----------------------------------------------------------------------
	// @pre of co_runtime_0_destroy: no event loop is running and every handler accepted by
	// the runtime's executors and event facilities has been invoked.
	for (co_event_loop_0 event_loop : event_loops) {
		co_event_loop_0_stop_0(event_loop);
	}
	for (std::thread& event_loop_thread : event_loop_threads) {
		event_loop_thread.join();
	}
	co_runtime_0_destroy(runtime);
	co_instance_0_destroy(instance);
	return EXIT_SUCCESS;
}