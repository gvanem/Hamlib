/*
 * Ripped from:
 *   https://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
 */
#include "config.h"

static LARGE_INTEGER getFILETIMEoffset (void)
{
  SYSTEMTIME    s;
  FILETIME      f;
  LARGE_INTEGER t;

  s.wYear = 1970;
  s.wMonth = 1;
  s.wDay = 1;
  s.wHour = 0;
  s.wMinute = 0;
  s.wSecond = 0;
  s.wMilliseconds = 0;
  SystemTimeToFileTime(&s, &f);
  t.QuadPart = f.dwHighDateTime;
  t.QuadPart <<= 32;
  t.QuadPart |= f.dwLowDateTime;
  return (t);
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
      frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
    }
    else
    {
      offset = getFILETIMEoffset();
      frequencyToMicroseconds = 10.;
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

  t.QuadPart -= offset.QuadPart;
  microseconds = (double)t.QuadPart / frequencyToMicroseconds;
  t.QuadPart = microseconds;
  ts->tv_sec = t.QuadPart / 1000000;
  ts->tv_nsec = t.QuadPart % 1000;
  (void) clock_id;
  return (0);
}

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

    return start;       /* Return the position where the string starts */
  }

  /* We ended up on a null byte, there are no more strings to find!
   */
  return NULL;
}
