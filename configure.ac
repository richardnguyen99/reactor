#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Initialize autoconf
AC_INIT([reactor], [0.0.1])
AC_PREREQ([2.69])

# Configure the build process
# AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_INSTALL

# Check for automake
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])
AM_PROG_AR
AM_PROG_CC_C_O

LT_INIT

# Check for libraries or dependencies (if needed)
# Example: AC_CHECK_LIB([library_name], [function_name], [action-if-found], [action-if-not-found])
AC_CHECK_LIB([pthread], [pthread_create], [], [AC_MSG_ERROR([pthread library not found])])
AC_CHECK_LIB([sqlite3], [sqlite3_open],   [], [AC_MSG_ERROR([sqlite3 library not found])])

# Check for header files (if needed)
# Example: AC_CHECK_HEADERS([header_file], [action-if-found], [action-if-not-found])
AC_CHECK_HEADERS([sqlite3.h],       [], [AC_MSG_ERROR([sqlite3.h not found])])
AC_CHECK_HEADERS([sqlite3ext.h],    [], [AC_MSG_ERROR([sqlite3ext.h not found])])
AC_CHECK_HEADERS([assert.h],        [], [AC_MSG_ERROR([assert.h not found])])
AC_CHECK_HEADERS([pthread.h],       [], [AC_MSG_ERROR([pthread.h not found])])
AC_CHECK_HEADERS([sys/epoll.h],     [], [AC_MSG_ERROR([sys/epoll.h not found])])
AC_CHECK_HEADERS([fcntl.h],         [], [AC_MSG_ERROR([fcntl.h not found])])
AC_CHECK_HEADERS([unistd.h],        [], [AC_MSG_ERROR([unistd.h not found])])
AC_CHECK_HEADERS([sys/socket.h],    [], [AC_MSG_ERROR([sys/socket.h not found])])
AC_CHECK_HEADERS([netinet/in.h],    [], [AC_MSG_ERROR([netinet/in.h not found])])
AC_CHECK_HEADERS([arpa/inet.h],     [], [AC_MSG_ERROR([arpa/inet.h not found])])
AC_CHECK_HEADERS([sys/types.h],     [], [AC_MSG_ERROR([sys/types.h not found])])
AC_CHECK_HEADERS([sys/stat.h],      [], [AC_MSG_ERROR([sys/stat.h not found])])
AC_CHECK_HEADERS([stdlib.h],        [], [AC_MSG_ERROR([stdlib.h not found])])
AC_CHECK_HEADERS([stdio.h],         [], [AC_MSG_ERROR([stdio.h not found])])
AC_CHECK_HEADERS([string.h],        [], [AC_MSG_ERROR([string.h not found])])
AC_CHECK_HEADERS([errno.h],         [], [AC_MSG_ERROR([errno.h not found])])
AC_CHECK_HEADERS([signal.h],        [], [AC_MSG_ERROR([signal.h not found])])
AC_CHECK_HEADERS([sys/time.h],      [], [AC_MSG_ERROR([sys/time.h not found])])
AC_CHECK_HEADERS([time.h],          [], [AC_MSG_ERROR([time.h not found])])
AC_CHECK_HEADERS([sys/mman.h],      [], [AC_MSG_ERROR([sys/mman.h not found])])

# Check for functions (if needed)
# Example: AC_CHECK_FUNCS([function_name], [action-if-found], [action-if-not-found])
AC_CHECK_FUNCS([pthread_create],    [], [AC_MSG_ERROR([pthread_create not found])])
AC_CHECK_FUNCS([pthread_join],      [], [AC_MSG_ERROR([pthread_join not found])])
AC_CHECK_FUNCS([pthread_detach],    [], [AC_MSG_ERROR([pthread_detach not found])])
AC_CHECK_FUNCS([pthread_cancel],    [], [AC_MSG_ERROR([pthread_cancel not found])])
AC_CHECK_FUNCS([epoll_create],      [], [AC_MSG_ERROR([epoll_create not found])])
AC_CHECK_FUNCS([epoll_ctl],         [], [AC_MSG_ERROR([epoll_ctl not found])])
AC_CHECK_FUNCS([epoll_wait],        [], [AC_MSG_ERROR([epoll_wait not found])])
AC_CHECK_FUNCS([fcntl],             [], [AC_MSG_ERROR([fcntl not found])])
AC_CHECK_FUNCS([socket],            [], [AC_MSG_ERROR([socket not found])])
AC_CHECK_FUNCS([bind],              [], [AC_MSG_ERROR([bind not found])])
AC_CHECK_FUNCS([listen],            [], [AC_MSG_ERROR([listen not found])])
AC_CHECK_FUNCS([accept],            [], [AC_MSG_ERROR([accept not found])])
AC_CHECK_FUNCS([close],             [], [AC_MSG_ERROR([close not found])])
AC_CHECK_FUNCS([read],              [], [AC_MSG_ERROR([read not found])])
AC_CHECK_FUNCS([write],             [], [AC_MSG_ERROR([write not found])])
AC_CHECK_FUNCS([recv],              [], [AC_MSG_ERROR([recv not found])])
AC_CHECK_FUNCS([mmap],              [], [AC_MSG_ERROR([mmap not found])])

# Check for declarations (if needed)
# Example: AC_CHECK_DECLS([type_name], [action-if-found], [action-if-not-found])

# Tolerant macro declarations
AC_CHECK_DECLS([NULL],      [RX_HAVE_NULLPTR=1],    [])

# Check for type definitions
# Example: AC_CHECK_TYPES([type_name], [action-if-found], [action-if-not-found])
AC_CHECK_TYPES([pthread_t], [RX_HAVE_PTHREAD_T=1],  [AC_MSG_ERROR([pthread_t type not found])])
AC_CHECK_TYPES([int8_t],    [RX_HAVE_INT8_T=1],     [])
AC_CHECK_TYPES([ssize_t],   [RX_HAVE_SSIZE_T=1],    [])
AC_CHECK_TYPES([size_t],    [RX_HAVE_SIZE_T=1],     [])
AC_CHECK_TYPES([u_char],    [RX_HAVE_U_CHAR=1],     [])


# Generate Makefiles for subdirectories
AC_CONFIG_FILES([
    Makefile 
    src/Makefile
    lib/Makefile
    lib/unity/Makefile
    test/Makefile
])

# Output the configuration summary
COMMON_CFLAGS="-Werror -std=c99"
AC_SUBST(COMMON_CFLAGS)

AC_OUTPUT
