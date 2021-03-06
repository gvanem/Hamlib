#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <time.h>
#include <windows.h>

/*
 * Ripped from:
 *   https://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
 *
 * Return the difference between Windows Epoch and Posix Epoch:
 * The seconds from '1 Jan 1601' to '1 Jan 1970'.
 */
static LARGE_INTEGER get_FILETIME_offset (void)
{
  SYSTEMTIME    st;
  FILETIME      ft;
  LARGE_INTEGER ret;

  st.wYear   = 1970;
  st.wMonth  = 1;
  st.wDay    = 1;
  st.wHour   = 0;
  st.wMinute = 0;
  st.wSecond = 0;
  st.wMilliseconds = 0;
  SystemTimeToFileTime (&st, &ft);
  ret.QuadPart = ft.dwHighDateTime;
  ret.QuadPart <<= 32;
  ret.QuadPart |= ft.dwLowDateTime;
  return (ret);
}

int clock_gettime (int clock_id, struct timespec *ts)
{
  LARGE_INTEGER        t;
  FILETIME             f;
  double               microseconds;
  static LARGE_INTEGER offset;
  static double        frequencyToMicroseconds;
  static int           initialized = 0;
  static BOOL          usePerformanceCounter = 0;

  if (!initialized)
  {
    LARGE_INTEGER performanceFrequency;

    initialized = 1;
    usePerformanceCounter = QueryPerformanceFrequency (&performanceFrequency);
    if (usePerformanceCounter)
    {
      QueryPerformanceCounter (&offset);
      frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.0;
    }
    else
    {
      offset = get_FILETIME_offset();
      frequencyToMicroseconds = 10.0;
    }
  }

  if (usePerformanceCounter)
     QueryPerformanceCounter (&t);
  else
  {
    GetSystemTimeAsFileTime (&f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
  }

  t.QuadPart  -= offset.QuadPart;
  microseconds = (double)t.QuadPart / frequencyToMicroseconds;
  t.QuadPart   = microseconds;
  ts->tv_sec   = t.QuadPart / 1000000;
  ts->tv_nsec  = t.QuadPart % 1000;
  (void) clock_id;
  return (0);
}

#if !defined(HAVE_STRTOK_R)
/*
 * A 'strtok_r()' function taken from libcurl:
 *
 * Copyright (C) 1998 - 2007, Daniel Stenberg, <daniel@haxx.se>, et al.
 */
char *strtok_r (char *s, const char *delim, char **ptrptr)
{
  if (!s)
  {
    /* We got NULL input so then we get our last position instead
     */
    s = *ptrptr;
  }

  /* Pass all letters that are including in the separator string
   */
  while (*s && strchr(delim, *s))
    ++s;

  if (*s)
  {
    /* So this is where the next piece of string starts
     */
    char *start = s;

    /* Set the end pointer to the first byte after the start
     */
    *ptrptr = start + 1;

    /* Scan through the string to find where it ends, it ends on a
     * null byte or a character that exists in the separator string
     */
    while (**ptrptr && !strchr(delim, **ptrptr))
      ++*ptrptr;

    if (**ptrptr)
    {
      /* The end is not a null byte
       */
      **ptrptr = '\0';  /* Zero terminate it! */
      ++*ptrptr;        /* Advance the last pointer to beyond the null byte */
    }

    return (start);     /* Return the position where the string starts */
  }

  /* We ended up on a null byte, there are no more strings to find!
   */
  return (NULL);
}
#endif