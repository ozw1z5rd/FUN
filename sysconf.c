/* Zjistení konfiguracních parametru systému */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void confstr_val(const char *str,int name)
{
  int sz;
  char *buf;
  if((sz=confstr(name,NULL,0))>0) {
    if(!(buf=malloc(sz))) {
      perror("malloc()");
      return;
    }
    confstr(name,buf,sz);
    printf("%s = \"%s\"\n",str,buf);
    free(buf);
  }
}

int main()
{
  printf("\n*** sysconf() ***\n\n");
#ifdef _SC_ARG_MAX
  printf("ARG_MAX = %ld\n",sysconf(_SC_ARG_MAX));
#endif
#ifdef _SC_BC_BASE_MAX
  printf("BC_BASE_MAX = %ld\n",sysconf(_SC_BC_BASE_MAX));
#endif
#ifdef _SC_BC_DIM_MAX
  printf("BC_DIM_MAX = %ld\n",sysconf(_SC_BC_DIM_MAX));
#endif
#ifdef _SC_BC_SCALE_MAX
  printf("BC_SCALE_MAX = %ld\n",sysconf(_SC_BC_SCALE_MAX));
#endif
#ifdef _SC_BC_STRING_MAX
  printf("BC_STRING_MAX = %ld\n",sysconf(_SC_BC_STRING_MAX));
#endif
#ifdef _SC_CHILD_MAX
  printf("CHILD_MAX = %ld\n",sysconf(_SC_CHILD_MAX));
#endif
#ifdef _SC_CLK_TCK
  printf("CLK_TCK = %ld\n",sysconf(_SC_CLK_TCK));
#endif
#ifdef _SC_COLL_WEIGHTS_MAX
  printf("COLL_WEIGHTS_MAX = %ld\n",sysconf(_SC_COLL_WEIGHTS_MAX));
#endif
#ifdef _SC_EXPR_NEST_MAX
  printf("EXPR_NEST_MAX = %ld\n",sysconf(_SC_EXPR_NEST_MAX));
#endif
#ifdef _SC_LINE_MAX
  printf("LINE_MAX = %ld\n",sysconf(_SC_LINE_MAX));
#endif
#ifdef _SC_NGROUPS_MAX
  printf("NGROUPS_MAX = %ld\n",sysconf(_SC_NGROUPS_MAX));
#endif
#ifdef _SC_OPEN_MAX
  printf("OPEN_MAX = %ld\n",sysconf(_SC_OPEN_MAX));
#endif
#ifdef _SC_PASS_MAX
  printf("PASS_MAX = %ld\n",sysconf(_SC_PASS_MAX));
#endif
#ifdef _SC_2_C_BIND
  printf("_POSIX2_C_BIND = %ld\n",sysconf(_SC_2_C_BIND));
#endif
#ifdef _SC_2_C_DEV
  printf("_POSIX2_C_DEV = %ld\n",sysconf(_SC_2_C_DEV));
#endif
#ifdef _SC_2_C_VERSION
  printf("_POSIX2_C_VERSION = %ld\n",sysconf(_SC_2_C_VERSION));
#endif
#ifdef _SC_2_CHAR_TERM
  printf("_POSIX2_CHAR_TERM = %ld\n",sysconf(_SC_2_CHAR_TERM));
#endif
#ifdef _SC_2_FORT_DEV
  printf("_POSIX2_FORT_DEV = %ld\n",sysconf(_SC_2_FORT_DEV));
#endif
#ifdef _SC_2_FORT_RUN
  printf("_POSIX2_FORT_RUN = %ld\n",sysconf(_SC_2_FORT_RUN));
#endif
#ifdef _SC_2_LOCALEDEF
  printf("_POSIX2_LOCALEDEF = %ld\n",sysconf(_SC_2_LOCALEDEF));
#endif
#ifdef _SC_2_SW_DEV
  printf("_POSIX2_SW_DEV = %ld\n",sysconf(_SC_2_SW_DEV));
#endif
#ifdef _SC_2_UPE
  printf("_POSIX2_UPE = %ld\n",sysconf(_SC_2_UPE));
#endif
#ifdef _SC_2_VERSION
  printf("_POSIX2_VERSION = %ld\n",sysconf(_SC_2_VERSION));
#endif
#ifdef _SC_JOB_CONTROL
  printf("_POSIX_JOB_CONTROL = %ld\n",sysconf(_SC_JOB_CONTROL));
#endif
#ifdef _SC_SAVED_IDS
  printf("_POSIX_SAVED_IDS = %ld\n",sysconf(_SC_SAVED_IDS));
#endif
#ifdef _SC_VERSION
  printf("_POSIX_VERSION = %ld\n",sysconf(_SC_VERSION));
#endif
#ifdef _SC_RE_DUP_MAX
  printf("RE_DUP_MAX = %ld\n",sysconf(_SC_RE_DUP_MAX));
#endif
#ifdef _SC_STREAM_MAX
  printf("STREAM_MAX = %ld\n",sysconf(_SC_STREAM_MAX));
#endif
#ifdef _SC_TZNAME_MAX
  printf("TZNAME_MAX = %ld\n",sysconf(_SC_TZNAME_MAX));
#endif
#ifdef _SC_XOPEN_CRYPT
  printf("_XOPEN_CRYPT = %ld\n",sysconf(_SC_XOPEN_CRYPT));
#endif
#ifdef _SC_XOPEN_ENH_I18N
  printf("_XOPEN_ENH_I18N = %ld\n",sysconf(_SC_XOPEN_ENH_I18N));
#endif
#ifdef _SC_XOPEN_SHM
  printf("_XOPEN_SHM = %ld\n",sysconf(_SC_XOPEN_SHM));
#endif
#ifdef _SC_XOPEN_VERSION
  printf("_XOPEN_VERSION = %ld\n",sysconf(_SC_XOPEN_VERSION));
#endif
#ifdef _SC_XOPEN_XCU_VERSION
  printf("_XOPEN_XCU_VERSION = %ld\n",sysconf(_SC_XOPEN_XCU_VERSION));
#endif
#ifdef _SC_XOPEN_REALTIME
  printf("_XOPEN_REALTIME = %ld\n",sysconf(_SC_XOPEN_REALTIME));
#endif
#ifdef _SC_XOPEN_REALTIME_THREADS
  printf("_XOPEN_REALTIME_THREADS = %ld\n",sysconf(_SC_XOPEN_REALTIME_THREADS));
#endif
#ifdef _SC_XOPEN_LEGACY
  printf("_XOPEN_LEGACY = %ld\n",sysconf(_SC_XOPEN_LEGACY));
#endif
#ifdef _SC_ATEXIT_MAX
  printf("ATEXIT_MAX = %ld\n",sysconf(_SC_ATEXIT_MAX));
#endif
#ifdef _SC_IOV_MAX
  printf("IOV_MAX = %ld\n",sysconf(_SC_IOV_MAX));
#endif
#ifdef _SC_T_IOV_MAX /* Linux */
  printf("T_IOV_MAX = %ld\n",sysconf(_SC_T_IOV_MAX));
#endif
#ifdef _SC_PAGESIZE
  printf("PAGESIZE = %ld\n",sysconf(_SC_PAGESIZE));
#endif
#ifdef _SC_PAGE_SIZE
  printf("PAGE_SIZE = %ld\n",sysconf(_SC_PAGE_SIZE));
#endif
#ifdef _SC_XOPEN_UNIX
  printf("_XOPEN_UNIX = %ld\n",sysconf(_SC_XOPEN_UNIX));
#endif
#ifdef _SC_XBS5_ILP32_OFF32
  printf("_XBS5_ILP32_OFF32 = %ld\n",sysconf(_SC_XBS5_ILP32_OFF32));
#endif
#ifdef _SC_XBS5_ILP32_OFFBIG
  printf("_XBS5_ILP32_OFFBIG = %ld\n",sysconf(_SC_XBS5_ILP32_OFFBIG));
#endif
#ifdef _SC_XBS5_LP64_OFF64
  printf("_XBS5_LP64_OFF64 = %ld\n",sysconf(_SC_XBS5_LP64_OFF64));
#endif
#ifdef _SC_XBS5_LPBIG_OFFBIG
  printf("_XBS5_LPBIG_OFFBIG = %ld\n",sysconf(_SC_XBS5_LPBIG_OFFBIG));
#endif
#ifdef _SC_AIO_LISTIO_MAX
  printf("AIO_LISTIO_MAX = %ld\n",sysconf(_SC_AIO_LISTIO_MAX));
#endif
#ifdef _SC_AIO_MAX
  printf("AIO_MAX = %ld\n",sysconf(_SC_AIO_MAX));
#endif
#ifdef _SC_AIO_PRIO_DELTA_MAX
  printf("AIO_PRIO_DELTA_MAX = %ld\n",sysconf(_SC_AIO_PRIO_DELTA_MAX));
#endif
#ifdef _SC_DELAYTIMER_MAX
  printf("DELAYTIMER_MAX = %ld\n",sysconf(_SC_DELAYTIMER_MAX));
#endif
#ifdef _SC_MQ_OPEN_MAX
  printf("MQ_OPEN_MAX = %ld\n",sysconf(_SC_MQ_OPEN_MAX));
#endif
#ifdef _SC_MQ_PRIO_MAX
  printf("MQ_PRIO_MAX = %ld\n",sysconf(_SC_MQ_PRIO_MAX));
#endif
#ifdef _SC_RTSIG_MAX
  printf("RTSIG_MAX = %ld\n",sysconf(_SC_RTSIG_MAX));
#endif
#ifdef _SC_SEM_NSEMS_MAX
  printf("SEM_NSEMS_MAX = %ld\n",sysconf(_SC_SEM_NSEMS_MAX));
#endif
#ifdef _SC_SEM_VALUE_MAX
  printf("SEM_VALUE_MAX = %ld\n",sysconf(_SC_SEM_VALUE_MAX));
#endif
#ifdef _SC_SIGQUEUE_MAX
  printf("SIGQUEUE_MAX = %ld\n",sysconf(_SC_SIGQUEUE_MAX));
#endif
#ifdef _SC_TIMER_MAX
  printf("TIMER_MAX = %ld\n",sysconf(_SC_TIMER_MAX));
#endif
#ifdef _SC_ASYNCHRONOUS_IO
  printf("_POSIX_ASYNCHRONOUS_IO = %ld\n",sysconf(_SC_ASYNCHRONOUS_IO));
#endif
#ifdef _SC_FSYNC
  printf("_POSIX_FSYNC = %ld\n",sysconf(_SC_FSYNC));
#endif
#ifdef _SC_MAPPED_FILES
  printf("_POSIX_MAPPED_FILES = %ld\n",sysconf(_SC_MAPPED_FILES));
#endif
#ifdef _SC_MEMLOCK
  printf("_POSIX_MEMLOCK = %ld\n",sysconf(_SC_MEMLOCK));
#endif
#ifdef _SC_MEMLOCK_RANGE
  printf("_POSIX_MEMLOCK_RANGE = %ld\n",sysconf(_SC_MEMLOCK_RANGE));
#endif
#ifdef _SC_MEMORY_PROTECTION
  printf("_POSIX_MEMORY_PROTECTION = %ld\n",sysconf(_SC_MEMORY_PROTECTION));
#endif
#ifdef _SC_MESSAGE_PASSING
  printf("_POSIX_MESSAGE_PASSING = %ld\n",sysconf(_SC_MESSAGE_PASSING));
#endif
#ifdef _SC_PRIORITIZED_IO
  printf("_POSIX_PRIORITIZED_IO = %ld\n",sysconf(_SC_PRIORITIZED_IO));
#endif
#ifdef _SC_PRIORITY_SCHEDULING
  printf("_POSIX_PRIORITY_SCHEDULING = %ld\n",sysconf(_SC_PRIORITY_SCHEDULING));
#endif
#ifdef _SC_REALTIME_SIGNALS
  printf("_POSIX_REALTIME_SIGNALS = %ld\n",sysconf(_SC_REALTIME_SIGNALS));
#endif
#ifdef _SC_SEMAPHORES
  printf("_POSIX_SEMAPHORES = %ld\n",sysconf(_SC_SEMAPHORES));
#endif
#ifdef _SC_SHARED_MEMORY_OBJECTS
  printf("_POSIX_SHARED_MEMORY_OBJECTS = %ld\n",sysconf(_SC_SHARED_MEMORY_OBJECTS));
#endif
#ifdef _SC_SYNCHRONIZED_IO
  printf("_POSIX_SYNCHRONIZED_IO = %ld\n",sysconf(_SC_SYNCHRONIZED_IO));
#endif
#ifdef _SC_TIMERS
  printf("_POSIX_TIMERS = %ld\n",sysconf(_SC_TIMERS));
#endif
#ifdef _SC_GETGR_R_SIZE_MAX
  printf("Maximum size of getgrgid_r() and getgrnam_r() data buffers  = %ld\n",sysconf(_SC_GETGR_R_SIZE_MAX));
#endif
#ifdef _SC_GETPW_R_SIZE_MAX
  printf("Maximum size of getpwuid_r() and getpwnam_r() data buffers  = %ld\n",sysconf(_SC_GETPW_R_SIZE_MAX));
#endif
#ifdef _SC_LOGIN_NAME_MAX
  printf("LOGIN_NAME_MAX = %ld\n",sysconf(_SC_LOGIN_NAME_MAX));
#endif
#ifdef _SC_THREAD_DESTRUCTOR_ITERATIONS
  printf("PTHREAD_DESTRUCTOR_ITERATIONS = %ld\n",sysconf(_SC_THREAD_DESTRUCTOR_ITERATIONS));
#endif
#ifdef _SC_THREAD_KEYS_MAX
  printf("PTHREAD_KEYS_MAX = %ld\n",sysconf(_SC_THREAD_KEYS_MAX));
#endif
#ifdef _SC_THREAD_STACK_MIN
  printf("PTHREAD_STACK_MIN = %ld\n",sysconf(_SC_THREAD_STACK_MIN));
#endif
#ifdef _SC_THREAD_THREADS_MAX
  printf("PTHREAD_THREADS_MAX = %ld\n",sysconf(_SC_THREAD_THREADS_MAX));
#endif
#ifdef _SC_TTY_NAME_MAX
  printf("TTY_NAME_MAX = %ld\n",sysconf(_SC_TTY_NAME_MAX));
#endif
#ifdef _SC_THREADS
  printf("_POSIX_THREADS = %ld\n",sysconf(_SC_THREADS));
#endif
#ifdef _SC_THREAD_ATTR_STACKADDR
  printf("_POSIX_THREAD_ATTR_STACKADDR = %ld\n",sysconf(_SC_THREAD_ATTR_STACKADDR));
#endif
#ifdef _SC_THREAD_ATTR_STACKSIZE
  printf("_POSIX_THREAD_ATTR_STACKSIZE = %ld\n",sysconf(_SC_THREAD_ATTR_STACKSIZE));
#endif
#ifdef _SC_THREAD_PRIORITY_SCHEDULING
  printf("_POSIX_THREAD_PRIORITY_SCHEDULING = %ld\n",sysconf(_SC_THREAD_PRIORITY_SCHEDULING));
#endif
#ifdef _SC_THREAD_PRIO_INHERIT
  printf("_POSIX_THREAD_PRIO_INHERIT = %ld\n",sysconf(_SC_THREAD_PRIO_INHERIT));
#endif
#ifdef _SC_THREAD_PRIO_PROTECT
  printf("_POSIX_THREAD_PRIO_PROTECT = %ld\n",sysconf(_SC_THREAD_PRIO_PROTECT));
#endif
#ifdef _SC_THREAD_PROCESS_SHARED
  printf("_POSIX_THREAD_PROCESS_SHARED = %ld\n",sysconf(_SC_THREAD_PROCESS_SHARED));
#endif
#ifdef _SC_THREAD_SAFE_FUNCTIONS
  printf("_POSIX_THREAD_SAFE_FUNCTIONS = %ld\n",sysconf(_SC_THREAD_SAFE_FUNCTIONS));
#endif
  printf("\n*** confstr() ***\n\n");
#ifdef _CS_PATH
  confstr_val("_CS_PATH",_CS_PATH);
#endif
#ifdef _CS_XBS5_ILP32_OFF32_CFLAGS
  confstr_val("_CS_XBS5_ILP32_OFF32_CFLAGS",_CS_XBS5_ILP32_OFF32_CFLAGS);
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LDFLAGS
  confstr_val("_CS_XBS5_ILP32_OFF32_LDFLAGS",_CS_XBS5_ILP32_OFF32_LDFLAGS);
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LIBS
  confstr_val("_CS_XBS5_ILP32_OFF32_LIBS",_CS_XBS5_ILP32_OFF32_LIBS);
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LINTFLAGS
  confstr_val("_CS_XBS5_ILP32_OFF32_LINTFLAGS",_CS_XBS5_ILP32_OFF32_LINTFLAGS);
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_CFLAGS
  confstr_val("_CS_XBS5_ILP32_OFFBIG_CFLAGS",_CS_XBS5_ILP32_OFFBIG_CFLAGS);
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LDFLAGS
  confstr_val("_CS_XBS5_ILP32_OFFBIG_LDFLAGS",_CS_XBS5_ILP32_OFFBIG_LDFLAGS);
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LIBS
  confstr_val("_CS_XBS5_ILP32_OFFBIG_LIBS",_CS_XBS5_ILP32_OFFBIG_LIBS);
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LINTFLAGS
  confstr_val("_CS_XBS5_ILP32_OFFBIG_LINTFLAGS",_CS_XBS5_ILP32_OFFBIG_LINTFLAGS);
#endif
#ifdef _CS_XBS5_LP64_OFF64_CFLAGS
  confstr_val("_CS_XBS5_LP64_OFF64_CFLAGS",_CS_XBS5_LP64_OFF64_CFLAGS);
#endif
#ifdef _CS_XBS5_LP64_OFF64_LDFLAGS
  confstr_val("_CS_XBS5_LP64_OFF64_LDFLAGS",_CS_XBS5_LP64_OFF64_LDFLAGS);
#endif
#ifdef _CS_XBS5_LP64_OFF64_LIBS
  confstr_val("_CS_XBS5_LP64_OFF64_LIBS",_CS_XBS5_LP64_OFF64_LIBS);
#endif
#ifdef _CS_XBS5_LP64_OFF64_LINTFLAGS
  confstr_val("_CS_XBS5_LP64_OFF64_LINTFLAGS",_CS_XBS5_LP64_OFF64_LINTFLAGS);
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_CFLAGS
  confstr_val("_CS_XBS5_LPBIG_OFFBIG_CFLAGS",_CS_XBS5_LPBIG_OFFBIG_CFLAGS);
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LDFLAGS
  confstr_val("_CS_XBS5_LPBIG_OFFBIG_LDFLAGS",_CS_XBS5_LPBIG_OFFBIG_LDFLAGS);
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LIBS
  confstr_val("_CS_XBS5_LPBIG_OFFBIG_LIBS",_CS_XBS5_LPBIG_OFFBIG_LIBS);
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS
  confstr_val("_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS",_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS);
#endif
  printf("\n*** pathconf() ***\n\n");
#ifdef _PC_FILESIZEBITS
  printf("FILESIZEBITS = %ld\n",pathconf("/",_PC_FILESIZEBITS));
#endif
#ifdef _PC_LINK_MAX
  printf("LINK_MAX = %ld\n",pathconf("/",_PC_LINK_MAX));
#endif
#ifdef _PC_MAX_CANON
  printf("MAX_CANON = %ld\n",pathconf("/",_PC_MAX_CANON));
#endif
#ifdef _PC_MAX_INPUT
  printf("MAX_INPUT = %ld\n",pathconf("/",_PC_MAX_INPUT));
#endif
#ifdef _PC_NAME_MAX
  printf("NAME_MAX = %ld\n",pathconf("/",_PC_NAME_MAX));
#endif
#ifdef _PC_PATH_MAX
  printf("PATH_MAX = %ld\n",pathconf("/",_PC_PATH_MAX));
#endif
#ifdef _PC_PIPE_BUF
  printf("PIPE_BUF = %ld\n",pathconf("/",_PC_PIPE_BUF));
#endif
#ifdef _PC_CHOWN_RESTRICTED
  printf("_POSIX_CHOWN_RESTRICTED = %ld\n",pathconf("/",_PC_CHOWN_RESTRICTED));
#endif
#ifdef _PC_NO_TRUNC
  printf("_POSIX_NO_TRUNC = %ld\n",pathconf("/",_PC_NO_TRUNC));
#endif
#ifdef _PC_VDISABLE
  printf("_POSIX_VDISABLE = %ld\n",pathconf("/",_PC_VDISABLE));
#endif
#ifdef _PC_ASYNC_IO
  printf("_POSIX_ASYNC_IO = %ld\n",pathconf("/",_PC_ASYNC_IO));
#endif
#ifdef _PC_PRIO_IO
  printf("_POSIX_PRIO_IO = %ld\n",pathconf("/",_PC_PRIO_IO));
#endif
#ifdef _PC_SYNC_IO
  printf("_POSIX_SYNC_IO = %ld\n",pathconf("/",_PC_SYNC_IO));
#endif
  exit(0);
}
