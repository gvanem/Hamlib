/*
 * Posix Threads library for Microsoft Windows
 *
 * Use at own risk, there is no implied warranty to this code.
 * It uses undocumented features of Microsoft Windows that can change
 * at any time in the future.
 *
 * (C) 2010 Lockless Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of Lockless Inc. nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AN
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Version 1.0.1 Released 2 Feb 2012
 * Fixes pthread_barrier_destroy() to wait for threads to exit the barrier.
 *
 * Version 1.0.2, <gvanem@yahoo.no> 2019
 * Many fixes for a Hamlib build using MSVC or clang-cl.
 * Moved all variables and static functions into 'lib/win-misc.c'.
 * Removed a lot of dead code, variables and macros.
 */
#if !defined(_WIN32) && !defined(GCC_MAKE_DEPEND)
#error "This header is for '_WIN32' only. Revise your '-I' path"
#endif

#ifndef LOCKLESS_PTHREADS
#define LOCKLESS_PTHREADS

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PTHREAD_CANCEL_ENABLE        0x01
#define PTHREAD_CANCEL_ASYNCHRONOUS  0x02
#define PTHREAD_DEFAULT_ATTR         PTHREAD_CANCEL_ENABLE
#define PTHREAD_CREATE_JOINABLE      0
#define PTHREAD_CREATE_DETACHED      4

/**
 * \def PTHREAD_MUTEX_INITIALIZER
 * This value matches the layout of `CRITICAL_SECTION`.
 * I.e. `_RTL_CRITICAL_SECTION`:
 * \code
 * typedef struct _RTL_CRITICAL_SECTION {
 *     PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
 *     //
 *     //  The following three fields control entering and exiting the critical
 *     //  section for the resource
 *     //
 *     LONG      LockCount;
 *     LONG      RecursionCount;
 *     HANDLE    OwningThread;    // from the thread's ClientId->UniqueThread
 *     HANDLE    LockSemaphore;
 *     ULONG_PTR SpinCount;        // force size on 64-bit systems when packed
 * } RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;
 * \endcode
 */
#define PTHREAD_MUTEX_INITIALIZER    { (void*)-1, -1, 0, 0, 0, 0 }

struct _pthread_v;
typedef struct _pthread_v *pthread_t;

typedef void* (*pthread_func_t)(void*);

typedef struct pthread_attr_t {
        unsigned  p_state;
        void     *stack;
        size_t    s_size;
      } pthread_attr_t;

typedef unsigned          pthread_mutexattr_t;
typedef CRITICAL_SECTION  pthread_mutex_t;

pthread_t pthread_self                (void);
int       pthread_exit                (void *res);
int       pthread_join                (pthread_t t, void **res);
int       pthread_create              (pthread_t *t, pthread_attr_t *a, pthread_func_t func, void *arg);
int       pthread_cancel              (pthread_t t);
int       pthread_attr_init           (pthread_attr_t *a);
int       pthread_attr_setdetachstate (pthread_attr_t *a, int flag);
int       pthread_mutex_init          (pthread_mutex_t *m, pthread_mutexattr_t *a);
int       pthread_mutex_destroy       (pthread_mutex_t *m);
int       pthread_mutex_lock          (pthread_mutex_t *m);
int       pthread_mutex_unlock        (pthread_mutex_t *m);

#ifdef __cplusplus
}
#endif

#endif /* LOCKLESS_PTHREADS */
