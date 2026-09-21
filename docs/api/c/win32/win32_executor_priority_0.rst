co_win32_executor_priority_0
============================

Executor priorities matching the levels of ``TP_CALLBACK_PRIORITY``.

.. code-block:: c

   #include <co/win32/executor_priority_0.h>

Includes ``co/executor_priority_0.h``. A Win32 thread pool class maps a priority of :doc:`../executor_priority_0` onto the
three levels of ``TP_CALLBACK_PRIORITY`` with three bands of equal width, the middle one centered on
``CO_EXECUTOR_PRIORITY_0_NORMAL``. ``CO_WIN32_EXECUTOR_PRIORITY_0_LOW``, ``_NORMAL`` and ``_HIGH`` are the centers of
the bands.

.. doxygengroup:: c_win32_executor_priority_0
   :content-only:
   :members:
