/*********************************************************************/
/*                                                                   */
/* ss7osdpn-Solaris2.x.h,v */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech AB                                    */
/*                                                                   */
/* The copyright to the computer  program herein is the property of  */
/* Ericsson Infotech AB. The program may be used and/or              */
/* copied only with the written permission from Ericsson Infotech    */
/* AB or in the accordance with the terms and conditions             */
/* stipulated in the agreement/contract under which the program has  */
/* been supplied.                                                    */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                 (Rev:    )                       */
/* 13/190 55-CAA 201 29 Ux                                           */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: ss7osdpn-Solaris2.x.h,v 1.31 2000/06/30 09:42:55 Exp
 */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/S/AP Dennis Eriksson                                          */
/*                                                                   */
/* Purpose:                                                          */
/*          Operating system dependent stuff for solaris2.x          */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 00-950307  EIN/S/AP Dennis Eriksson                               */
/*                                                                   */
/* 01-960330 EIN/N/D Henrik Berglund                                 */
/*           Added some stuff for new cp (to v1.4 )                  */
/*                                                                   */
/* 02-970610 EIN/N/S Pär Larsson                                     */
/*           Added more new stuff for cp                             */
/*                                                                   */
/* 03-98xxxx EIN/N/B Dan Liljemark                                   */
/*           Added multi-thread support                              */
/*                                                                   */
/* 04-990322  EIN/N/E Dan Liljemark                                  */
/*            Updated after review.                                  */
/*            Inspection master plan: 32/17017-CAA20129              */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/
#ifndef __SS7OSDPN_H__
#define __SS7OSDPN_H__

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif


/*********************************************************************/
/*********************   I  N  C  L  U  D  E  S   ********************/
/*********************************************************************/
#ifdef EINSS7_THREADSAFE
#include <pthread.h>
#include <sys/filio.h> 
#endif /* EINSS7_THREADSAFE */

#include <stropts.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h> 
#include <errno.h>
#include <fcntl.h> 
#include <signal.h>
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h> 
/* #include <sys/unistd.h> TR 1187 rifr 991122 */
#include <unistd.h>     
#include <memory.h>
#include <sys/mman.h> /* memory locking */
#include <sys/wait.h>
#ifdef _POSIX_C_SOURCE
  #if (_POSIX_C_SOURCE >= 199309L)
    #include <time.h>
  #else
    #include <sys/times.h>
    #include <sys/time.h>
  #endif /* (_POSIX_C_SOURCE >= 199309L) */
#else
  #include <sys/time.h>
  #include <sys/times.h>
#endif /* _POSIX_C_SOURCE */


#include <limits.h>

#include <time.h>

#include <sys/un.h>
#include <sys/uio.h>

/* QINXAGO 001128: offsetof() */
#include <stddef.h>

/* for FIONREAD */
#include <sys/filio.h>

#if !defined(BSD) && !defined(SYSV)  
    /* System V Unix */
    #define SYSV 
#endif


#ifdef SYSV
#include <poll.h>
#elif defined BSD
#else
#error NO_OS_SPECIFIED
#endif

#include <netinet/in.h> 
#include <arpa/inet.h>  
#include <netdb.h> 
#include <netinet/tcp.h>
/*********************************************************************/
/*                                                                   */
/*                         T Y P E D E F S                           */
/*                                                                   */
/*********************************************************************/
typedef unsigned char   UCHAR_T;
typedef signed   char   CHAR_T;
typedef unsigned short  USHORT_T;
typedef signed short    SHORT_T;
#ifdef  _LP64
  /* QINXAGO 001123: Force ULONG_T & LONG_T to be 4-byte long */
  typedef uint32_t        ULONG_T;
  typedef int32_t         LONG_T;
  /* QINXAGO 001123: Define these types only for 64-bit environment */
  typedef uint64_t	ULONGLONG_T;
  typedef int64_t	        LONGLONG_T;
#else
  typedef unsigned long   ULONG_T;
  typedef signed long LONG_T;
#endif /* _LP64 */
typedef char        TEXT_T;
typedef int     INT_T;
typedef size_t      SIZE_T;
typedef FILE        FILE_T;
typedef struct sockaddr SOCKADDR_T;
typedef struct sockaddr_un SOCKADDR_UN_T;
typedef struct sockaddr_in SOCKADDR_IN_T;
typedef struct linger LINGER_T;
typedef int             EINSS7SOCKET_T;
typedef EINSS7SOCKET_T  EINSS7CPSOCKET_T;
typedef INT_T           EINSS7SOCKLEN_T;
typedef caddr_t         EINSS7CADDR_T;
typedef struct hostent  EINSS7HOSTENT_T;
typedef fd_set EINSS7CP_FDSET_T;

#ifdef _POSIX_C_SOURCE
  #if (_POSIX_C_SOURCE >= 199309L)
  #else
    typedef struct timeval  EINSS7CPTIMEVAL_T;
    typedef struct timezone EINSS7CPTIMEZONE_T;
  #endif /* (_POSIX_C_SOURCE >= 199309L) */
#else
  typedef struct timeval  EINSS7CPTIMEVAL_T;
  typedef struct timezone EINSS7CPTIMEZONE_T;
#endif /* _POSIX_C_SOURCE */


#if !(defined(__STDC__) && (__STDC__==1))
typedef sigset_t        EINSS7SIGSET_T;
#endif  /* __STDC__ */

#ifdef EINSS7_THREADSAFE
typedef struct timespec EINSS7TIMESPEC_T;
typedef pthread_t       EINSS7THREAD_T;
typedef pthread_mutex_t EINSS7THREADMUTEX_T;
typedef pthread_cond_t  EINSS7THREADCOND_T;
typedef pthread_once_t  EINSS7THREADONCE_T;
typedef pthread_key_t   EINSS7THREADKEY_T;
typedef pthread_attr_t  EINSS7THREADATTR_T;
#endif /* EINSS7_THREADSAFE */


/*********************************************************************/
/*                                                                   */
/*                          M I S C                                  */
/*                                                                   */
/*********************************************************************/

typedef int BOOLEAN_T;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL                  0 
#endif

/* Max value for a LONG_T */
#define EINSS7_LONG_MAX       LONG_MAX

#ifndef __BOOL_T__
#define __BOOL_T__
typedef BOOLEAN_T BOOL_T;
#endif

#define OS_SOCK_ERR  -1

#ifndef SHUT_WR
#define       SHUT_WR         1
#endif /* SHUT_WR */

#ifndef SHUT_RD
#define	SHUT_RD		0
#endif

#ifndef SHUT_RDWR
#define	SHUT_RDWR	2
#endif

/*********************************************************************/
/*                                                                   */
/*            Definitions for file/stream/string  access             */
/*                                                                   */
/*********************************************************************/
#define Strtok_m(s,ct)                          strtok(s,ct)
#define Strlen_m(cs)                            strlen(cs)
#define Strcpy_m(s,ct)                          strcpy(s,ct)
#define Strcat_m(s,ct)                          strcat(s,ct)

#define Fopen_m(file,access)                    fopen(file,access)
#define Fseek_m(fd,offset,whence)               fseek(fd,offset,whence)
#define Ftell_m(fd)                             ftell(fd)
#define Fclose_m(fd)                            fclose(fd)
#define Fgets_m(data,max,fd)                    fgets(data,max,fd)
#define Fwrite_m(buf,size,n,fd)                 fwrite(buf,size,n,fd)
#define Feof_m(fd)                              feof(fd)
#define Ferror_m(fd)                            ferror(fd)
#define Fprintf_m(a)                            fprintf a
#define Fread_m(buf,size,n,fd)                  fread(buf,size,n,fd)

#define Accept_m(s, addr, addrlen)              accept(s, addr, addrlen)
#define Bind_m(s, name, namelen)                bind(s, name, namelen)
#define Close_m(fd)                             close(fd)
#define Connect_m(s, name, namelen)             connect(s, name, namelen)
#define Dup_m(fd)                               dup(fd)
#define Fcntl_m(fd, cmd, arg)                   fcntl(fd, cmd, arg)
#define Getsockopt_m(s,level,oname,ovalue,olen) getsockopt(s,level,oname,ovalue,olen)
#define Gmtime_m(t)                             gmtime(t)
#define Getmsg_m(fd,cptr,dptr,flags)            getmsg(fd,cptr,dptr,flags)
#define Listen_m(s, backlog)                    listen(s, backlog)
#define Localtime_m(t)                          localtime(t) 
#define Localtime_r_m(t,res)                    localtime_r(t,res)             
#define Open_m(file,flags,mode)                 open(file,flags,mode)
#define Opens_m(file,flags)                     open(file,flags)
#define Pipe_m(s)                               pipe(s)
#define Poll_m( fdarray,nfds, timeout)          poll(fdarray, nfds, timeout )
#define Read_m(fd,buf,nByte)                    read(fd,buf,nByte)
#define Recv_m(s, buf, len, flags)              recv(s, buf, len, flags)
#define Recvfrom_m(s,buf,len,flags,addr,adlen)  recvfrom(s, buf, len, flags,addr,adlen)
#define Rename_m(oldName,newName)               rename(oldName,newName)
#define Sched_yield_m()                         sched_yield()
#define Seek_m(fd,offset,whence)                lseek(fd,offset,whence)
#define Select_m(width, rfds, wfds, efds,timeo) select(width, rfds, wfds, efds, timeo)
#define Send_m(s, msg, len, flags)              send(s, msg, len, flags)
#define Sendto_m(s, msg,len,flags,addr,addrlen) sendto(s, msg, len, flags,addr,addrlen)
#define Setsockopt_m(s,level,oname,ovalue,olen) setsockopt(s,level,oname,ovalue,olen)
#define Shutdown_m(s,how)                       shutdown(s,how)
#define Socket_m(domain, type, protocol)        socket(domain, type, protocol)

#define Sigaddset_m(set_p, signo)               sigaddset(set_p, signo)
#define Sigdelset_m(set_p, signo)               sigdelset(set_p, signo)
#define Sigfillset_m(set_p)                     sigfillset(set_p)
#define Sigemptyset_m(set_p)                    sigemptyset(set_p)
#define Sigwait_m(set_p, sig_p)                 sigwait(set_p, sig_p)
#define UnLink_m(path)                          unlink(path)
#define Write_m(fd,buf,nByte)                   write(fd,buf,nByte)
#define Ulimit_m(cmd,newlimit)                  ulimit(cmd,newlimit)
#define InetAddr_m(a)                           inet_addr((a))
#define GetHostByName_m(a)                      gethostbyname((a))
#define GetHostByAddr_m(a, b, c)                gethostbyaddr((a),(b),(c))

/* Macros for OS syslog, do not confuse with commonpart SysLog */
#define EINSS7CpOpenlog_m(ident, logopt, fac)   openlog((ident),(logopt),(fac))
#define EINSS7CpCloselog_m                      closelog

/* The following macros are used for mlock. Page alignment is required by OS. */
/* Align a pointer to be a multiple of a page size (downwards) */
#define EINSS7_AlignDown_m(a)                   ((EINSS7CADDR_T)\
                                                 ((ULONG_T)(a) -\
                                                  (((ULONG_T)(a)) % PAGESIZE)))
/* Align a length to be a multiple of a page size (upwards) */
#define EINSS7_AlignUp_m(a)                     ((EINSS7CADDR_T)\
                                                 ((ULONG_T)((a)+PAGESIZE-1) -\
                                                  ((ULONG_T)((a)+PAGESIZE-1) %\
                                                           PAGESIZE)))
#define EINSS7_MLock_m(addr,len)                 mlock(EINSS7_AlignDown_m((addr)),\
                                                       EINSS7_AlignUp_m((len)))
#define EINSS7_MUnLock_m(addr,len)               munlock(EINSS7_AlignDown_m((addr)),\
                                                       EINSS7_AlignUp_m((len)))

#define EINSS7GetTimeOfDay_m(tv, tz)            gettimeofday((tv),(tz))

#ifdef EINSS7_THREADSAFE
/* Once control */
#define EINSS7ThreadOnce_m(once_ctrl, func)     pthread_once((once_ctrl),\
                                                             (func))

/* Mutex handling */
#define EINSS7ThreadMutexLock_m(mutex_p)        pthread_mutex_lock((mutex_p))
#define EINSS7ThreadMutexUnlock_m(mutex_p)      pthread_mutex_unlock((mutex_p))
#define EINSS7ThreadMutexInit_m(mutex_p, a_p)   pthread_mutex_init((mutex_p),\
                                                                   (a_p))
#define EINSS7ThreadMutexDestroy_m(mutex_p)     pthread_mutex_destroy((mutex_p))

/* EINSS7Thread specific data */
#define EINSS7ThreadKeyCreate_m(key_p,destr_p)  pthread_key_create((key_p),\
                                                                   (destr_p))
#define EINSS7ThreadGetSpecific_m(key)          pthread_getspecific((key))
#define EINSS7ThreadSetSpecific_m(key, val)     pthread_setspecific((key),\
                                                                    (val))


/* Cancellation */
#define EINSS7ThreadSetCancelState_m(state, oldstate) pthread_setcancelstate(\
                                                                    (state),\
                                                                    (oldstate))
#define EINSS7ThreadSetCancelType_m(type,oldtype) pthread_setcanceltype((type),\
                                                                      (oldtype))
#define EINSS7ThreadCleanUpPush_m(func,arg)     pthread_cleanup_push((func),\
                                                                     (arg))
#define EINSS7ThreadCleanUpPop_m(execute)       pthread_cleanup_pop((execute))

/* Condition variables */
#define EINSS7ThreadCondInit_m(cond_p,attr_p)   pthread_cond_init((cond_p),\
                                                                  (attr_p))
#define EINSS7ThreadCondDestroy_m(cond_p)         pthread_cond_destroy(cond_p)
#define EINSS7ThreadCondSignal_m(cond_p)        pthread_cond_signal((cond_p))
#define EINSS7ThreadCondBroadcast_m(cond_p)     pthread_cond_broadcast((cond_p))
#define EINSS7ThreadCondTimedWait_m(cond_p,mut_p,t_p) pthread_cond_timedwait(\
                                                    (cond_p),(mut_p),(t_p))
#define EINSS7ThreadCondWait_m(cond_p,mut_p)    pthread_cond_wait((cond_p),\
                                                                  (mut_p))

/* EINSS7Thread management */
#define EINSS7ThreadCreate_m(tId,attr,func,arg) pthread_create((tId),(attr),\
                                                               (func),(arg))
#define EINSS7ThreadCancel_m(tId)               pthread_cancel((tId))
#define EINSS7ThreadJoin_m(tId,val)             pthread_join((tId),(val))
#define EINSS7ThreadExit_m(val)                 pthread_exit((val))
#define EINSS7ThreadEqual_m(a,b)                pthread_equal((a),(b))
#define EINSS7ThreadSelf_m                      pthread_self

#define EINSS7ThreadSigmask_m(how, set_p,oset_p) pthread_sigmask(how, set_p,oset_p)

#define EINSS7ThreadAttrDestroy_m(attr_p)         pthread_attr_destroy(attr_p)
#define EINSS7ThreadAttrInit_m(attr_p)            pthread_attr_init(attr_p)
#define EINSS7ThreadAttrSetDetachState_m(attr_p,state) \
                                pthread_attr_setdetachstate(attr_p,state)

#define EINSS7CpEnterLogMutex_m(a, b)           EINSS7CpEnterLogMutex((a),(b))
#define EINSS7CpLeaveLogMutex_m(a, b)           EINSS7CpLeaveLogMutex((a),(b))


/* String handling */
#define Strtokr_m(a,b,c)                        strtok_r((a),(b),(c))
#define GetHostByName_r_m(a,b,c,d,e)            gethostbyname_r((a),(b),\
                                                                (c),(d),(e))
#define GetHostByAddr_r_m(a, b, c, d, e, f, g)  gethostbyaddr_r((a),(b),(c),\
                                                            (d),(e),(f),(g))


#else
/* Once control */
#define EINSS7ThreadOnce_m(once_ctrl, func)           

/* Mutex handling */
#define EINSS7ThreadMutexLock_m(mutex_p)              
#define EINSS7ThreadMutexUnlock_m(mutex_p)            
#define EINSS7ThreadMutexInit_m(mutex_p, a_p)              
#define EINSS7ThreadMutexDestroy_m(mutex_p)           

/* EINSS7Thread specific data */
#define EINSS7ThreadKeyCreate_m(key_p,destr_p)        
#define EINSS7ThreadGetSpecific_m(key)                
#define EINSS7ThreadSetSpecific_m(key, val)            

/* Cancellation */
#define EINSS7ThreadSetCancelState_m(state, oldstate)  
#define EINSS7ThreadSetCancelType_m(type, oldtype)    
#define EINSS7ThreadCleanUpPush_m(func,arg)           
#define EINSS7ThreadCleanUpPop_m(execute)             

/* Condition variables */
#define EINSS7ThreadCondInit_m(cond_p,attr_p)         
#define EINSS7ThreadCondDestroy_m(cond_p)
#define EINSS7ThreadCondSignal_m(cond_p)              
#define EINSS7ThreadCondBroadcast_m(cond_p)           
#define EINSS7ThreadCondTimedWait_m(cond_p,mut_p,t_p) 
#define EINSS7ThreadCondWait_m(cond_p,mut_p)          

/* EINSS7Thread management */
#define EINSS7ThreadCreate_m(tId,attr,func,arg)       
#define EINSS7ThreadCancel_m(tId)                     
#define EINSS7ThreadJoin_m(tId,val)                   
#define EINSS7ThreadExit_m(val)                       
#define EINSS7ThreadEqual_m(a,b)                      
#define EINSS7ThreadSelf_m                            
#define EINSS7ThreadSigmask_m(how, set_p,oset_p)

#define EINSS7ThreadAttrDestroy(attr_p)
#define EINSS7ThreadAttrInit_m(attr_p) 
#define EINSS7ThreadAttrSetDetachState_m(attr_p,state) 

#define EINSS7CpEnterLogMutex_m(a, b)              
#define EINSS7CpLeaveLogMutex_m(a, b)              

#endif /* EINSS7_THREADSAFE */

/*********************************************************************/
/*                                                                   */
/*            Definitions for multithread support                    */
/*                                                                   */
/*********************************************************************/
#ifdef EINSS7_THREADSAFE
#define EINSS7THREAD_ONCE_INIT            PTHREAD_ONCE_INIT
#define EINSS7THREAD_MUTEX_INITIALIZER    PTHREAD_MUTEX_INITIALIZER
#define EINSS7THREAD_CANCEL_DISABLE       PTHREAD_CANCEL_DISABLE
#define EINSS7THREAD_COND_INITIALIZER     PTHREAD_COND_INITIALIZER
#define EINSS7THREAD_CANCEL_DEFERRED      PTHREAD_CANCEL_DEFERRED
#define EINSS7THREAD_CANCEL_ASYNCHRONOUS  PTHREAD_CANCEL_ASYNCHRONOUS
/* Values for clean up handler removal */
enum
{
    EINSS7THREAD_JUST_POP = 0x00,
    EINSS7THREAD_EXECUTE_AND_POP
};
#endif /* EINSS7_THREADSAFE */


/*********************************************************************/
/*                                                                   */
/*            Definitions for file flags/mode                        */
/*                                                                   */
/*********************************************************************/
#define O_RDONLY_m                    O_RDONLY
#define O_WRONLY_m                    O_WRONLY
#define O_RDWR_m                        O_RDWR
#define O_SYNC_m                        O_SYNC
#define O_APPEND_m                    O_APPEND
#define O_CREAT_m                       O_CREAT
#define O_TRUNC_m                       O_TRUNC
#define O_NDELAY_m                    O_NDELAY
#define O_NONBLOCK_m            O_NONBLOCK

#define F_SETFL_m               F_SETFL

#define SOCK_STREAM_m               SOCK_STREAM
#define SOCK_DGRAM_m                SOCK_DGRAM
#define SOCK_RAW_m                  SOCK_RAW
#define SOCK_SEQPACKET_m            SOCK_SEQPACKET
#define SOCK_RDM_m                  SOCK_RDM

#define PF_UNIX_m                   PF_UNIX
#define PF_INET_m                   PF_INET
#define PF_IMPLINK_m                PF_IMPLINK

#define AF_UNIX_m                   AF_UNIX
#define AF_INET_m                   AF_INET
#define AF_IMPLINK_m                AF_IMPLINK

#define SOL_SOCKET_m                            (SOL_SOCKET)
#define SO_LINGER_m                             (SO_LINGER)
#define SO_KEEPALIVE_m                          (SO_KEEPALIVE)
#define SO_REUSEADDR_m                          (SO_REUSEADDR)

#define MAX_LINE_LENGTH             256
/*********************************************************************/
/*                                                                   */
/*            Definitions for other services                         */
/*                                                                   */
/*********************************************************************/

/* signals and time related functions*/
#define Signal_m(signr,func)            signal(signr,func)
#define Alarm_m                     alarm(1)
#define Sigaction_m(a,b,c)                      sigaction(a,b,c)
#define EINSS7CpPerror_m                        perror
#define EINSS7CpGetPid_m()                        getpid()

/*********************************************************************/
/*                                                                   */
/*            Prototypes not given by the system header files        */
/*                                                                   */
/*********************************************************************/

/* Predefined castings for predefined macros */
#define CurLine_m               (USHORT_T) __LINE__

#define CurFile_m               (CHAR_T *) __FILE__

/* Convert an integer to chars     */

#define LoByte_m(n)             ((UCHAR_T) ((n) & 0xFF))

#define HiByte_m(n)             ((UCHAR_T) (((n) >> 8) & 0xFF))


#define TriLoByte_m(n)              ((UCHAR_T) ((n) & 0xFF))

#define TriMiByte_m(n)              ((UCHAR_T) (((n) >> 8) & 0xFF))

#define TriHiByte_m(n)              ((UCHAR_T) (((n) >> 16) & 0xFF))

#define LongHostToIFF_m(lw)
#define ShortHostToIFF_m(lw)

/* Following 4 lines added by UlMe 960523 because of compilation problems
   with ANSI ISUP */

#define QLoByte_m(n)                            ((UCHAR_T) ((n) & 0xFF))
#define QMiLoByte_m(n)              ((UCHAR_T) (((n) >> 8) & 0xFF))
#define QMiHiByte_m(n)              ((UCHAR_T) (((n) >> 16) & 0xFF))
#define QHiByte_m(n)                ((UCHAR_T) (((n) >> 24) & 0xFF))
 
/*   to be continued ............ */

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif
#endif
