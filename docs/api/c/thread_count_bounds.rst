co_thread_count_bounds_info_0
=============================

Bounds of the number of threads of a thread pool whose number of threads varies.

.. code-block:: c

   #include <co/thread_count_bounds.h>

Put it in the next chain of a ``co_event_facility_0_create_info_0`` that also holds a
``co_dynamic_thread_pool_0_create_info_0``, in any order, at most once.
Without it, the class chooses the bounds. See :doc:`dynamic_thread_pool_0` for an example.

The bounds are a requirement, not a hint. Handlers that block until other handlers run deadlock when the pool runs
fewer threads than they need, so a class never runs fewer than ``minimum_thread_count`` nor more than
``maximum_thread_count`` threads. A class that cannot honor the bounds declines with
``CO_RESULT_0_ERROR_NOT_SUPPORTED`` and the instance tries the next candidate class. A class whose pool has a fixed
number of threads may accept equal bounds. If your handlers need some number of threads to make progress, always give
the bounds rather than relying on the class's choice.

.. doxygengroup:: c_thread_count_bounds
   :content-only:
   :members:
