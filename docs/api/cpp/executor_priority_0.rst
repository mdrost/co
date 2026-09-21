co::executor_priority_0
=======================

Scale of executor priorities, used by :doc:`executor_priority_info_0`.

.. code-block:: cpp

   #include <co/executor_priority_0.hpp>

A priority is a ``std::intptr_t``. Only the order of priorities is meaningful: ``co::executor_priority_0_normal`` (0) is
normal, greater is more urgent. ``INTPTR_MIN`` is not a priority. The generic constants
``executor_priority_0_lowest``, ``_low``, ``_normal``, ``_high`` and ``_highest`` are symmetric around normal. See the C
API for how classes map priorities onto the levels of their backend.

.. doxygengroup:: cpp_executor_priority_0
   :content-only:
   :members:
