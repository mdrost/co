co_fixed_thread_pool_0
======================

Event facilities running their handlers on a thread pool of a fixed number of threads they own.

.. code-block:: c

   #include <co/fixed_thread_pool_0.h>

A fixed thread pool is an abstract kind: there is no ``co_fixed_thread_pool_0`` handle. Put a
``co_fixed_thread_pool_0_create_info_0`` in the next chain of a ``co_event_facility_0_create_info_0`` and the instance
creates a ``co_event_facility_0`` of a class handling it, as if ``co_fixed_thread_pool_0`` were a kind derived from
``co_event_facility_0``. Use it through ``co_event_facility_0`` and ``co_event_domain_0``.

Example
-------

.. code-block:: c

   const co_fixed_thread_pool_0_create_info_0 fixed_thread_pool = {
	   .structure_type = CO_STRUCTURE_TYPE_0_FIXED_THREAD_POOL_0_CREATE_INFO_0,
	   .next_structure = NULL,
	   .thread_count = 4,
   };
   const co_event_facility_0_create_info_0 event_facility_create_info = {
	   .structure_type = CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0,
	   .next_structure = CO_IN_STRUCTURE_0_CAST(&fixed_thread_pool),
	   .required_services = NULL,
	   .required_service_count = 0,
   };
   co_event_domain_0 event_domain;
   if (co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&event_facility_create_info), &event_domain) == CO_RESULT_0_SUCCESS) {
	   co_event_facility_0 thread_pool = CO_EVENT_FACILITY_0_CAST(event_domain);
	   /* ... */
	   co_event_domain_0_destroy(event_domain);
   }

API reference
-------------

.. doxygengroup:: c_fixed_thread_pool
   :content-only:
   :members:
