co::timer_0
===========

Timers with a settable expiry whose waits complete on an executor.

.. code-block:: cpp

   #include <co/timer_0.hpp>

Example
-------

A timeout guarding an operation. Every call on the timer is made from handlers of one strand, so the wait, its
completion and the cancellation never race. The completion callback is invoked exactly once, whether the timer expired
or the wait was cancelled, and no wait is pending from then on, so it is the place to destroy the timer:

.. code-block:: cpp

   struct request
   {
	   co::strand_0_executor_0 strand;
	   co::timer_0 timeout;
   };

   co::service_0 service;
   co::result_0 result = event_facility.get_service_0(co::service_type_0_timer_0_service_0, &service);
   if (result == co::result_0::success) {
	   co::timer_0_create_info_0 timer_create_info = {
		   .structure_type = co::structure_type_0_timer_0_create_info_0,
		   .clock = clock, // required; must outlive the timer
	   };
	   result = co::timer_0_service_0_cast(service).create_timer_0(timer_create_info, &r->timeout);
   }
   if (result == co::result_0::success) {
	   result = r->timeout.expiry_after_0(5'000'000'000 /* 5 s */);
   }
   if (result == co::result_0::success) {
	   result = r->timeout.wait_0(nullptr, r->strand, [](void* data, co::result_0 status) noexcept {
		   request* r = static_cast<request*>(data);
		   if (status == co::result_0::success) {
			   // The timer expired: the operation took too long, abort it.
		   } else {
			   // status == co::result_0::cancelled: the operation finished in time and cancelled the wait.
		   }
		   r->timeout.destroy(); // precondition: no wait pending
		   delete r;
	   }, r);
   }

   // ... later, in a handler of r->strand, when the operation has finished:
   r->timeout.cancel_0();

In a coroutine, the wait can be awaited. A task_0 resumes on its own executor:

.. code-block:: cpp

   co::task_0<> tick(co::timer_0 timer)
   {
	   for (;;) {
		   timer.expiry_after_0(100'000'000 /* 100 ms */);
		   if (co_await timer.wait_0() != co::result_0::success) {
			   co_return;
		   }
		   // ... periodic work ...
	   }
   }

API reference
-------------

.. doxygengroup:: cpp_timer_0
   :content-only:
   :members:
