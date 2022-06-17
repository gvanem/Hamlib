/*
 *  Hamlib Interface - toolbox header
 *  Copyright (c) 2000-2004 by Stephane Fillod
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

#ifndef _MISC_H
#define _MISC_H 1

#include <hamlib/rig.h>

/*
 */
#ifdef HAVE_PTHREAD
  #include <pthread.h>
  #define set_transaction_active(rig)    do {                                                     \
                                           pthread_mutex_lock(&rig->state.mutex_set_transaction); \
                                           (rig)->state.transaction_active = 1;                   \
                                         } while (0)

  #define set_transaction_inactive(rig)  do {                                                       \
                                           (rig)->state.transaction_active = 0;                     \
                                           pthread_mutex_unlock(&rig->state.mutex_set_transaction); \
                                         } while (0)

  /*
   * In pthreads-win32 ver 2.8.1 (and later), 'pthread_t' is a structure:
   *
   *  typedef struct {
   *    void        *p;      // Pointer to actual object
   *    unsigned int x;      // Extra information - reuse count etc
   *  } ptw32_handle_t;
   *
   * typedef ptw32_handle_t pthread_t;
   *
   * Hence this doesn't work with MSVC (only gcc):
   *   pthread_cancel ((pthread_t)id);   // cast of struct to struct is not allowed.
   *
   * So for Windows, this PTHREAD_ID() macro will return something unique
   * since 'id->p' comes from a calloc() call in ptw32_new.c.
   */
  #if defined(_WIN32)
    #define PTHREAD_ID(id)          (*(unsigned long*)&(id))
    #define PTHREAD_ID_CLEAR(id)    memset (&(id), '\0', sizeof(id))
  #else
    #define PTHREAD_ID(id)          (*(unsigned long*)&(id))
    #define PTHREAD_ID_CLEAR(id)    PTHREAD_ID(id) = 0
  #endif

#else
  #define set_transaction_active(rig)    (rig)->state.transaction_active = 1
  #define set_transaction_inactive(rig)  (rig)->state.transaction_active = 0
#endif

__BEGIN_DECLS

// a function to return just a string of spaces for indenting rig debug lines
HAMLIB_EXPORT (const char *) spaces();
/*
 * Do a hex dump of the unsigned char array.
 */

void dump_hex(const unsigned char ptr[], size_t size);

/*
 * BCD conversion routines.
 *
 * to_bcd() converts a long long int to a little endian BCD array,
 *  and return a pointer to this array.
 *
 * from_bcd() converts a little endian BCD array to long long int
 *  representation, and return it.
 *
 * bcd_len is the number of digits in the BCD array.
 */
extern HAMLIB_EXPORT(unsigned char *) to_bcd(unsigned char bcd_data[],
                                             unsigned long long freq,
                                             unsigned bcd_len);

extern HAMLIB_EXPORT(unsigned long long) from_bcd(const unsigned char
                                                  bcd_data[],
                                                  unsigned bcd_len);

/*
 * same as to_bcd() and from_bcd(), but in Big Endian mode
 */
extern HAMLIB_EXPORT(unsigned char *) to_bcd_be(unsigned char bcd_data[],
                                                unsigned long long freq,
                                                unsigned bcd_len);

extern HAMLIB_EXPORT(unsigned long long) from_bcd_be(const unsigned char
                                                     bcd_data[],
                                                     unsigned bcd_len);

extern HAMLIB_EXPORT(size_t) to_hex(size_t source_length,
                                    const unsigned char *source_data,
                                    size_t dest_length,
                                    char *dest_data);

extern HAMLIB_EXPORT(double) morse_code_dot_to_millis(int wpm);
extern HAMLIB_EXPORT(int) dot10ths_to_millis(int dot10ths, int wpm);
extern HAMLIB_EXPORT(int) millis_to_dot10ths(int millis, int wpm);

extern HAMLIB_EXPORT(int) sprintf_freq(char *str, int str_len, freq_t);

/* flag that determines if AI mode should be restored on exit on
   applicable rigs - See rig_no_restore_ai() */
extern int no_restore_ai;

/* check if it's any of CR or LF */
#define isreturn(c) ((c) == 10 || (c) == 13)


/* needs config.h included beforehand in .c file */
#ifdef HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif

extern HAMLIB_EXPORT(int)       rig_check_cache_timeout(const struct timeval *tv, int timeout);
extern HAMLIB_EXPORT(void)      rig_force_cache_timeout(struct timeval *tv);
extern HAMLIB_EXPORT(setting_t) rig_idx2setting(int i);

extern HAMLIB_EXPORT(int)       hl_usleep(rig_useconds_t usec);
extern HAMLIB_EXPORT(double)    elapsed_ms(struct timespec *start, int start_flag);
extern HAMLIB_EXPORT(vfo_t)     vfo_fixup(RIG *rig, vfo_t vfo, split_t split);
extern HAMLIB_EXPORT(vfo_t)     vfo_fixup2a(RIG *rig, vfo_t vfo, split_t split, const char *func, const int line);
#define vfo_fixup(r,v,s)        vfo_fixup2a(r,v,s,__func__,__LINE__)

extern HAMLIB_EXPORT(int)       parse_hoststr(char *hoststr, int hoststr_len, char host[256], char port[6]);
extern HAMLIB_EXPORT(uint32_t)  CRC32_function(uint8_t *buf, uint32_t len);
extern HAMLIB_EXPORT(char *)    date_strget(char *buf, int buflen, int localtime);

#ifdef PRId64
/** \brief printf(3) format to be used for long long (64bits) type */
#  define PRIll PRId64
#  define PRXll PRIx64
#else
#  ifdef FBSD4
#    define PRIll "qd"
#    define PRXll "qx"
#  else
#    define PRIll "lld"
#    define PRXll "lld"
#  endif
#endif

/* And the same for 'unsigned long long' type.
 * No need for the hex format.
 */
#ifdef PRIu64
#  define PRIull PRIu64
#else
#  define PRIull "llu"
#endif

#ifdef SCNd64
/** \brief scanf(3) format to be used for long long (64bits) type */
#  define SCNll SCNd64
#  define SCNXll SCNx64
#else
#  ifdef FBSD4
#    define SCNll "qd"
#    define SCNXll "qx"
#  else
#    define SCNll "lld"
#    define SCNXll "llx"
#  endif
#endif

void errmsg(int err, char *s, const char *func, const char *file, int line);

#define TRACE   rig_debug(RIG_DEBUG_TRACE, "%s(%d) trace\n", __FILE__, __LINE__)

#if defined(USE_GV_DEBUG)
  /*
   * Returns basename from '__FILE__'
   */
  const char *rig_debug_filename(const char *file);

#else
  #define rig_debug_filename(file) (strrchr(file, '/') ? strrchr(file, '/') + 1 : file)
#endif

//
// Use this instead of snprintf for automatic detection of buffer limit
//
#define SNPRINTF(s, n, ...) do {                                              \
                              snprintf (s, n, ##__VA_ARGS__);                 \
                              if (strlen(s) > n - 1)                          \
                                 fprintf (stderr,                             \
                                   "****** %s(%d): buffer overflow ******\n", \
                                   __func__, __LINE__);                       \
                            } while (0)

#define ERRMSG(err, s) errmsg(err,  s, __func__, rig_debug_filename(__FILE__), __LINE__)

#define ENTERFUNC  do {                                                           \
                     ++rig->state.depth;                                          \
                     rig_debug(RIG_DEBUG_VERBOSE, "%.*s%d:%s(%d):%s entered\n",   \
                               rig->state.depth, spaces(), rig->state.depth,      \
                               rig_debug_filename(__FILE__), __LINE__, __func__); \
                   } while (0)

#define ENTERFUNC2 do {                                                           \
                     rig_debug(RIG_DEBUG_VERBOSE, "%s(%d):%s entered\n",          \
                               rig_debug_filename(__FILE__), __LINE__, __func__); \
                   } while (0)

// Measuring elapsed time -- local variable inside function when macro is used
#define ELAPSED1  struct timespec __begin;  \
                  elapsed_ms(&__begin, HAMLIB_ELAPSED_SET)

#define ELAPSED2  rig_debug(RIG_DEBUG_TRACE, "%.*s%d:%s: elapsed=%.0lfms\n", \
                            rig->state.depth, spaces(), rig->state.depth,    \
                            __func__, elapsed_ms(&__begin, HAMLIB_ELAPSED_GET))

#define fprintf_flush(file, fmt, ...)         \
        do {                                  \
          fprintf (file, fmt, ##__VA_ARGS__); \
          fflush (file);                      \
        } while (0)

// we need to refer to rc just once as it
// could be a function call
#define RETURNFUNC(rc) do { \
                         int rctmp = rc;                                              \
                         rig_debug (RIG_DEBUG_VERBOSE,                                \
                                    "%.*s%d:%s(%d):%s returning(%ld) %s\n",           \
                                    rig->state.depth, spaces(), rig->state.depth,     \
                                    rig_debug_filename(__FILE__), __LINE__, __func__, \
                                    (long int) (rctmp),                               \
                                    rctmp < 0 ? rigerror2(rctmp) : "");               \
                         --rig->state.depth;                                          \
                         return (rctmp);                                              \
                       } while (0)

#define RETURNFUNC2(rc) do {                                                           \
                          int rctmp = rc;                                              \
                          rig_debug (RIG_DEBUG_VERBOSE,                                \
                                     "%s(%d):%s returning2(%ld) %s\n",                 \
                                     rig_debug_filename(__FILE__), __LINE__, __func__, \
                                     (long int) (rctmp),                               \
                                     rctmp < 0 ? rigerror2(rctmp) : "");               \
                          return (rctmp);                                              \
                        } while (0)

#define CACHE_RESET do {\
    elapsed_ms(&rig->state.cache.time_freqMainA, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_freqMainB, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_freqSubA, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_freqSubB, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_vfo, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_modeMainA, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_modeMainB, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_modeMainC, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_modeSubA, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_modeSubB, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_modeSubC, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_widthMainA, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_widthMainB, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_widthMainC, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_widthSubA, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_widthSubB, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_widthSubC, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_ptt, HAMLIB_ELAPSED_INVALIDATE);\
    elapsed_ms(&rig->state.cache.time_split, HAMLIB_ELAPSED_INVALIDATE);\
  } while (0)


typedef enum settings_value_e
{
    e_CHAR, e_INT, e_LONG, e_FLOAT, e_DOUBLE
} settings_value_t;


extern HAMLIB_EXPORT(int) rig_settings_save(char *setting, void *value, settings_value_t valuet);
extern HAMLIB_EXPORT(int) rig_settings_load(char *setting, void *value, settings_value_t valuet);
extern HAMLIB_EXPORT(int) rig_settings_load_all(char *settings_file);

__END_DECLS

#endif /* _MISC_H */
