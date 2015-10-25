
#define HAVE_LIMITS_H
#define HAVE_STRING_H
#define HAVE_STDLIB_H
#define HAVE_TIME_H
#define HAVE_SYS_DIRECT_H

#define NEED_DECLARATION_GETENV
#define NEED_DECLARATION_FREE
#define NEED_DECLARATION_ATOF
#define NEED_DECLARATION_ATOL
#define NEED_DECLARATION_STRSTR
#define HAVE_STRERROR
#define NEED_DECLARATION_STRERROR
#define MKDIR_TAKES_ONE_ARG





#define POSIX

//#define HAVE_parse_number
//#define HAVE_translate_name
//#define HAVE_make_assertion
#define  HAVE_NEW_HRD_CFG





/* Look for the include files in the system-defined places.  */

#undef GPLUSPLUS_INCLUDE_DIR
#define GPLUSPLUS_INCLUDE_DIR "/usr/include/g++"

#undef GCC_INCLUDE_DIR
#define GCC_INCLUDE_DIR "/usr/include"

#undef INCLUDE_DEFAULTS
#define INCLUDE_DEFAULTS			\
  {						\
    { GPLUSPLUS_INCLUDE_DIR, "G++", 1, 1 },	\
    { GCC_INCLUDE_DIR, "GCC", 0, 0 },		\
    { 0, 0, 0, 0 }				\
  }

/* Under NetBSD, the normal location of the compiler back ends is the
   /usr/libexec directory.  */

#undef STANDARD_EXEC_PREFIX
#define STANDARD_EXEC_PREFIX		"/usr/libexec/"

/* Under NetBSD, the normal location of the various *crt*.o files is the
   /usr/lib directory.  */

#undef STANDARD_STARTFILE_PREFIX
#define STANDARD_STARTFILE_PREFIX	"/usr/lib/"


#define FATAL_EXIT_CODE  1

#define SUCCESS_EXIT_CODE  0

#ifndef NULL_PTR
#define NULL_PTR  0
#endif

#define  BITS_PER_UNIT      8
#define  BITS_PER_WORD      32
#define  HOST_BITS_PER_LONG 32
#define  HOST_BITS_PER_INT  32



/* Find HOST_WIDEST_INT and set its bit size, type and print macros.
   It will be the largest integer mode supported by the host which may
   (or may not) be larger than HOST_WIDE_INT.  This must appear after
   <limits.h> since we only use `long long' if its bigger than a
   `long' and also if it is supported by macros in limits.h.  For old
   hosts which don't have a limits.h (and thus won't include it in
   stage2 cause we don't rerun configure) we assume gcc supports long
   long.)  Note, you won't get these defined if you don't include
   {ht}config.h before this file to set the HOST_BITS_PER_* macros. */
#ifdef __GNUC__
#ifndef HOST_WIDEST_INT
# if defined (HOST_BITS_PER_LONG) && defined (HOST_BITS_PER_LONGLONG)
#  if (HOST_BITS_PER_LONGLONG > HOST_BITS_PER_LONG) && (defined (LONG_LONG_MAX) || defined (LONGLONG_MAX) || defined (LLONG_MAX) || defined (__GNUC__))
#   define HOST_BITS_PER_WIDEST_INT HOST_BITS_PER_LONGLONG
#   define HOST_WIDEST_INT long long
#   define HOST_WIDEST_INT_PRINT_DEC "%lld"
#   define HOST_WIDEST_INT_PRINT_UNSIGNED "%llu"
#   define HOST_WIDEST_INT_PRINT_HEX "0x%llx"
#  else
#   define HOST_BITS_PER_WIDEST_INT HOST_BITS_PER_LONG
#   define HOST_WIDEST_INT long
#   define HOST_WIDEST_INT_PRINT_DEC "%ld"
#   define HOST_WIDEST_INT_PRINT_UNSIGNED "%lu"
#   define HOST_WIDEST_INT_PRINT_HEX "0x%lx"
#  endif /*(long long>long) && (LONG_LONG_MAX||LONGLONG_MAX||LLONG_MAX||GNUC)*/
# endif /* defined(HOST_BITS_PER_LONG) && defined(HOST_BITS_PER_LONGLONG) */
#endif /* ! HOST_WIDEST_INT */
#else

#define HOST_BITS_PER_WIDEST_INT HOST_BITS_PER_LONG
#define HOST_WIDEST_INT long
#define HOST_WIDEST_INT_PRINT_DEC "%ld"
#define HOST_WIDEST_INT_PRINT_UNSIGNED "%lu"
#define HOST_WIDEST_INT_PRINT_HEX "0x%lx"
#endif



#define  _INC_VARARGS 


#define ASM_OPEN_PAREN ""
#define ASM_CLOSE_PAREN ""

/* Define results of standard character escape sequences.  */
#define TARGET_BELL 007
#define TARGET_BS 010
#define TARGET_TAB 011
#define TARGET_NEWLINE 012
#define TARGET_VT 013
#define TARGET_FF 014
#define TARGET_CR 015

#define DIR_SEPARATOR ']'

#define PREFIX "GNU_ROOT:"


//#define __STDC__ 1
