#ifndef _KT_STRPTIME_H_
#define _KT_STRPTIME_H_

#ifdef TARGET_PLATFORM_WINDOWS

char *strptime(const char *buf, const char *fmt, struct tm *tm);
#endif

#endif
