co::timer_0
===========

One-shot timers whose completions are posted to an executor.

.. code-block:: cpp

   #include <co/timer_0.hpp>

Example
-------

A timeout guarding an operation. Every call on the timer is made from handlers of one strand, so the start, its
completion and the cancellation never race. The completion callback is invoked exactly once, whether the timer elapsed
or was cancelled, and the timer is idle from then on, so it is the place to destroy the timer:

.. code-block:: cpp

   struct request
   {
	   co::strand_0_executor_0 strand;
	   co::timer_0 timeout;
   };

   co::service_0 service;
   co::result_0 result = event_facility.get_service_0(co::service_type_0_timer_0_service_0, &service);
   if (result == co::result_0::success) {
	   result = co::timer_0_service_0_cast(service).create_timer_0(&r->timeout);
   }
   if (result == co::result_0::success) {
	   result = r->timeout.start_0(5'000'000'000 /* 5 s */, r->strand, [](void* data, co::result_0 status) noexcept {
		   request* r = static_cast<request*>(data);
		   if (status == co::result_0::success) {
			   // The timer elapsed: the operation took too long, abort it.
		   } else {
			   // status == co::result_0::cancelled: the operation finished in time and cancelled the timer.
		   }
		   r->timeout.destroy(); // precondition: idle
		   delete r;
	   }, r);
   }

   // ... later, in a handler of r->strand, when the operation has finished:
   r->timeout.cancel_0();

A timer may be started again from within the completion callback of its previous start, for example to tick
periodically. To stop it, a flag tells the callback not to start it again:

.. code-block:: cpp

   static void on_tick(void* data, co::result_0 status) noexcept
   {
	   ticker* t = static_cast<ticker*>(data);
	   if (status == co::result_0::success && !t->stopping) {
		   // ... periodic work ...
		   t->timer.start_0(100'000'000 /* 100 ms */, t->strand, &on_tick, t);
		   return;
	   }
	   t->timer.destroy(); // precondition: idle
	   delete t;
   }

   // ... later, in a handler of t->strand:
   t->stopping = true;
   t->timer.cancel_0();

API reference
-------------

.. doxygengroup:: cpp_timer_0
   :content-only:
   :members:
