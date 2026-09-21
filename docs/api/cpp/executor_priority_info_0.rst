co::executor_priority_info_0
============================

Priority of the handlers posted to an executor relative to other work of the same execution context.

.. code-block:: cpp

   #include <co/executor_priority_0.hpp>

Put it in the next chain of a ``co::executor_0_create_info_0``, at most once; directly as a requirement or wrapped in a
``co::hint_info_0`` as a preference. ``priority`` is on the scale of :doc:`executor_priority_0`.

.. doxygengroup:: cpp_executor_priority_info_0
   :content-only:
   :members:
