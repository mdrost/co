co_dynamic_thread_pool_0
========================

Event facilities running their handlers on a thread pool they own, whose number of threads varies.

.. code-block:: c

   #include <co/dynamic_thread_pool_0.h>
   #include <co/thread_count_bounds.h>

A dynamic thread pool is an abstract kind: there is no ``co_dynamic_thread_pool_0`` handle. Put a
``co_dynamic_thread_pool_0_create_info_0`` in the next chain of a ``co_event_facility_0_create_info_0`` and the instance
creates a ``co_event_facility_0`` of a class handling it, as if ``co_dynamic_thread_pool_0`` were a kind derived from
``co_event_facility_0``. Use it through ``co_event_facility_0`` and ``co_event_domain_0``. The class chooses the
bounds of the number of threads unless a ``co_thread_count_bounds_info_0`` is in the same next chain.

Example
-------

.. code-block:: c

   const co_thread_count_bounds_info_0 thread_count_bounds = { /* optional */
	   .structure_type = CO_STRUCTURE_TYPE_0_THREAD_COUNT_BOUNDS_INFO_0,
	   .next_structure = NULL,
	   .minimum_thread_count = 1,
	   .maximum_thread_count = 8,
   };
   const co_dynamic_thread_pool_0_create_info_0 dynamic_thread_pool = {
	   .structure_type = CO_STRUCTURE_TYPE_0_DYNAMIC_THREAD_POOL_0_CREATE_INFO_0,
	   .next_structure = CO_IN_STRUCTURE_0_CAST(&thread_count_bounds),
   };
   const co_event_facility_0_create_info_0 event_facility_create_info = {
	   .structure_type = CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0,
	   .next_structure = CO_IN_STRUCTURE_0_CAST(&dynamic_thread_pool),
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

.. doxygengroup:: c_dynamic_thread_pool
   :content-only:
   :members:
