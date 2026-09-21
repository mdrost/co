co::impl
========

Helpers for implementing classes (event domains, services, executors, factories), in the library, in an extension
library or in a plugin.

.. code-block:: cpp

   #include <co/impl.hpp>

The helpers are header-only and use only public C structures: each module compiles its own copy, so nothing crosses a
module boundary and a class depends on no internals of the library.

An object is allocated with the ``co_allocation_callbacks_0`` it is created with and released with the same callbacks.
``new_object_0()`` and ``delete_object_0()`` pass the size and alignment of the type, as ``new`` and ``delete`` do, so
over-aligned types work with any callbacks:

.. code-block:: cpp

   // in the create function; the constructor stores the callbacks in the object:
   my_executor* executor = co::impl::new_object_0<my_executor>(allocation_callbacks, allocation_callbacks);
   if (executor == nullptr) {
	   return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
   }

   // in the destroy method:
   co::impl::delete_object_0(executor->allocation_callbacks, executor);

``delete_object_0()`` copies the callbacks before destroying the object, so they may be a member of the object.

.. doxygengroup:: cpp_impl
   :content-only:
   :members:
