#pragma once

#if defined(_WIN32)
# ifdef CO_STATIC
#  define CO_API
# else
#  ifndef CO_API
#   ifdef CO_EXPORTS
// We are building this library
#    define CO_API __declspec(dllexport)
#   else
// We are using this library
#    define CO_API __declspec(dllimport)
#   endif
#  endif
# endif
#else
// TODO:
#endif
