#pragma once

#if defined(_WIN32)
# ifdef CO_WIN32_STATIC
#  define CO_WIN32_API
# else
#  ifndef CO_WIN32_API
#   ifdef CO_WIN32_EXPORTS
// We are building this library
#    define CO_WIN32_API __declspec(dllexport)
#   else
// We are using this library
#    define CO_WIN32_API __declspec(dllimport)
#   endif
#  endif
# endif
#else
// TODO:
#endif
