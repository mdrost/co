co_strand_0_executor_0
======================

Executors that invoke posted handlers one at a time, in posting order, on top of another executor.

.. code-block:: c

   #include <co/strand_0.h>

Example
-------

.. code-block:: c

   static void increment(void* data)
   {
	   ++*(int*)data; /* no locking needed: handlers of one strand never run concurrently */
   }

   co_strand_0_executor_0_create_info_0 create_info = {
	   .structure_type = CO_STRUCTURE_TYPE_0_STRAND_0_EXECUTOR_0_CREATE_INFO_0,
	   .next_structure = NULL,
	   .inner_executor = thread_pool_executor,
   };
   co_strand_0_executor_0 strand;
   if (co_strand_0_executor_0_create_0(instance, &create_info, &strand) == CO_RESULT_0_SUCCESS) {
	   co_executor_handler_0 handler = {.invoke_fn = &increment, .data = &counter};
	   co_strand_0_executor_0_post_0(strand, handler);
	   /* ... stop posting and let the inner executor invoke all of its handlers ... */
	   co_strand_0_executor_0_destroy(strand); /* precondition: the strand is idle */
   }

A strand may also be destroyed from within its last handler, for example by an object that owns it:

.. code-block:: c

   static void destroy_strand(void* data)
   {
	   /* ... last work of the owner ... */
	   co_strand_0_executor_0_destroy(CO_STRAND_0_EXECUTOR_0_CAST(data)); /* precondition: no other handler is queued */
   }

   co_executor_handler_0 handler = {.invoke_fn = &destroy_strand, .data = strand};
   co_strand_0_executor_0_post_0(strand, handler);

API reference
-------------

.. doxygengroup:: c_strand_0_executor_0
   :content-only:
   :members:
