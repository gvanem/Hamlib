/*
 *  Hamlib Interface - numeric locale wrapping helpers
 *  Copyright (c) 2009 by Stephane Fillod
 *
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef _NUM_STDIO_H
#define _NUM_STDIO_H 1

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * This header file is internal to Hamlib and its backends,
 * thus not part of the API.
 */

/*
 * Wrapper for sscanf to workaround some locales where the decimal
 * separator (float, ...) is not the dot.
 */
#if defined(_MSC_VER)
  /*
   * MSVC have problem with the below 'num_sscanf()' macro in netrigctl.c.
   * And also some issues with 'num_sprintf()'. So use inline functions
   * for all of these instead.
   */
  static __inline int num_sscanf (const char *buf, const char *fmt, ...)
  {
    const char *savedlocale = setlocale (LC_NUMERIC, NULL);
    int     ret;
    va_list args;

    va_start (args, fmt);
    setlocale (LC_NUMERIC, "C");
    ret = vsscanf (buf, fmt, args);
    va_end (args);
    setlocale (LC_NUMERIC, savedlocale);
    return (ret);
  }

  static __inline int num_sprintf (char *buf, const char *fmt, ...)
  {
    _locale_t loc = NULL; // GetLocaleForCP (CP_ACP);
    int       ret;
    va_list   args;

    va_start (args, fmt);
    ret = _vsprintf_l (buf, fmt, loc, args);
    va_end (args);
    return (ret);
  }

  static __inline int num_snprintf (char *buf, size_t max, const char *fmt, ...)
  {
    const char *savedlocale = setlocale (LC_NUMERIC, NULL);
    int     ret;
    va_list args;

    va_start (args, fmt);
    setlocale (LC_NUMERIC, "C");
    ret = vsnprintf (buf, max, fmt, args);
    va_end (args);
    setlocale (LC_NUMERIC, savedlocale);
    return (ret);
  }

#else
  #define num_sscanf(a...) \
      ({ int __ret; char *__savedlocale; \
         __savedlocale = setlocale(LC_NUMERIC, NULL); \
         setlocale(LC_NUMERIC, "C"); \
         __ret = sscanf(a); \
         setlocale(LC_NUMERIC, __savedlocale); \
         __ret; \
       })

  #define num_sprintf(s, a...) \
      ({ int __ret; char *__savedlocale; \
         __savedlocale = setlocale(LC_NUMERIC, NULL); \
         setlocale(LC_NUMERIC, "C"); \
         __ret = sprintf(s, a); \
         setlocale(LC_NUMERIC, __savedlocale); \
         __ret; \
       })

  #define num_snprintf(s, n, a...) \
      ({ int __ret; char *__savedlocale; \
         __savedlocale = setlocale(LC_NUMERIC, NULL); \
         setlocale(LC_NUMERIC, "C"); \
         __ret = snprintf(s, n, a); \
         setlocale(LC_NUMERIC, __savedlocale); \
         __ret; \
       })
#endif  /* _MSC_VER */

#endif  /* _NUM_STDIO_H */
