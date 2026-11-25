#ifndef HOLLOWENGINE_H
#define HOLLOWENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__)
#	define HOLLOW_API __attribute__((visibility("default")))
#else
#	define HOLLOW_API
#endif

HOLLOW_API void HOLLOWENGINE_INIT(void);
HOLLOW_API void HOLLOWENGINE_UPDATE(double delta_time);
HOLLOW_API void HOLLOWENGINE_SHUTDOWN(void);

#ifdef __cplusplus
}
#endif

#endif
