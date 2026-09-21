co
==

``co`` is an asynchronous runtime for C and C++. Applications post work to executors and wait for events, such as
timers, through event loops and event facilities, which can be built-in, provided by the operating system, or
supplied by plugin modules. Its C API is meant to stay binary compatible across library versions; header-only C++
wrappers and coroutine types are built on top of it.

- **Executors** run handlers in an execution context: an event loop, a thread pool, or a strand that serializes
  handlers on top of another executor. Each handler that an executor accepts is invoked exactly once.
- **Event domains** (event loops and event facilities) provide services, such as the timer service. You either name
  the class you want or state requirements, such as a platform thread pool or a fixed number of threads, and the
  instance picks a class that can meet them.
- **Extensible classes.** Event domain classes are registered when the instance is created: the built-in ones, the
  Win32 ones (I/O completion port event loops and an adapter of the Windows default thread pool), and classes from
  your own modules. Executor classes need no registration.
- **Versioned API.** Every public symbol carries a version suffix (``co_timer_0_start_0``, ``co::timer_0``), the
  instance behaves as specified by the API version it was created with, and vtables are only ever appended to. New
  versions are added next to old ones instead of changing them.
- **C++ coroutines.** ``co::task_0`` runs coroutines on executors, and ``co::awaitable_timer_0`` lets coroutines start
  a timer, wait for it and cancel it with ``co_await``.

``co`` is in early development: nothing has been released yet, and the API may still change.

See :doc:`concepts` for how instances, classes, objects and next chains fit together.

.. toctree::
   :maxdepth: 2
   :caption: Contents

   concepts

.. toctree::
   :maxdepth: 2
   :caption: API Reference

   api/c/index
   api/cpp/index
