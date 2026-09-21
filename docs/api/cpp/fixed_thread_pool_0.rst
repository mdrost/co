co::fixed_thread_pool_0
=======================

Event facilities running their handlers on a thread pool of a fixed number of threads they own.

.. code-block:: cpp

   #include <co/fixed_thread_pool_0.hpp>

A fixed thread pool is an abstract kind: there is no ``co::fixed_thread_pool_0`` type. Put a
``co::fixed_thread_pool_0_create_info_0`` in the next chain of a ``co::event_facility_0_create_info_0`` and the instance
creates a ``co::event_facility_0`` of a class handling it, as if ``co::fixed_thread_pool_0`` were a kind derived from
``co::event_facility_0``.

.. doxygengroup:: cpp_fixed_thread_pool
   :content-only:
   :members:
