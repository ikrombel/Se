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

#ifndef CONCATENATE
#define CONCATENATE(PRIFIX, NAME)  {PRIFIX ## NAME}
#endif

// #define MODULE_LIB(name) \
//     #ifdef CONCATENATE(name, _STATIC_LIB) \
//     #  define CONCATENATE(name, _API) \
//     #elif defined CONCATENATE(name, _EXPORT) \
//     #  define CONCATENATE(name, _API) LIB_EXPORT_API \
//     #else \
//     #  define CONCATENATE(name, _API) LIB_IMPORT_API \
//     #endif

// MODULE_LIB(SEA)

#if SE_STATIC_LIB
#  define SE_API
#elif SE_EXPORT
#  define SE_API LIB_EXPORT_API
#else
#  define SE_API LIB_IMPORT_API
#endif


#endif //LIB_STATIC