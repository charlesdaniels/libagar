/*	Public domain	*/

#ifndef _AGAR_CORE_PUBLIC_H_
#define _AGAR_CORE_PUBLIC_H_
#include <agar/core/core_begin.h>

#include <agar/core/core_init.h>

#include <agar/core/string.h>
#ifdef _USE_AGAR_STD
# include <agar/core/snprintf.h>
# include <agar/core/vsnprintf.h>
# include <agar/core/vasprintf.h>
#endif

#include <agar/core/byteswap.h>

#ifdef AG_SERIALIZATION
# include <agar/core/data_source.h>
# include <agar/core/load_integral.h>
# ifdef AG_HAVE_FLOAT
#  include <agar/core/load_real.h>
# endif
# include <agar/core/load_string.h>
#endif
#include <agar/core/load_version.h>

#include <agar/core/version.h>
#include <agar/core/object.h>
#include <agar/core/text.h>
#include <agar/core/list.h>
#include <agar/core/tree.h>
#include <agar/core/tbl.h>
#include <agar/core/config.h>
#include <agar/core/file.h>
#include <agar/core/dir.h>
#include <agar/core/dso.h>
#include <agar/core/db.h>
#include <agar/core/getopt.h>
#include <agar/core/exec.h>
#include <agar/core/user.h>
#include <agar/core/net.h>

#include <agar/core/core_close.h>
#endif
