
#include <hamlib/config.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <setjmp.h>
#include <time.h>
#include <windows.h>
#include "src/misc.h"
#include "lib/pthread.h"

#if defined(__MINGW32__)
#error "This file is not needed for MinGW"
#endif

/*
 * Internals for the Lockless Inc. version of Pthreads.
 */
static volatile long  _pthread_cancelling;
static DWORD          _pthread_tls;
static long           _pthread_tls_once;

typedef struct _pthread_v {
        void             *ret_arg;
        pthread_func_t    func;
        HANDLE            hnd;
        int               cancelled;
        unsigned          p_state;
        jmp_buf           jb;
      } _pthread_v;

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
  static bool          usePerformanceCounter = 1;

  if (!initialized)
  {
    LARGE_INTEGER performanceFrequency;

    initialized = 1;
    if (usePerformanceCounter)
    {
      QueryPerformanceFrequency (&performanceFrequency);
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

/*
 * Number of micro-seconds between the beginning of the Windows epoch
 * (Jan. 1, 1601) and the Unix epoch (Jan. 1, 1970).
 */
#define DELTA_EPOCH_IN_USEC  11644473600000000Ui64

static uint64_t FILETIME_to_unix_epoch (const FILETIME *ft)
{
  uint64_t res = (uint64_t) ft->dwHighDateTime << 32;

  res |= ft->dwLowDateTime;
  res /= 10;                   /* from 100 nano-sec periods to usec */
  res -= DELTA_EPOCH_IN_USEC;  /* from Win epoch to Unix epoch */
  return (res);
}

int nanosleep (const struct timespec *ts, struct timespec *remain)
{
  assert (remain == NULL);
  SleepEx (1000 *ts->tv_sec + ts->tv_nsec/1000000, TRUE);
  return (0);
}

int gettimeofday (struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  uint64_t tim;

  if (!tv)
      return (EINVAL);

  GetSystemTimePreciseAsFileTime (&ft);
  tim = FILETIME_to_unix_epoch (&ft);
  tv->tv_sec  = (long) (tim / 1000000L);
  tv->tv_usec = (long) (tim % 1000000L);
  (void) tz;
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

    return (start);     /* Return the position where the string starts */
  }

  /* We ended up on a null byte, there are no more strings to find!
   */
  return (NULL);
}

static void _pthread_tls_init (void)
{
  _pthread_tls = TlsAlloc();
  if (_pthread_tls == TLS_OUT_OF_INDEXES)  /* Cannot continue if out of indexes */
    abort();
}

static int _pthread_once_raw (long *o, void (*func) (void))
{
  long state = *o;

  _ReadWriteBarrier();

  while (state != 1)
  {
    if (!state)
    {
      if (!_InterlockedCompareExchange (o, 2, 0))
      {
        (*func)();   /* Success */
        *o = 1;      /* Mark as done */
        return (0);
      }
    }
    YieldProcessor();
    _ReadWriteBarrier();
    state = *o;
  }
  return (0);   /* Done */
}

static int _pthread_create_wrapper (void *args)
{
  _pthread_v *tv = (_pthread_v*) args;

  _pthread_once_raw (&_pthread_tls_once, _pthread_tls_init);
  TlsSetValue (_pthread_tls, tv);

  if (!setjmp(tv->jb))
  {
    /* Call function and save return value */
    tv->ret_arg = (*tv->func) (tv->ret_arg);
  }

  /* If we exit too early, then we can race with create */
  while (tv->hnd == INVALID_HANDLE_VALUE)
  {
    YieldProcessor();
    _ReadWriteBarrier();
  }

  /* Make sure we free ourselves if we are detached */
  if (!tv->hnd)
     free (tv);
  return (0);
}

static int _pthread_set_state (pthread_attr_t *attr, unsigned flag, unsigned val)
{
  if (~flag & val)
     return (EINVAL);

  attr->p_state &= ~flag;
  attr->p_state |= val;
  return (0);
}

int pthread_attr_init (pthread_attr_t *attr)
{
  attr->p_state = PTHREAD_DEFAULT_ATTR;
  attr->stack   = NULL;
  attr->s_size  = 0;
  return (0);
}

int pthread_attr_setdetachstate (pthread_attr_t *a, int flag)
{
  return _pthread_set_state (a, PTHREAD_CREATE_DETACHED, flag);
}

unsigned _pthread_get_state (pthread_attr_t *attr, unsigned flag)
{
  return attr->p_state & flag;
}

pthread_t pthread_self (void)
{
  pthread_t t;

  _pthread_once_raw (&_pthread_tls_once, _pthread_tls_init);
  t = (pthread_t) TlsGetValue (_pthread_tls);
  t->ret_arg = NULL;

  /* Main thread? */
  if (t)
  {
    t = (pthread_t) malloc (sizeof(struct _pthread_v));

    /* If cannot initialize main thread, then the only thing we can do is abort */
    if (!t)
      abort();

    t->ret_arg   = NULL;
    t->func      = NULL;
    t->cancelled = 0;
    t->p_state   = PTHREAD_DEFAULT_ATTR;
    t->hnd       = GetCurrentThread();

    /* Save for later */
    TlsSetValue (_pthread_tls, t);

    if (setjmp(t->jb))
    {
      /* Make sure we free ourselves if we are detached */
      if (!t->hnd)
         free (t);

      /* Time to die */
      _endthreadex (0);
    }
  }
  return (t);
}

static void _pthread_invoke_cancel (void)
{
  _InterlockedDecrement (&_pthread_cancelling);
  pthread_exit (NULL);
}

static void _pthread_testcancel (void)
{
  if (_pthread_cancelling)
  {
    pthread_t t = pthread_self();

    if (t->cancelled && (t->p_state & PTHREAD_CANCEL_ENABLE))
       _pthread_invoke_cancel();
  }
}

int pthread_create (pthread_t *th, pthread_attr_t *attr, pthread_func_t func, void *func_arg)
{
  _pthread_v *tv = malloc (sizeof(*tv));
  unsigned ssize = 0;

  if (!tv)
     return (1);

  *th = tv;

  /* Save data in '*tv' */
  tv->ret_arg   = func_arg;
  tv->func      = func;
  tv->cancelled = 0;
  tv->p_state   = PTHREAD_DEFAULT_ATTR;
  tv->hnd       = INVALID_HANDLE_VALUE;

  if (attr)
  {
    tv->p_state = attr->p_state;
    ssize = (unsigned int) attr->s_size;
  }

  _ReadWriteBarrier();
  tv->hnd = (HANDLE) _beginthreadex (NULL, ssize, (_beginthreadex_proc_type)_pthread_create_wrapper, tv, 0, NULL);
  if (!tv->hnd)
  {
    free (tv);
    return (1);
  }

  if (tv->p_state & PTHREAD_CREATE_DETACHED)
  {
    CloseHandle (tv->hnd);
    _ReadWriteBarrier();
    tv->hnd = 0;
  }
  return (0);
}

int pthread_join (pthread_t t, void **res)
{
  _pthread_v *tv = t;

  _pthread_testcancel();
  WaitForSingleObject (tv->hnd, INFINITE);
  CloseHandle (tv->hnd);

  if (res)
     *res = tv->ret_arg;

  free (tv);
  return (0);
}

int pthread_cancel (pthread_t t)
{
  rig_debug (RIG_DEBUG_VERBOSE, "%s() called\n", __func__);

  if (t->p_state & PTHREAD_CANCEL_ASYNCHRONOUS)
  {
    CONTEXT ctxt;

    if (t->cancelled)   /* Already done? */
    {
      rig_debug (RIG_DEBUG_VERBOSE, "  t->cancelled==1\n");
      return (ESRCH);
    }

    ctxt.ContextFlags = CONTEXT_CONTROL;
    SuspendThread (t->hnd);
    GetThreadContext (t->hnd, &ctxt);

#ifdef _M_X64
    ctxt.Rip = (uintptr_t) _pthread_invoke_cancel;
#else
    ctxt.Eip = (uintptr_t) _pthread_invoke_cancel;
#endif

    SetThreadContext (t->hnd, &ctxt);
    t->cancelled = 1;                              /* Also try deferred Cancelling */
    _InterlockedIncrement (&_pthread_cancelling);  /* Notify everyone to look */
    ResumeThread (t->hnd);
  }
  else
  {
    t->cancelled = 1;                              /* Safe deferred Cancelling */
    _InterlockedIncrement (&_pthread_cancelling);  /* Notify everyone to look */
  }
  return (0);
}

int pthread_exit (void *res)
{
  pthread_t t = pthread_self();

  rig_debug (RIG_DEBUG_VERBOSE, "%s() called\n", __func__);

  t->ret_arg = res;
  longjmp (t->jb, 1);
}

int pthread_mutex_lock (pthread_mutex_t *m)
{
  EnterCriticalSection (m);
  return (0);
}

int pthread_mutex_unlock (pthread_mutex_t *m)
{
  LeaveCriticalSection (m);
  return (0);
}

int pthread_mutex_trylock (pthread_mutex_t *m)
{
  return TryEnterCriticalSection (m) ? 0 : EBUSY;
}

int pthread_mutex_init (pthread_mutex_t *m, pthread_mutexattr_t *a)
{
  (void) a;
  InitializeCriticalSection (m);
  return (0);
}

int pthread_mutex_destroy (pthread_mutex_t *m)
{
  DeleteCriticalSection (m);
  return (0);
}
