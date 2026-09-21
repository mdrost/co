co::strand_0_executor_0
=======================

Executors that invoke posted handlers one at a time, in posting order, on top of another executor.

.. code-block:: cpp

   #include <co/strand_0.hpp>

Example
-------

.. code-block:: cpp

   co::strand_0_executor_0 strand;
   co::strand_0_executor_0_create_0(instance,
	   .structure_type = co::structure_type_0_strand_0_executor_0_create_info_0,
	   .next_structure = nullptr,
	   .inner_executor = thread_pool_executor,
   }, &strand);
   if (result == co::result_0::success) {
	   int counter = 0;
	   for (int i = 0; i < 100; ++i) {
		   // No locking needed: handlers of one strand never run concurrently.
		   strand.post_0([&counter]() noexcept { ++counter; });
	   }
	   // ... stop posting and let the inner executor invoke all of its handlers ...
	   strand.destroy(); // precondition: the strand is idle
   }

A strand may also be destroyed from within its last handler, for example by an object that owns it:

.. code-block:: cpp

   strand.post_0([strand]() noexcept {
	   // ... last work of the owner ...
	   strand.destroy(); // precondition: no other handler is queued
   });

API reference
-------------

.. doxygengroup:: cpp_strand_0_executor_0
   :content-only:
   :members:
