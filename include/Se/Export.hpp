#pragma once

#if LIB_STATIC
#  define LIB_EXPORT_API
#  define LIB_IMPORT_API
#else

#if defined _WIN32 || defined __CYGWIN__
#  define LIB_EXPORT_API __attribute__ ((dllexport))
#  define LIB_IMPORT_API __attribute__ ((dllimport))
#elif (__GNUC__ >= 4)
#  define LIB_EXPORT_API __attribute__ ((visibility ("default")))
#  define LIB_IMPORT_API __attribute__ ((visibility ("default")))
#else
#  define LIB_EXPORT_API __declspec(dllexport)
#  define LIB_IMPORT_API __declspec(dllimport)
#endif

#endif //LIB_STATIC