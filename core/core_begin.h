/*	Public domain	*/

#include <agar/core/options.h>
#include <agar/core/error.h>
#include <agar/core/threads.h>
#include <agar/core/types.h>
#include <agar/core/attributes.h>
#include <agar/core/limits.h>
#include <agar/core/queue.h>
#include <agar/core/cpuinfo.h>

#if !defined(AG_BIG_ENDIAN) && !defined(AG_LITTLE_ENDIAN)
# define AG_BIG_ENDIAN 4321
# define AG_LITTLE_ENDIAN 1234
# include <agar/config/_mk_big_endian.h>
# include <agar/config/_mk_little_endian.h>
# if defined(_MK_BIG_ENDIAN)
#  define AG_BYTEORDER AG_BIG_ENDIAN
# elif defined(_MK_LITTLE_ENDIAN)
#  define AG_BYTEORDER AG_LITTLE_ENDIAN
# else
#  error "Byte order is unknown"
# endif
# undef _MK_BIG_ENDIAN
# undef _MK_LITTLE_ENDIAN
#endif /* !AG_BIG_ENDIAN && !AG_LITTLE_ENDIAN */
