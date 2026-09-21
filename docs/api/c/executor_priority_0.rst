co_executor_priority_0
======================

Scale of executor priorities, used by :doc:`executor_priority_info_0`.

.. code-block:: c

   #include <co/executor_priority_0.h>

A priority is an ``intptr_t``. Only the order of priorities is meaningful: ``CO_EXECUTOR_PRIORITY_0_NORMAL`` (0) is
normal, greater is more urgent. ``INTPTR_MIN`` is not a priority.

A class whose backend has N levels splits the range from ``CO_EXECUTOR_PRIORITY_0_LOWEST`` to
``CO_EXECUTOR_PRIORITY_0_HIGHEST`` into N bands of equal width, in order, and maps every priority of a band to its
level; with an odd N the middle band is centered on normal. Platform headers define a constant at the center of each
band, e.g. :doc:`win32/win32_executor_priority_0` for ``TP_CALLBACK_PRIORITY``.

The generic constants are symmetric around normal: ``CO_EXECUTOR_PRIORITY_0_LOWEST`` (``-INTPTR_MAX``),
``CO_EXECUTOR_PRIORITY_0_LOW`` (``-(INTPTR_MAX / 2)``), ``CO_EXECUTOR_PRIORITY_0_NORMAL``,
``CO_EXECUTOR_PRIORITY_0_HIGH`` (``INTPTR_MAX / 2``) and ``CO_EXECUTOR_PRIORITY_0_HIGHEST`` (``INTPTR_MAX``), leaving
room for constants matching the levels of specific platforms.

.. doxygengroup:: c_executor_priority_0
   :content-only:
   :members:
