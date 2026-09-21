co_timer_0
==========

One-shot timers whose completions are posted to an executor.

.. code-block:: c

   #include <co/timer_0.h>

Example
-------

A timeout guarding an operation. Every call on the timer is made from handlers of one strand, so the start, its
completion and the cancellation never race. The completion callback is invoked exactly once, whether the timer elapsed
or was cancelled, and the timer is idle from then on, so it is the place to destroy the timer:

.. code-block:: c

   typedef struct request {
	   co_executor_0 strand;
	   co_timer_0 timeout;
   } request;

   static void on_timeout(void* data, co_result_0 status)
   {
	   request* r = (request*)data;
	   if (status == CO_RESULT_0_SUCCESS) {
		   /* The timer elapsed: the operation took too long, abort it. */
	   } else {
		   /* status == CO_RESULT_0_CANCELLED: the operation finished in time and cancelled the timer. */
	   }
	   co_timer_0_destroy(r->timeout); /* precondition: idle */
	   free(r);
   }

   co_service_0 service;
   co_result_0 result = co_event_facility_0_get_service_0(event_facility, CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0, &service);
   if (result == CO_RESULT_0_SUCCESS) {
	   result = co_timer_0_service_0_create_timer_0(CO_TIMER_0_SERVICE_0_CAST(service), &r->timeout);
   }
   if (result == CO_RESULT_0_SUCCESS) {
	   result = co_timer_0_start_0(r->timeout, 5000000000 /* 5 s */, r->strand, &on_timeout, r);
   }

   /* ... later, in a handler of r->strand, when the operation has finished: */
   co_timer_0_cancel_0(r->timeout);

A timer may be started again from within the completion callback of its previous start, for example to tick
periodically. To stop it, a flag tells the callback not to start it again:

.. code-block:: c

   static void on_tick(void* data, co_result_0 status)
   {
	   ticker* t = (ticker*)data;
	   if (status == CO_RESULT_0_SUCCESS && !t->stopping) {
		   /* ... periodic work ... */
		   co_timer_0_start_0(t->timer, 100000000 /* 100 ms */, t->strand, &on_tick, t);
		   return;
	   }
	   co_timer_0_destroy(t->timer); /* precondition: idle */
	   free(t);
   }

   /* ... later, in a handler of t->strand: */
   t->stopping = true;
   co_timer_0_cancel_0(t->timer);

API reference
-------------

.. doxygengroup:: c_timer_0
   :content-only:
   :members:

For implementers
----------------

.. doxygengroup:: c_timer_0_impl
   :content-only:
   :members:
