# Kompilátor C
CC=cc

# Volby pro kompilátor
#CFLAGS=-g -W -Wimplicit -Wunused
CFLAGS=-g -Wall

# Volby pro linker
LDFLAGS=

# Knihovny, které se budou linkovat k programum
# libgen.so potrebuje program args na IRIXu
#LIBS=-lgen
LIBS=

# Jména cílových programu
TARGETS=hello args env change_env getopt getopt_long \
        copy fcopy lseek mmap mmap_reverse dir dir_name stat_link access \
	proc_info sysconf \
	fork_wait fork_cycl fork_exec \
	dlopen dlopen_fact.so dlopen_ops.so \
	pipe_p2c pipe_c2p pipe_sh pipe_bash pipe_capacity pipe_select \
	sigaction sig_alrm \
	thread_run thread_sync thread_signal thread_syscall \
	shared_var \
	lock_nolock lock_lockfile lock_flock \
	msg_send msg_recv shm_sem_send shm_sem_recv \
	net_server net_client \
	curses pseudoterm readline \
	multilock_sem multilock_cond \
	memtest memtest2 seznamy \
	udp_server udp_client tcp udp

# Jména zdrojových souboru (pouzívají se v make depend)
SRCS=hello.c args.c env.c change_env.c getopt.c getopt_long.c \
     copy.c fcopy.c lseek.c mmap.c mmap_reverse.c dir.c dir_name.c \
     stat_link.c access.c \
     proc_info.c sysconf.c \
     fork_wait.c fork_cycl.c fork_exec.c \
     dlopen.c dlopen_fact.c dlopen_ops.c \
     pipe_p2c.c pipe_c2p.c pipe_sh.c pipe_bash.c pipe_capacity.c \
     pipe_select.c \
     sigaction.c sig_alrm.c \
     thread_run.c thread_sync.c thread_signal.c thread_syscall.c \
     shared_var.c \
     lock.c \
     msg_send.c msg_recv.c shm_sem_send.c shm_sem_recv.c \
     net_server.c net_client.c \
     curses.c pseudoterm.c readline.c \
     multilock_sem.c multilock_cond.c \
     memtest.c memtest2.c seznamy.c \
     udp_server.c udp_client.c tcp.c udp.c

# Prípony pro inferencní pravidla (inference rules)
.SUFFIXES:
.SUFFIXES: .o .c

# Preklad *.c -> *.o
.c.o :
	$(CC) $(CFLAGS) -c $*.c

# Linkování *.o -> program
.o :
	$(CC) $(CFLAGS) $(LDFLAGS) -o $* $*.o $(LIBS)

# Preklad a linkování *.c -> *.o -> program
# Pri pouzití GNU make toto pravidlo nemusíme psát, protoze si ho make odvodí
# z predchozích dvou (a na konci smaze takto vytvorené *.o soubory).
# Nemusíme ho psát ani kdyz máme v makefile uvedené závislosti tvaru
# *.o : *.c
# protoze se postupne uplatní obe predchozí pravidla
.c :
	$(CC) $(CFLAGS) -c $*.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $* $*.o $(LIBS)
	rm $*.o

# Prelozit vsechno
all : $(TARGETS)

# Linkování programu pouzívajících thready, pouzívá se knihovna pthread
shared_var : shared_var.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o shared_var shared_var.o \
	$(LIBS) -lpthread

thread_run : thread_run.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o thread_run thread_run.o \
	$(LIBS) -lpthread

thread_sync : thread_sync.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o thread_sync thread_sync.o \
	$(LIBS) -lpthread

thread_signal : thread_signal.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o thread_signal thread_signal.o \
	$(LIBS) -lpthread

thread_syscall : thread_syscall.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o thread_syscall thread_syscall.o \
	$(LIBS) -lpthread

multilock_sem : multilock_sem.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o multilock_sem multilock_sem.o \
	$(LIBS) -lpthread

multilock_cond : multilock_cond.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o multilock_cond multilock_cond.o \
	$(LIBS) -lpthread

# Linkování programu pouzívajících knihovnu (n)curses
curses : curses.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o curses curses.o $(LIBS) -lcurses

# Linkování programu pouzívajících knihovnu readline
readline : readline.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o readline readline.o $(LIBS) -lreadline \
	-l curses

# Programy lock_* (zamykání souboru) - je treba definovat správná makra
lock_nolock : lock.c
	$(CC) $(CFLAGS) -c lock.c -DLOCK_NOLOCK
	$(CC) $(CFLAGS) $(LDFLAGS) -o lock_nolock lock.o $(LIBS)
	rm lock.o
	
lock_lockfile : lock.c
	$(CC) $(CFLAGS) -c lock.c -DLOCK_LOCKFILE
	$(CC) $(CFLAGS) $(LDFLAGS) -o lock_lockfile lock.o $(LIBS)
	rm lock.o

lock_flock : lock.c
	$(CC) $(CFLAGS) -c lock.c -DLOCK_FLOCK
	$(CC) $(CFLAGS) $(LDFLAGS) -o lock_flock lock.o $(LIBS)
	rm lock.o

# Linkování programu dlopen s knihovnou libdl.so
dlopen : dlopen.o
	$(CC) $(CFLAGS) -o dlopen dlopen.o -ldl $(LIBS)

# Linkování sdílených knihoven pro program dlopen
dlopen_fact.so : dlopen_fact.o
	$(CC) $(CFLAGS) -shared -o dlopen_fact.so dlopen_fact.o

dlopen_ops.so : dlopen_ops.o
	$(CC) $(CFLAGS) -shared -o dlopen_ops.so dlopen_ops.o

# Smazat objektové soubory a zálohy editoru
clean :
	rm -f *.o *~

# Smazat prelozené programy
cleanbin :
	rm -f $(TARGETS)

# Vypsat seznam souboru *.c spolu s úvodním komentárem v kazdém souboru
list :
	for f in *.c;do echo $$f;awk 'NR==1,/\*\//{print}' $$f;echo;done
# Tags pro vi
tags : $(SRCS)
	ctags $(SRCS)

# Vytvorit seznam závislostí (pro GNU make nemusíme make depend volat, protoze
# make automaticky na zacátku zavolá pravidlo Makefile), navíc automaticky
# vygenerujeme i soubor tags pro editor vi
depend :
	makedepend $(CFLAGS) $(SRCS) > /dev/null 2>&1

# Makefile : $(SRCS) tags
#	makedepend $(CFLAGS) $(SRCS) > /dev/null 2>&1

# Jiné resení:
#Makefile : $(SRCS)
#	sed -n '1,/^# DO NOT DELETE THIS LINE/p' Makefile >Makefile.new
#	$(CC) $(CFLAGS) -M $(SRCS) >> Makefile.new
#	mv Makefile.new Makefile
 
# Seznam závislostí
# DO NOT DELETE THIS LINE -- make depend depends on it.


access.o: /usr/include/asm/errno.h /usr/include/bits/confname.h
access.o: /usr/include/bits/environments.h /usr/include/bits/errno.h
access.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
access.o: /usr/include/bits/sched.h /usr/include/bits/stdio_lim.h
access.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
access.o: /usr/include/errno.h /usr/include/features.h
access.o: /usr/include/_G_config.h /usr/include/gconv.h /usr/include/getopt.h
access.o: /usr/include/gnu/stubs.h /usr/include/libio.h
access.o: /usr/include/linux/errno.h /usr/include/stdio.h
access.o: /usr/include/sys/cdefs.h /usr/include/unistd.h /usr/include/wchar.h
access.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
access.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
args.o: /usr/include/alloca.h /usr/include/bits/endian.h
args.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
args.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
args.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
args.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
args.o: /usr/include/endian.h /usr/include/features.h /usr/include/_G_config.h
args.o: /usr/include/gconv.h /usr/include/gnu/stubs.h /usr/include/libgen.h
args.o: /usr/include/libio.h /usr/include/stdio.h /usr/include/stdlib.h
args.o: /usr/include/string.h /usr/include/sys/cdefs.h
args.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
args.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/wchar.h
args.o: /usr/include/xlocale.h
args.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
args.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
copy.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
copy.o: /usr/include/bits/environments.h /usr/include/bits/fcntl.h
copy.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
copy.o: /usr/include/bits/select.h /usr/include/bits/sched.h
copy.o: /usr/include/bits/sigset.h /usr/include/bits/stat.h
copy.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
copy.o: /usr/include/bits/wordsize.h /usr/include/endian.h
copy.o: /usr/include/fcntl.h /usr/include/features.h /usr/include/_G_config.h
copy.o: /usr/include/gconv.h /usr/include/getopt.h /usr/include/gnu/stubs.h
copy.o: /usr/include/libio.h /usr/include/stdio.h /usr/include/sys/cdefs.h
copy.o: /usr/include/sys/select.h /usr/include/sys/stat.h
copy.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
copy.o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
copy.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
copy.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
curses.o: /usr/include/alloca.h /usr/include/assert.h
curses.o: /usr/include/bits/endian.h /usr/include/bits/pthreadtypes.h
curses.o: /usr/include/bits/select.h /usr/include/bits/sched.h
curses.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
curses.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
curses.o: /usr/include/bits/waitstatus.h /usr/include/ctype.h
curses.o: /usr/include/curses.h /usr/include/endian.h /usr/include/features.h
curses.o: /usr/include/_G_config.h /usr/include/gconv.h
curses.o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/locale.h
curses.o: /usr/include/ncurses/curses.h /usr/include/ncurses/unctrl.h
curses.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/sys/cdefs.h
curses.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
curses.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/wchar.h
curses.o: /usr/include/xlocale.h
curses.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
curses.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
dir_name.o: /usr/include/alloca.h /usr/include/asm/errno.h
dir_name.o: /usr/include/bits/dirent.h /usr/include/bits/endian.h
dir_name.o: /usr/include/bits/errno.h /usr/include/bits/local_lim.h
dir_name.o: /usr/include/bits/posix1_lim.h /usr/include/bits/pthreadtypes.h
dir_name.o: /usr/include/bits/select.h /usr/include/bits/sched.h
dir_name.o: /usr/include/bits/sigset.h /usr/include/bits/stat.h
dir_name.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
dir_name.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
dir_name.o: /usr/include/dirent.h /usr/include/endian.h /usr/include/errno.h
dir_name.o: /usr/include/features.h /usr/include/_G_config.h
dir_name.o: /usr/include/gconv.h /usr/include/gnu/stubs.h /usr/include/libio.h
dir_name.o: /usr/include/linux/errno.h /usr/include/linux/limits.h
dir_name.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
dir_name.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
dir_name.o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
dir_name.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/wchar.h
dir_name.o: /usr/include/xlocale.h
dir_name.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
dir_name.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
dir.o: /usr/include/asm/errno.h /usr/include/bits/dirent.h
dir.o: /usr/include/bits/endian.h /usr/include/bits/errno.h
dir.o: /usr/include/bits/local_lim.h /usr/include/bits/posix1_lim.h
dir.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
dir.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
dir.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
dir.o: /usr/include/dirent.h /usr/include/endian.h /usr/include/errno.h
dir.o: /usr/include/features.h /usr/include/_G_config.h /usr/include/gconv.h
dir.o: /usr/include/gnu/stubs.h /usr/include/libio.h
dir.o: /usr/include/linux/errno.h /usr/include/linux/limits.h
dir.o: /usr/include/stdio.h /usr/include/sys/cdefs.h /usr/include/sys/select.h
dir.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
dir.o: /usr/include/time.h /usr/include/wchar.h
dir.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
dir.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
dlopen.o: /usr/include/alloca.h /usr/include/bits/dlfcn.h
dlopen.o: /usr/include/bits/endian.h /usr/include/bits/pthreadtypes.h
dlopen.o: /usr/include/bits/select.h /usr/include/bits/sched.h
dlopen.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
dlopen.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
dlopen.o: /usr/include/bits/waitstatus.h /usr/include/dlfcn.h
dlopen.o: /usr/include/endian.h /usr/include/features.h
dlopen.o: /usr/include/_G_config.h /usr/include/gconv.h
dlopen.o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/stdio.h
dlopen.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
dlopen.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
dlopen.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/wchar.h
dlopen.o: /usr/include/xlocale.h
dlopen.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
dlopen.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
env.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
env.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
env.o: /usr/include/features.h /usr/include/_G_config.h /usr/include/gconv.h
env.o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/stdio.h
env.o: /usr/include/sys/cdefs.h /usr/include/wchar.h
env.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
env.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
fcopy.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
fcopy.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
fcopy.o: /usr/include/features.h /usr/include/_G_config.h /usr/include/gconv.h
fcopy.o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/stdio.h
fcopy.o: /usr/include/sys/cdefs.h /usr/include/wchar.h
fcopy.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
fcopy.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
fork_cycl.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
fork_cycl.o: /usr/include/bits/environments.h /usr/include/bits/posix_opt.h
fork_cycl.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
fork_cycl.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
fork_cycl.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
fork_cycl.o: /usr/include/bits/wordsize.h /usr/include/endian.h
fork_cycl.o: /usr/include/features.h /usr/include/_G_config.h
fork_cycl.o: /usr/include/gconv.h /usr/include/getopt.h
fork_cycl.o: /usr/include/gnu/stubs.h /usr/include/libio.h
fork_cycl.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
fork_cycl.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
fork_cycl.o: /usr/include/sys/types.h /usr/include/time.h
fork_cycl.o: /usr/include/unistd.h /usr/include/wchar.h
fork_cycl.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
fork_cycl.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
fork_exec.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
fork_exec.o: /usr/include/bits/environments.h /usr/include/bits/posix_opt.h
fork_exec.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
fork_exec.o: /usr/include/bits/siginfo.h /usr/include/bits/stdio_lim.h
fork_exec.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
fork_exec.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
fork_exec.o: /usr/include/endian.h /usr/include/features.h
fork_exec.o: /usr/include/_G_config.h /usr/include/gconv.h
fork_exec.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
fork_exec.o: /usr/include/libio.h /usr/include/stdio.h
fork_exec.o: /usr/include/sys/cdefs.h /usr/include/sys/wait.h
fork_exec.o: /usr/include/unistd.h /usr/include/wchar.h
fork_exec.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
fork_exec.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
fork_wait.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
fork_wait.o: /usr/include/bits/environments.h /usr/include/bits/posix_opt.h
fork_wait.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
fork_wait.o: /usr/include/bits/sched.h /usr/include/bits/siginfo.h
fork_wait.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
fork_wait.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
fork_wait.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
fork_wait.o: /usr/include/endian.h /usr/include/features.h
fork_wait.o: /usr/include/_G_config.h /usr/include/gconv.h
fork_wait.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
fork_wait.o: /usr/include/libio.h /usr/include/stdio.h
fork_wait.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
fork_wait.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
fork_wait.o: /usr/include/sys/wait.h /usr/include/time.h /usr/include/unistd.h
fork_wait.o: /usr/include/wchar.h
fork_wait.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
fork_wait.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
getopt_long.o: /usr/include/alloca.h /usr/include/bits/confname.h
getopt_long.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
getopt_long.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
getopt_long.o: /usr/include/bits/select.h /usr/include/bits/sched.h
getopt_long.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
getopt_long.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
getopt_long.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
getopt_long.o: /usr/include/endian.h /usr/include/features.h
getopt_long.o: /usr/include/_G_config.h /usr/include/gconv.h
getopt_long.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
getopt_long.o: /usr/include/libgen.h /usr/include/libio.h /usr/include/stdio.h
getopt_long.o: /usr/include/stdlib.h /usr/include/string.h
getopt_long.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
getopt_long.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
getopt_long.o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
getopt_long.o: /usr/include/xlocale.h
getopt_long.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
getopt_long.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
getopt.o: /usr/include/alloca.h /usr/include/bits/confname.h
getopt.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
getopt.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
getopt.o: /usr/include/bits/select.h /usr/include/bits/sched.h
getopt.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
getopt.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
getopt.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
getopt.o: /usr/include/endian.h /usr/include/features.h
getopt.o: /usr/include/_G_config.h /usr/include/gconv.h /usr/include/getopt.h
getopt.o: /usr/include/gnu/stubs.h /usr/include/libgen.h /usr/include/libio.h
getopt.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
getopt.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
getopt.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
getopt.o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
getopt.o: /usr/include/xlocale.h
getopt.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
getopt.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
hello.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
hello.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
hello.o: /usr/include/features.h /usr/include/_G_config.h /usr/include/gconv.h
hello.o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/stdio.h
hello.o: /usr/include/sys/cdefs.h /usr/include/wchar.h
hello.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
hello.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
change_env.o: /usr/include/alloca.h /usr/include/bits/confname.h
change_env.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
change_env.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
change_env.o: /usr/include/bits/select.h /usr/include/bits/sched.h
change_env.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
change_env.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
change_env.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
change_env.o: /usr/include/endian.h /usr/include/features.h
change_env.o: /usr/include/_G_config.h /usr/include/gconv.h
change_env.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
change_env.o: /usr/include/libio.h /usr/include/stdio.h /usr/include/stdlib.h
change_env.o: /usr/include/string.h /usr/include/sys/cdefs.h
change_env.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
change_env.o: /usr/include/sys/types.h /usr/include/time.h
change_env.o: /usr/include/unistd.h /usr/include/wchar.h
change_env.o: /usr/include/xlocale.h
change_env.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
change_env.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
lock.o: mechanism??? /usr/include/alloca.h /usr/include/asm/errno.h
lock.o: /usr/include/asm/sigcontext.h /usr/include/bits/confname.h
lock.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
lock.o: /usr/include/bits/errno.h /usr/include/bits/fcntl.h
lock.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
lock.o: /usr/include/bits/select.h /usr/include/bits/sched.h
lock.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
lock.o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
lock.o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
lock.o: /usr/include/bits/sigthread.h /usr/include/bits/stat.h
lock.o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
lock.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
lock.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
lock.o: /usr/include/endian.h /usr/include/errno.h /usr/include/fcntl.h
lock.o: /usr/include/features.h /usr/include/_G_config.h /usr/include/gconv.h
lock.o: /usr/include/getopt.h /usr/include/gnu/stubs.h /usr/include/libio.h
lock.o: /usr/include/linux/errno.h /usr/include/signal.h /usr/include/stdio.h
lock.o: /usr/include/stdlib.h /usr/include/string.h /usr/include/sys/cdefs.h
lock.o: /usr/include/sys/file.h /usr/include/sys/select.h
lock.o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
lock.o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
lock.o: /usr/include/time.h /usr/include/ucontext.h /usr/include/unistd.h
lock.o: /usr/include/wchar.h /usr/include/xlocale.h
lock.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
lock.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
lseek.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
lseek.o: /usr/include/bits/environments.h /usr/include/bits/fcntl.h
lseek.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
lseek.o: /usr/include/bits/select.h /usr/include/bits/sched.h
lseek.o: /usr/include/bits/sigset.h /usr/include/bits/stat.h
lseek.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
lseek.o: /usr/include/bits/wordsize.h /usr/include/endian.h
lseek.o: /usr/include/fcntl.h /usr/include/features.h /usr/include/_G_config.h
lseek.o: /usr/include/gconv.h /usr/include/getopt.h /usr/include/gnu/stubs.h
lseek.o: /usr/include/libio.h /usr/include/stdio.h /usr/include/string.h
lseek.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
lseek.o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
lseek.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/unistd.h
lseek.o: /usr/include/wchar.h /usr/include/xlocale.h
lseek.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
lseek.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
memtest.o: /usr/include/alloca.h /usr/include/bits/endian.h
memtest.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
memtest.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
memtest.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
memtest.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
memtest.o: /usr/include/endian.h /usr/include/features.h
memtest.o: /usr/include/_G_config.h /usr/include/gconv.h
memtest.o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/stdio.h
memtest.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
memtest.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
memtest.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/wchar.h
memtest.o: /usr/include/xlocale.h
memtest.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
memtest.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
memtest2.o: /usr/include/alloca.h /usr/include/bits/confname.h
memtest2.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
memtest2.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
memtest2.o: /usr/include/bits/select.h /usr/include/bits/sched.h
memtest2.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
memtest2.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
memtest2.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
memtest2.o: /usr/include/endian.h /usr/include/features.h
memtest2.o: /usr/include/_G_config.h /usr/include/gconv.h
memtest2.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
memtest2.o: /usr/include/libio.h /usr/include/stdio.h /usr/include/stdlib.h
memtest2.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
memtest2.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
memtest2.o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
memtest2.o: /usr/include/xlocale.h
memtest2.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
memtest2.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
mmap.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
mmap.o: /usr/include/bits/environments.h /usr/include/bits/fcntl.h
mmap.o: /usr/include/bits/mman.h /usr/include/bits/posix_opt.h
mmap.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
mmap.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
mmap.o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
mmap.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
mmap.o: /usr/include/endian.h /usr/include/fcntl.h /usr/include/features.h
mmap.o: /usr/include/_G_config.h /usr/include/gconv.h /usr/include/getopt.h
mmap.o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/stdio.h
mmap.o: /usr/include/string.h /usr/include/sys/cdefs.h /usr/include/sys/mman.h
mmap.o: /usr/include/sys/select.h /usr/include/sys/stat.h
mmap.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
mmap.o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
mmap.o: /usr/include/xlocale.h
mmap.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
mmap.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
mmap_reverse.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
mmap_reverse.o: /usr/include/bits/environments.h /usr/include/bits/fcntl.h
mmap_reverse.o: /usr/include/bits/mman.h /usr/include/bits/posix_opt.h
mmap_reverse.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
mmap_reverse.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
mmap_reverse.o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
mmap_reverse.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
mmap_reverse.o: /usr/include/endian.h /usr/include/fcntl.h
mmap_reverse.o: /usr/include/features.h /usr/include/_G_config.h
mmap_reverse.o: /usr/include/gconv.h /usr/include/getopt.h
mmap_reverse.o: /usr/include/gnu/stubs.h /usr/include/libio.h
mmap_reverse.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
mmap_reverse.o: /usr/include/sys/mman.h /usr/include/sys/select.h
mmap_reverse.o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
mmap_reverse.o: /usr/include/sys/types.h /usr/include/time.h
mmap_reverse.o: /usr/include/unistd.h /usr/include/wchar.h
mmap_reverse.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
mmap_reverse.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
msg_recv.o: /usr/include/alloca.h /usr/include/bits/confname.h
msg_recv.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
msg_recv.o: /usr/include/bits/ipc.h /usr/include/bits/msq.h
msg_recv.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
msg_recv.o: /usr/include/bits/select.h /usr/include/bits/sched.h
msg_recv.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
msg_recv.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
msg_recv.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
msg_recv.o: /usr/include/endian.h /usr/include/features.h
msg_recv.o: /usr/include/_G_config.h /usr/include/gconv.h
msg_recv.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
msg_recv.o: /usr/include/libio.h /usr/include/stdio.h /usr/include/stdlib.h
msg_recv.o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
msg_recv.o: /usr/include/sys/msg.h /usr/include/sys/select.h
msg_recv.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
msg_recv.o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
msg_recv.o: /usr/include/xlocale.h
msg_recv.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
msg_recv.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
msg_send.o: /usr/include/alloca.h /usr/include/bits/confname.h
msg_send.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
msg_send.o: /usr/include/bits/ipc.h /usr/include/bits/msq.h
msg_send.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
msg_send.o: /usr/include/bits/select.h /usr/include/bits/sched.h
msg_send.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
msg_send.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
msg_send.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
msg_send.o: /usr/include/endian.h /usr/include/features.h
msg_send.o: /usr/include/_G_config.h /usr/include/gconv.h
msg_send.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
msg_send.o: /usr/include/libio.h /usr/include/stdio.h /usr/include/stdlib.h
msg_send.o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
msg_send.o: /usr/include/sys/msg.h /usr/include/sys/select.h
msg_send.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
msg_send.o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
msg_send.o: /usr/include/xlocale.h
msg_send.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
msg_send.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
multilock_cond.o: /usr/include/alloca.h /usr/include/asm/errno.h
multilock_cond.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
multilock_cond.o: /usr/include/bits/environments.h /usr/include/bits/errno.h
multilock_cond.o: /usr/include/bits/posix_opt.h
multilock_cond.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
multilock_cond.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
multilock_cond.o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
multilock_cond.o: /usr/include/bits/time.h /usr/include/bits/types.h
multilock_cond.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
multilock_cond.o: /usr/include/bits/wordsize.h /usr/include/endian.h
multilock_cond.o: /usr/include/errno.h /usr/include/features.h
multilock_cond.o: /usr/include/_G_config.h /usr/include/gconv.h
multilock_cond.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
multilock_cond.o: /usr/include/libio.h /usr/include/linux/errno.h
multilock_cond.o: /usr/include/pthread.h /usr/include/sched.h
multilock_cond.o: /usr/include/signal.h /usr/include/stdio.h
multilock_cond.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
multilock_cond.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
multilock_cond.o: /usr/include/sys/types.h /usr/include/time.h
multilock_cond.o: /usr/include/unistd.h /usr/include/wchar.h
multilock_cond.o: /usr/include/xlocale.h
multilock_cond.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
multilock_cond.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
multilock_sem.o: /usr/include/alloca.h /usr/include/asm/errno.h
multilock_sem.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
multilock_sem.o: /usr/include/bits/environments.h /usr/include/bits/errno.h
multilock_sem.o: /usr/include/bits/ipc.h /usr/include/bits/posix_opt.h
multilock_sem.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
multilock_sem.o: /usr/include/bits/sem.h /usr/include/bits/sched.h
multilock_sem.o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
multilock_sem.o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
multilock_sem.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
multilock_sem.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
multilock_sem.o: /usr/include/endian.h /usr/include/errno.h
multilock_sem.o: /usr/include/features.h /usr/include/_G_config.h
multilock_sem.o: /usr/include/gconv.h /usr/include/getopt.h
multilock_sem.o: /usr/include/gnu/stubs.h /usr/include/libio.h
multilock_sem.o: /usr/include/linux/errno.h /usr/include/pthread.h
multilock_sem.o: /usr/include/sched.h /usr/include/signal.h
multilock_sem.o: /usr/include/stdio.h /usr/include/stdlib.h
multilock_sem.o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
multilock_sem.o: /usr/include/sys/select.h /usr/include/sys/sem.h
multilock_sem.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
multilock_sem.o: /usr/include/time.h /usr/include/unistd.h
multilock_sem.o: /usr/include/wchar.h /usr/include/xlocale.h
multilock_sem.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
multilock_sem.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
net_client.o: /usr/include/alloca.h /usr/include/arpa/inet.h
net_client.o: /usr/include/asm/errno.h /usr/include/asm/socket.h
net_client.o: /usr/include/asm/sockios.h /usr/include/bits/byteswap.h
net_client.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
net_client.o: /usr/include/bits/environments.h /usr/include/bits/errno.h
net_client.o: /usr/include/bits/in.h /usr/include/bits/netdb.h
net_client.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
net_client.o: /usr/include/bits/select.h /usr/include/bits/sched.h
net_client.o: /usr/include/bits/sigset.h /usr/include/bits/sockaddr.h
net_client.o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
net_client.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
net_client.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
net_client.o: /usr/include/endian.h /usr/include/errno.h
net_client.o: /usr/include/features.h /usr/include/_G_config.h
net_client.o: /usr/include/gconv.h /usr/include/getopt.h
net_client.o: /usr/include/gnu/stubs.h /usr/include/libio.h
net_client.o: /usr/include/linux/errno.h /usr/include/netdb.h
net_client.o: /usr/include/netinet/in.h /usr/include/rpc/netdb.h
net_client.o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/stdlib.h
net_client.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
net_client.o: /usr/include/sys/socket.h /usr/include/sys/sysmacros.h
net_client.o: /usr/include/sys/types.h /usr/include/time.h
net_client.o: /usr/include/unistd.h /usr/include/wchar.h
net_client.o: /usr/include/xlocale.h
net_client.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
net_client.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
net_client.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
net_client.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
net_server.o: /usr/include/alloca.h /usr/include/arpa/inet.h
net_server.o: /usr/include/asm/errno.h /usr/include/asm/socket.h
net_server.o: /usr/include/asm/sockios.h /usr/include/bits/byteswap.h
net_server.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
net_server.o: /usr/include/bits/environments.h /usr/include/bits/errno.h
net_server.o: /usr/include/bits/in.h /usr/include/bits/netdb.h
net_server.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
net_server.o: /usr/include/bits/select.h /usr/include/bits/sched.h
net_server.o: /usr/include/bits/sigset.h /usr/include/bits/sockaddr.h
net_server.o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
net_server.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
net_server.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
net_server.o: /usr/include/endian.h /usr/include/errno.h
net_server.o: /usr/include/features.h /usr/include/_G_config.h
net_server.o: /usr/include/gconv.h /usr/include/getopt.h
net_server.o: /usr/include/gnu/stubs.h /usr/include/libio.h
net_server.o: /usr/include/linux/errno.h /usr/include/netdb.h
net_server.o: /usr/include/netinet/in.h /usr/include/rpc/netdb.h
net_server.o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/stdlib.h
net_server.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
net_server.o: /usr/include/sys/socket.h /usr/include/sys/sysmacros.h
net_server.o: /usr/include/sys/types.h /usr/include/time.h
net_server.o: /usr/include/unistd.h /usr/include/wchar.h
net_server.o: /usr/include/xlocale.h
net_server.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
net_server.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
net_server.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
net_server.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
pipe_bash.o: /usr/include/alloca.h /usr/include/asm/errno.h
pipe_bash.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
pipe_bash.o: /usr/include/bits/environments.h /usr/include/bits/errno.h
pipe_bash.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
pipe_bash.o: /usr/include/bits/select.h /usr/include/bits/sched.h
pipe_bash.o: /usr/include/bits/siginfo.h /usr/include/bits/sigset.h
pipe_bash.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
pipe_bash.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
pipe_bash.o: /usr/include/bits/wordsize.h /usr/include/endian.h
pipe_bash.o: /usr/include/errno.h /usr/include/features.h
pipe_bash.o: /usr/include/_G_config.h /usr/include/gconv.h
pipe_bash.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
pipe_bash.o: /usr/include/libgen.h /usr/include/libio.h
pipe_bash.o: /usr/include/linux/errno.h /usr/include/stdio.h
pipe_bash.o: /usr/include/stdlib.h /usr/include/string.h
pipe_bash.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
pipe_bash.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
pipe_bash.o: /usr/include/sys/wait.h /usr/include/time.h /usr/include/unistd.h
pipe_bash.o: /usr/include/wait.h /usr/include/wchar.h /usr/include/xlocale.h
pipe_bash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
pipe_bash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
pipe_capacity.o: /usr/include/alloca.h /usr/include/bits/confname.h
pipe_capacity.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
pipe_capacity.o: /usr/include/bits/posix_opt.h
pipe_capacity.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
pipe_capacity.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
pipe_capacity.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
pipe_capacity.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
pipe_capacity.o: /usr/include/bits/wordsize.h /usr/include/endian.h
pipe_capacity.o: /usr/include/features.h /usr/include/_G_config.h
pipe_capacity.o: /usr/include/gconv.h /usr/include/getopt.h
pipe_capacity.o: /usr/include/gnu/stubs.h /usr/include/libio.h
pipe_capacity.o: /usr/include/stdio.h /usr/include/stdlib.h
pipe_capacity.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
pipe_capacity.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
pipe_capacity.o: /usr/include/time.h /usr/include/unistd.h
pipe_capacity.o: /usr/include/wchar.h /usr/include/xlocale.h
pipe_capacity.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
pipe_capacity.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
pipe_c2p.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
pipe_c2p.o: /usr/include/bits/environments.h /usr/include/bits/posix_opt.h
pipe_c2p.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
pipe_c2p.o: /usr/include/bits/siginfo.h /usr/include/bits/stdio_lim.h
pipe_c2p.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
pipe_c2p.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
pipe_c2p.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/features.h
pipe_c2p.o: /usr/include/_G_config.h /usr/include/gconv.h
pipe_c2p.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
pipe_c2p.o: /usr/include/libio.h /usr/include/stdio.h /usr/include/sys/cdefs.h
pipe_c2p.o: /usr/include/sys/wait.h /usr/include/unistd.h /usr/include/wchar.h
pipe_c2p.o: /usr/include/xlocale.h
pipe_c2p.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
pipe_c2p.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
pipe_p2c.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
pipe_p2c.o: /usr/include/bits/environments.h /usr/include/bits/posix_opt.h
pipe_p2c.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
pipe_p2c.o: /usr/include/bits/siginfo.h /usr/include/bits/stdio_lim.h
pipe_p2c.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
pipe_p2c.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
pipe_p2c.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/features.h
pipe_p2c.o: /usr/include/_G_config.h /usr/include/gconv.h
pipe_p2c.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
pipe_p2c.o: /usr/include/libio.h /usr/include/stdio.h /usr/include/sys/cdefs.h
pipe_p2c.o: /usr/include/sys/wait.h /usr/include/unistd.h /usr/include/wchar.h
pipe_p2c.o: /usr/include/xlocale.h
pipe_p2c.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
pipe_p2c.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
pipe_select.o: /usr/include/alloca.h /usr/include/bits/confname.h
pipe_select.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
pipe_select.o: /usr/include/bits/fcntl.h /usr/include/bits/posix_opt.h
pipe_select.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
pipe_select.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
pipe_select.o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
pipe_select.o: /usr/include/bits/time.h /usr/include/bits/types.h
pipe_select.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
pipe_select.o: /usr/include/bits/wordsize.h /usr/include/endian.h
pipe_select.o: /usr/include/fcntl.h /usr/include/features.h
pipe_select.o: /usr/include/_G_config.h /usr/include/gconv.h
pipe_select.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
pipe_select.o: /usr/include/libio.h /usr/include/stdio.h /usr/include/stdlib.h
pipe_select.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
pipe_select.o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
pipe_select.o: /usr/include/sys/time.h /usr/include/sys/types.h
pipe_select.o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
pipe_select.o: /usr/include/xlocale.h
pipe_select.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
pipe_select.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
pipe_sh.o: /usr/include/alloca.h /usr/include/bits/confname.h
pipe_sh.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
pipe_sh.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
pipe_sh.o: /usr/include/bits/select.h /usr/include/bits/sched.h
pipe_sh.o: /usr/include/bits/siginfo.h /usr/include/bits/sigset.h
pipe_sh.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
pipe_sh.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
pipe_sh.o: /usr/include/bits/wordsize.h /usr/include/endian.h
pipe_sh.o: /usr/include/features.h /usr/include/_G_config.h
pipe_sh.o: /usr/include/gconv.h /usr/include/getopt.h /usr/include/gnu/stubs.h
pipe_sh.o: /usr/include/libgen.h /usr/include/libio.h /usr/include/stdio.h
pipe_sh.o: /usr/include/stdlib.h /usr/include/string.h
pipe_sh.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
pipe_sh.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
pipe_sh.o: /usr/include/sys/wait.h /usr/include/time.h /usr/include/unistd.h
pipe_sh.o: /usr/include/wait.h /usr/include/wchar.h /usr/include/xlocale.h
pipe_sh.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
pipe_sh.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
proc_info.o: /usr/include/alloca.h /usr/include/bits/confname.h
proc_info.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
proc_info.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
proc_info.o: /usr/include/bits/select.h /usr/include/bits/sched.h
proc_info.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
proc_info.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
proc_info.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
proc_info.o: /usr/include/endian.h /usr/include/features.h
proc_info.o: /usr/include/_G_config.h /usr/include/gconv.h
proc_info.o: /usr/include/getopt.h /usr/include/gnu/stubs.h /usr/include/grp.h
proc_info.o: /usr/include/libio.h /usr/include/pwd.h /usr/include/stdio.h
proc_info.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
proc_info.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
proc_info.o: /usr/include/sys/types.h /usr/include/time.h
proc_info.o: /usr/include/unistd.h /usr/include/wchar.h /usr/include/xlocale.h
proc_info.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
proc_info.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
proc_info.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
proc_info.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
pseudoterm.o: /usr/include/alloca.h /usr/include/asm/errno.h
pseudoterm.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
pseudoterm.o: /usr/include/bits/environments.h /usr/include/bits/errno.h
pseudoterm.o: /usr/include/bits/fcntl.h /usr/include/bits/posix_opt.h
pseudoterm.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
pseudoterm.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
pseudoterm.o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
pseudoterm.o: /usr/include/bits/time.h /usr/include/bits/types.h
pseudoterm.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
pseudoterm.o: /usr/include/bits/wordsize.h /usr/include/endian.h
pseudoterm.o: /usr/include/errno.h /usr/include/fcntl.h
pseudoterm.o: /usr/include/features.h /usr/include/_G_config.h
pseudoterm.o: /usr/include/gconv.h /usr/include/getopt.h
pseudoterm.o: /usr/include/gnu/stubs.h /usr/include/libio.h
pseudoterm.o: /usr/include/linux/errno.h /usr/include/stdio.h
pseudoterm.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
pseudoterm.o: /usr/include/sys/fcntl.h /usr/include/sys/select.h
pseudoterm.o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
pseudoterm.o: /usr/include/sys/time.h /usr/include/sys/types.h
pseudoterm.o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
pseudoterm.o: /usr/include/xlocale.h
pseudoterm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
pseudoterm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
readline.o: /usr/include/alloca.h /usr/include/bits/endian.h
readline.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
readline.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
readline.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
readline.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
readline.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/features.h
readline.o: /usr/include/_G_config.h /usr/include/gconv.h
readline.o: /usr/include/gnu/stubs.h /usr/include/libio.h
readline.o: /usr/include/readline/history.h /usr/include/readline/chardefs.h
readline.o: /usr/include/readline/keymaps.h /usr/include/readline/readline.h
readline.o: /usr/include/readline/rlstdc.h /usr/include/readline/tilde.h
readline.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
readline.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
readline.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
readline.o: /usr/include/time.h /usr/include/wchar.h /usr/include/xlocale.h
readline.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
readline.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
seznamy.o: /usr/include/alloca.h /usr/include/asm/errno.h
seznamy.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
seznamy.o: /usr/include/bits/environments.h /usr/include/bits/errno.h
seznamy.o: /usr/include/bits/fcntl.h /usr/include/bits/mman.h
seznamy.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
seznamy.o: /usr/include/bits/select.h /usr/include/bits/sched.h
seznamy.o: /usr/include/bits/sigset.h /usr/include/bits/stat.h
seznamy.o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
seznamy.o: /usr/include/bits/types.h /usr/include/bits/utsname.h
seznamy.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
seznamy.o: /usr/include/bits/wordsize.h /usr/include/ctype.h
seznamy.o: /usr/include/endian.h /usr/include/errno.h /usr/include/fcntl.h
seznamy.o: /usr/include/features.h /usr/include/_G_config.h
seznamy.o: /usr/include/gconv.h /usr/include/getopt.h /usr/include/gnu/stubs.h
seznamy.o: /usr/include/libio.h /usr/include/linux/errno.h
seznamy.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
seznamy.o: /usr/include/sys/cdefs.h /usr/include/sys/mman.h
seznamy.o: /usr/include/sys/select.h /usr/include/sys/stat.h
seznamy.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
seznamy.o: /usr/include/sys/types.h /usr/include/sys/utsname.h
seznamy.o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
seznamy.o: /usr/include/xlocale.h
seznamy.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
seznamy.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
shared_var.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
shared_var.o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
shared_var.o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
shared_var.o: /usr/include/bits/types.h /usr/include/features.h
shared_var.o: /usr/include/_G_config.h /usr/include/gconv.h
shared_var.o: /usr/include/gnu/stubs.h /usr/include/libio.h
shared_var.o: /usr/include/pthread.h /usr/include/sched.h
shared_var.o: /usr/include/signal.h /usr/include/stdio.h
shared_var.o: /usr/include/sys/cdefs.h /usr/include/time.h
shared_var.o: /usr/include/wchar.h
shared_var.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
shared_var.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
shm_sem_recv.o: /usr/include/alloca.h /usr/include/bits/confname.h
shm_sem_recv.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
shm_sem_recv.o: /usr/include/bits/ipc.h /usr/include/bits/posix_opt.h
shm_sem_recv.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
shm_sem_recv.o: /usr/include/bits/sem.h /usr/include/bits/shm.h
shm_sem_recv.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
shm_sem_recv.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
shm_sem_recv.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
shm_sem_recv.o: /usr/include/bits/wordsize.h /usr/include/endian.h
shm_sem_recv.o: /usr/include/features.h /usr/include/_G_config.h
shm_sem_recv.o: /usr/include/gconv.h /usr/include/getopt.h
shm_sem_recv.o: /usr/include/gnu/stubs.h /usr/include/libio.h
shm_sem_recv.o: /usr/include/stdio.h /usr/include/stdlib.h
shm_sem_recv.o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
shm_sem_recv.o: /usr/include/sys/select.h /usr/include/sys/sem.h
shm_sem_recv.o: /usr/include/sys/shm.h /usr/include/sys/sysmacros.h
shm_sem_recv.o: /usr/include/sys/types.h /usr/include/time.h
shm_sem_recv.o: /usr/include/unistd.h /usr/include/wchar.h
shm_sem_recv.o: /usr/include/xlocale.h
shm_sem_recv.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
shm_sem_recv.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
shm_sem_send.o: /usr/include/alloca.h /usr/include/bits/confname.h
shm_sem_send.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
shm_sem_send.o: /usr/include/bits/ipc.h /usr/include/bits/posix_opt.h
shm_sem_send.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
shm_sem_send.o: /usr/include/bits/sem.h /usr/include/bits/shm.h
shm_sem_send.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
shm_sem_send.o: /usr/include/bits/stdio_lim.h /usr/include/bits/types.h
shm_sem_send.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
shm_sem_send.o: /usr/include/bits/wordsize.h /usr/include/endian.h
shm_sem_send.o: /usr/include/features.h /usr/include/_G_config.h
shm_sem_send.o: /usr/include/gconv.h /usr/include/getopt.h
shm_sem_send.o: /usr/include/gnu/stubs.h /usr/include/libio.h
shm_sem_send.o: /usr/include/stdio.h /usr/include/stdlib.h
shm_sem_send.o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
shm_sem_send.o: /usr/include/sys/select.h /usr/include/sys/sem.h
shm_sem_send.o: /usr/include/sys/shm.h /usr/include/sys/sysmacros.h
shm_sem_send.o: /usr/include/sys/types.h /usr/include/time.h
shm_sem_send.o: /usr/include/unistd.h /usr/include/wchar.h
shm_sem_send.o: /usr/include/xlocale.h
shm_sem_send.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
shm_sem_send.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
sigaction.o: /usr/include/alloca.h /usr/include/asm/errno.h
sigaction.o: /usr/include/asm/sigcontext.h /usr/include/bits/confname.h
sigaction.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
sigaction.o: /usr/include/bits/errno.h /usr/include/bits/posix_opt.h
sigaction.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
sigaction.o: /usr/include/bits/setjmp.h /usr/include/bits/sched.h
sigaction.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
sigaction.o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
sigaction.o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
sigaction.o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
sigaction.o: /usr/include/bits/time.h /usr/include/bits/types.h
sigaction.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
sigaction.o: /usr/include/bits/wordsize.h /usr/include/endian.h
sigaction.o: /usr/include/errno.h /usr/include/features.h
sigaction.o: /usr/include/_G_config.h /usr/include/gconv.h
sigaction.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
sigaction.o: /usr/include/libio.h /usr/include/linux/errno.h
sigaction.o: /usr/include/setjmp.h /usr/include/signal.h /usr/include/stdio.h
sigaction.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
sigaction.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
sigaction.o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
sigaction.o: /usr/include/time.h /usr/include/ucontext.h /usr/include/unistd.h
sigaction.o: /usr/include/wchar.h /usr/include/xlocale.h
sigaction.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
sigaction.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
sig_alrm.o: /usr/include/asm/errno.h /usr/include/asm/sigcontext.h
sig_alrm.o: /usr/include/bits/confname.h /usr/include/bits/environments.h
sig_alrm.o: /usr/include/bits/errno.h /usr/include/bits/posix_opt.h
sig_alrm.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
sig_alrm.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
sig_alrm.o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
sig_alrm.o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
sig_alrm.o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
sig_alrm.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
sig_alrm.o: /usr/include/errno.h /usr/include/features.h
sig_alrm.o: /usr/include/_G_config.h /usr/include/gconv.h
sig_alrm.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
sig_alrm.o: /usr/include/libio.h /usr/include/linux/errno.h
sig_alrm.o: /usr/include/signal.h /usr/include/stdio.h /usr/include/string.h
sig_alrm.o: /usr/include/sys/cdefs.h /usr/include/sys/ucontext.h
sig_alrm.o: /usr/include/time.h /usr/include/ucontext.h /usr/include/unistd.h
sig_alrm.o: /usr/include/wchar.h /usr/include/xlocale.h
sig_alrm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
sig_alrm.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
stat_link.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
stat_link.o: /usr/include/bits/environments.h /usr/include/bits/posix_opt.h
stat_link.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
stat_link.o: /usr/include/bits/sched.h /usr/include/bits/sigset.h
stat_link.o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
stat_link.o: /usr/include/bits/types.h /usr/include/bits/wordsize.h
stat_link.o: /usr/include/endian.h /usr/include/features.h
stat_link.o: /usr/include/_G_config.h /usr/include/gconv.h
stat_link.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
stat_link.o: /usr/include/libio.h /usr/include/stdio.h
stat_link.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
stat_link.o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
stat_link.o: /usr/include/sys/types.h /usr/include/time.h
stat_link.o: /usr/include/unistd.h /usr/include/wchar.h
stat_link.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
stat_link.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
stat_link.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
stat_link.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
sysconf.o: /usr/include/alloca.h /usr/include/bits/confname.h
sysconf.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
sysconf.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
sysconf.o: /usr/include/bits/select.h /usr/include/bits/sched.h
sysconf.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
sysconf.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
sysconf.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
sysconf.o: /usr/include/endian.h /usr/include/features.h
sysconf.o: /usr/include/_G_config.h /usr/include/gconv.h /usr/include/getopt.h
sysconf.o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/stdio.h
sysconf.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
sysconf.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
sysconf.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/unistd.h
sysconf.o: /usr/include/wchar.h /usr/include/xlocale.h
sysconf.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
sysconf.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
thread_run.o: /usr/include/asm/errno.h /usr/include/bits/errno.h
thread_run.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
thread_run.o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
thread_run.o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
thread_run.o: /usr/include/bits/types.h /usr/include/errno.h
thread_run.o: /usr/include/features.h /usr/include/_G_config.h
thread_run.o: /usr/include/gconv.h /usr/include/gnu/stubs.h
thread_run.o: /usr/include/libio.h /usr/include/linux/errno.h
thread_run.o: /usr/include/pthread.h /usr/include/sched.h
thread_run.o: /usr/include/signal.h /usr/include/stdio.h
thread_run.o: /usr/include/sys/cdefs.h /usr/include/time.h
thread_run.o: /usr/include/wchar.h
thread_run.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
thread_run.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
thread_signal.o: /usr/include/alloca.h /usr/include/asm/sigcontext.h
thread_signal.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
thread_signal.o: /usr/include/bits/environments.h
thread_signal.o: /usr/include/bits/posix_opt.h
thread_signal.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/select.h
thread_signal.o: /usr/include/bits/sched.h /usr/include/bits/sigaction.h
thread_signal.o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
thread_signal.o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
thread_signal.o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
thread_signal.o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
thread_signal.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
thread_signal.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
thread_signal.o: /usr/include/endian.h /usr/include/features.h
thread_signal.o: /usr/include/_G_config.h /usr/include/gconv.h
thread_signal.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
thread_signal.o: /usr/include/libio.h /usr/include/pthread.h
thread_signal.o: /usr/include/sched.h /usr/include/signal.h
thread_signal.o: /usr/include/stdio.h /usr/include/stdlib.h
thread_signal.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
thread_signal.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
thread_signal.o: /usr/include/sys/ucontext.h /usr/include/time.h
thread_signal.o: /usr/include/ucontext.h /usr/include/unistd.h
thread_signal.o: /usr/include/wchar.h /usr/include/xlocale.h
thread_signal.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
thread_signal.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
thread_sync.o: /usr/include/alloca.h /usr/include/bits/confname.h
thread_sync.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
thread_sync.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
thread_sync.o: /usr/include/bits/select.h /usr/include/bits/sched.h
thread_sync.o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
thread_sync.o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
thread_sync.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
thread_sync.o: /usr/include/bits/waitstatus.h /usr/include/bits/wordsize.h
thread_sync.o: /usr/include/endian.h /usr/include/features.h
thread_sync.o: /usr/include/_G_config.h /usr/include/gconv.h
thread_sync.o: /usr/include/getopt.h /usr/include/gnu/stubs.h
thread_sync.o: /usr/include/libio.h /usr/include/pthread.h
thread_sync.o: /usr/include/sched.h /usr/include/signal.h /usr/include/stdio.h
thread_sync.o: /usr/include/stdlib.h /usr/include/sys/cdefs.h
thread_sync.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
thread_sync.o: /usr/include/sys/types.h /usr/include/time.h
thread_sync.o: /usr/include/unistd.h /usr/include/wchar.h
thread_sync.o: /usr/include/xlocale.h
thread_sync.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
thread_sync.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
thread_syscall.o: /usr/include/asm/errno.h /usr/include/asm/sigcontext.h
thread_syscall.o: /usr/include/bits/confname.h
thread_syscall.o: /usr/include/bits/environments.h /usr/include/bits/errno.h
thread_syscall.o: /usr/include/bits/posix_opt.h
thread_syscall.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
thread_syscall.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
thread_syscall.o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
thread_syscall.o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
thread_syscall.o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
thread_syscall.o: /usr/include/bits/time.h /usr/include/bits/types.h
thread_syscall.o: /usr/include/bits/wordsize.h /usr/include/errno.h
thread_syscall.o: /usr/include/features.h /usr/include/_G_config.h
thread_syscall.o: /usr/include/gconv.h /usr/include/getopt.h
thread_syscall.o: /usr/include/gnu/stubs.h /usr/include/libio.h
thread_syscall.o: /usr/include/linux/errno.h /usr/include/pthread.h
thread_syscall.o: /usr/include/sched.h /usr/include/signal.h
thread_syscall.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
thread_syscall.o: /usr/include/sys/ucontext.h /usr/include/time.h
thread_syscall.o: /usr/include/ucontext.h /usr/include/unistd.h
thread_syscall.o: /usr/include/wchar.h
thread_syscall.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
thread_syscall.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
