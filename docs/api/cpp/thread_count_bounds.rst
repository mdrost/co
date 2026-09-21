co::thread_count_bounds_info_0
==============================

Bounds of the number of threads of a thread pool whose number of threads varies.

.. code-block:: cpp

   #include <co/thread_count_bounds_0.hpp>

Put it in the next chain of a ``co::event_facility_0_create_info_0`` that also holds a
``co::dynamic_thread_pool_0_create_info_0``, in any order, at most once. Without it, the class chooses the bounds.

The bounds are a requirement, not a hint. Handlers that block until other handlers run deadlock when the pool runs
fewer threads than they need, so a class never runs fewer than ``minimum_thread_count`` nor more than
``maximum_thread_count`` threads. A class that cannot honor the bounds declines with
``co::result_0::error_not_supported`` and the instance tries the next candidate class. A class whose pool has a fixed
number of threads may accept equal bounds.

.. doxygengroup:: cpp_thread_count_bounds
   :content-only:
   :members:
