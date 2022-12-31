#ifndef IVY_DEBUG_LOG_H
#define IVY_DEBUG_LOG_H

void ivyLogImplementation(char const *functionName, char const *format, ...);

#if defined(__GNUC__) || defined(__clang__)
#define IVY_CURRENT_FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define IVY_CURRENT_FUNCTION_NAME __func__
#else
#define IVY_CURRENT_FUNCTION_NAME "(unknown)"
#endif

#ifndef NDEBUG
#define IVY_DEBUG_LOG(format, ...)                                            \
  ivyLogImplementation(IVY_CURRENT_FUNCTION_NAME, format, __VA_ARGS__);
#else
#define IVY_DEBUG_LOG(format, ...)
#endif

#endif
