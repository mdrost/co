co_executor_priority_info_0
===========================

Priority of the handlers posted to an executor relative to other work of the same execution context.

.. code-block:: c

   #include <co/executor_priority_0.h>

Put it in the next chain of a ``co_executor_0_create_info_0``, at most once. ``priority`` is on the scale of
:doc:`executor_priority_0`.

Put directly in the chain, it is a requirement: classes that cannot honor it decline with
``CO_RESULT_0_ERROR_NOT_SUPPORTED``. Usually it is a preference, wrapped in a ``co_hint_info_0``:

.. code-block:: c

   const co_executor_priority_info_0 priority = {
	   .structure_type = CO_STRUCTURE_TYPE_0_EXECUTOR_PRIORITY_INFO_0,
	   .next_structure = NULL,
	   .priority = CO_EXECUTOR_PRIORITY_0_HIGH,
   };
   const co_hint_info_0 priority_hint = {
	   .structure_type = CO_STRUCTURE_TYPE_0_HINT_INFO_0,
	   .next_structure = NULL,
	   .hint = CO_IN_STRUCTURE_0_CAST(&priority),
   };
   const co_executor_0_create_info_0 executor_create_info = {
	   .structure_type = CO_STRUCTURE_TYPE_0_EXECUTOR_0_CREATE_INFO_0,
	   .next_structure = CO_IN_STRUCTURE_0_CAST(&priority_hint),
   };

.. doxygengroup:: c_executor_priority_info_0
   :content-only:
   :members:
