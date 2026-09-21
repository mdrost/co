co_platform_thread_pool_0
=========================

Event facilities adapting the thread pool of the platform, the runtime the application runs on: the operating system
(for example the default process thread pool on Windows) or an application framework (for example the global thread
pool of Qt).

.. code-block:: c

   #include <co/platform_thread_pool_0.h>

A platform thread pool is an abstract kind: there is no ``co_platform_thread_pool_0`` handle. Put a
``co_platform_thread_pool_0_create_info_0`` in the next chain of a ``co_event_facility_0_create_info_0`` and the instance
creates a ``co_event_facility_0`` of a class handling it, as if ``co_platform_thread_pool_0`` were a kind derived from
``co_event_facility_0``. Use it through ``co_event_facility_0`` and ``co_event_domain_0``. To ask for a given platform,
use the create info of its class instead, for example
``co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0``.

Example
-------

.. code-block:: c

   const co_platform_thread_pool_0_create_info_0 platform_thread_pool = {
	   .structure_type = CO_STRUCTURE_TYPE_0_PLATFORM_THREAD_POOL_0_CREATE_INFO_0,
	   .next_structure = NULL,
   };
   const co_event_facility_0_create_info_0 event_facility_create_info = {
	   .structure_type = CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0,
	   .next_structure = CO_IN_STRUCTURE_0_CAST(&platform_thread_pool),
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

.. doxygengroup:: c_platform_thread_pool
   :content-only:
   :members:
