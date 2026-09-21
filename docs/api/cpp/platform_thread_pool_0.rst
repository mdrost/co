co::platform_thread_pool_0
==========================

Event facilities adapting the thread pool of the platform, the runtime the application runs on: the operating system
or an application framework.

.. code-block:: cpp

   #include <co/platform_thread_pool_0.hpp>

A platform thread pool is an abstract kind: there is no ``co::platform_thread_pool_0`` type. Put a
``co::platform_thread_pool_0_create_info_0`` in the next chain of a ``co::event_facility_0_create_info_0`` and the
instance creates a ``co::event_facility_0`` of a class handling it, as if ``co::platform_thread_pool_0`` were a kind
derived from ``co::event_facility_0``.

.. doxygengroup:: cpp_platform_thread_pool
   :content-only:
   :members:
