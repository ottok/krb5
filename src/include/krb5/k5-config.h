/*
 * Copyright 1990,1991,1994,1995 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 * Configuration definition file.
 */


#ifndef KRB5_CONFIG__
#define KRB5_CONFIG__

#ifdef _MSDOS
/* 
 * Machine-type definitions: PC Clone 386 running Microloss Windows
 */

#define KRB5_DBM_COMPAT__                       /* Don't load dbm.h */
#define KRB5_KDB5__                             /* Don't load kdb.h */
#define KRB5_KDB5_DBM__                         /* Don't load kdb_dbm.h */

#define BITS16
#define SIZEOF_INT      2
#define SIZEOF_SHORT    2
#define SIZEOF_LONG     4
#define MAXHOSTNAMELEN  512

#define KRB5_USE_INET
#define MSDOS_FILESYSTEM
#define USE_STRING_H 
#define HAVE_SRAND
#define HAVE_ERRNO
#define HAS_STRDUP
#define NO_USERID
#define NOFCHMOD
#define NOCHMOD

#define HAS_ANSI_CONST                          /* For compiling w/o -Za */
#define HAS_ANSI_VOLATILE
#define HAS_VOID_TYPE
#define KRB5_PROVIDE_PROTOTYPES
#define HAVE_STDARG_H

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif 

/* FIXME, Keith!  I'm not sure how it ever worked.
   Typedeffing u_long and u_char twice will break.  Please eliminate either
   the DOS-only _U_TYPES_DEFINED stuff, or the HAVE_SYS_TYPES_H stuff in
   the DOS section of this file.  The KRB5_SYSTYPES section below was
   previously in ../krb5.h, but I moved it into k5-config.h because
   there was an interaction on Unix needing <sys/types.h> before
   defining the networking include files in k5-config.h.  */
#ifndef _U_TYPES_DEFINED
typedef unsigned long u_long;
typedef unsigned char u_char;
#define _U_LONG_DEFINED
#endif 

#ifndef KRB5_SYSTYPES__
#define KRB5_SYSTYPES__

#ifdef HAVE_SYS_TYPES_H		/* From autoconf.h */
#include <sys/types.h>
#else /* HAVE_SYS_TYPES_H */
typedef unsigned long 	u_long;
typedef unsigned int	u_int;
typedef unsigned short	u_short;
typedef unsigned char	u_char;
#endif /* HAVE_SYS_TYPES_H */
#endif /* KRB5_SYSTYPES__ */

#ifndef INTERFACE
#define INTERFACE   __far __export __pascal
#define INTERFACE_C __far __export __cdecl
#endif

/*
 * The following defines are needed to make <windows.h> work
 * in stdc mode (/Za flag). Winsock.h needs <windows.h>.
 */
#define FAR     _far
#define NEAR    _near
#define _far    __far
#define _near   __near
#define _pascal __pascal
#define _cdecl  __cdecl
#define _huge   __huge

#ifdef NEED_WINDOWS
#include <windows.h>
#endif

#ifdef NEED_SOCKETS
#include <winsock.h>
#endif

#ifdef NEED_LOWLEVEL_IO
/* Ugly. Microsoft, in stdc mode, doesn't support the low-level i/o
 * routines directly. Rather, they only export the _<function> version.
 * The following defines works around this problem. 
 */
#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#define O_RDONLY        _O_RDONLY
#define O_WRONLY        _O_WRONLY
#define O_RDWR          _O_RDWR
#define O_APPEND        _O_APPEND
#define O_CREAT         _O_CREAT
#define O_TRUNC         _O_TRUNC
#define O_EXCL          _O_EXCL
#define O_TEXT          _O_TEXT
#define O_BINARY        _O_BINARY
#define O_NOINHERIT     _O_NOINHERIT
#define stat            _stat
#define unlink          _unlink
#define lseek           _lseek
#define write           _write
#define open            _open
#define close           _close
#define read            _read
#define fstat           _fstat
#define mktemp          _mktemp

#define getpid          _getpid
#endif

#ifdef NEED_SYSERROR
/* Only needed by util/et/error_message.c but let's keep the source clean */
#define sys_nerr        _sys_nerr
#define sys_errlist     _sys_errlist
#endif

/* XXX these should be parameterized soon... */
#define PROVIDE_DES_CBC_MD5
#define PROVIDE_DES_CBC_CRC
#define PROVIDE_RAW_DES_CBC
#define PROVIDE_CRC32
#define PROVIDE_DES_CBC_CKSUM
#define PROVIDE_RSA_MD4
#define PROVIDE_RSA_MD5
#define DEFAULT_PWD_STRING1 "Enter password:"
#define DEFAULT_PWD_STRING2 "Re-enter password for verification:"

/* Functions with slightly different names on the PC
*/                   
#define strcasecmp   _stricmp
#define strdup       _strdup
#define off_t        _off_t

#else 		/* Rest of include file is for non-Microloss-Windows */

#ifndef KRB5_AUTOCONF__
#define KRB5_AUTOCONF__
#include "autoconf.h"
#endif

#ifndef KRB5_SYSTYPES__
#define KRB5_SYSTYPES__

#ifdef HAVE_SYS_TYPES_H		/* From autoconf.h */
#include <sys/types.h>
#else /* HAVE_SYS_TYPES_H */
typedef unsigned long 	u_long;
typedef unsigned int	u_int;
typedef unsigned short	u_short;
typedef unsigned char	u_char;
#endif /* HAVE_SYS_TYPES_H */
#endif /* KRB5_SYSTYPES__ */

#ifdef SYSV
/* Change srandom and random to use rand and srand */
/* Taken from the Sandia changes.  XXX  We should really just include */
/* srandom and random into Kerberos release, since rand() is a really */
/* bad random number generator.... [tytso:19920616.2231EDT] */
#define random() rand()
#define srandom(a) srand(a)
#ifndef unicos61
#define utimes(a,b) utime(a,b)
#endif  /* unicos61 */
#endif /* SYSV */

/* XXX these should be parameterized soon... */
#define PROVIDE_DES_CBC_MD5
#define PROVIDE_DES_CBC_CRC
#define PROVIDE_RAW_DES_CBC
#define PROVIDE_CRC32
#define PROVIDE_DES_CBC_CKSUM
#define PROVIDE_RSA_MD4
#define PROVIDE_RSA_MD5

#define DEFAULT_PWD_STRING1 "Enter password:"
#define DEFAULT_PWD_STRING2 "Re-enter password for verification:"

#define	KRB5_KDB_MAX_LIFE	(60*60*24) /* one day */
#define	KRB5_KDB_MAX_RLIFE	(60*60*24*7) /* one week */
#define	KRB5_KDB_EXPIRATION	2145830400 /* Thu Jan  1 00:00:00 2038 UTC */

/*
 * For paranoid DOE types that don't want to give helpful error
 * messages to the client....er, attacker
 */
#undef KRBCONF_VAGUE_ERRORS

/*
 * Define this if you want the KDC to modify the Kerberos database;
 * this allows the last request information to be updated, as well as
 * the failure count information.
 * 
 * Note that this doesn't work if you're using slave servers!!!  It
 * also causes the database to be modified (and thus need to be
 * locked) frequently.
 */
#undef KRBCONF_KDC_MODIFIES_KDB
    
/* 
 * Windows requires a different api interface to each function. Here
 * just define it as NULL.
 */
#define INTERFACE
#define INTERFACE_C
#define FAR
#define NEAR

#ifndef HAS_LABS
#define labs(x) abs(x)
#endif


#ifdef NEED_SOCKETS
/* If this source file requires it, define struct sockaddr_in
   (and possibly other things related to network I/O).  */

#ifdef HAVE_MACSOCK_H		/* Sockets stuff differs on Mac */
#include "macsock.h"		/* Macintosh sockets emulation library */

#else  /* HAVE_MACSOCK_H */	/* Sockets stuff for Unix machines */

#include <netinet/in.h>		/* For struct sockaddr_in and in_addr */
#include <arpa/inet.h>		/* For inet_ntoa */
#include <netdb.h>		/* For struct hostent, gethostbyname, etc */
#include <sys/param.h>		/* For MAXHOSTNAMELEN */
#include <sys/socket.h>		/* For SOCK_*, AF_*, etc */
#include <sys/time.h>		/* For struct timeval */
#include <net/if.h>		/* For struct ifconf, for localaddr.c */

#endif /* HAVE_MACSOCK_H */
#endif /* NEED_SOCKETS */

#endif /* _MSDOS */

#endif /* KRB5_CONFIG__ */
