#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Initialize autoconf
AC_INIT([reactor], [0.0.1])
AC_PREREQ([2.69])

# Check for automake
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])
AM_PROG_AR
AM_PROG_CC_C_O

LT_INIT

# Configure the build process
# AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_INSTALL

# Check for libraries or dependencies (if needed)
# Example: AC_CHECK_LIB([library_name], [function_name], [action-if-found], [action-if-not-found])
AC_CHECK_LIB([pthread], [pthread_create], [], [AC_MSG_ERROR([pthread library not found])])

# Check for header files (if needed)
# Example: AC_CHECK_HEADERS([header_file], [action-if-found], [action-if-not-found])
AC_CHECK_HEADERS([pthread.h], [], [AC_MSG_ERROR([pthread.h not found])])
AC_CHECK_HEADERS([sys/epoll.h], [], [AC_MSG_ERROR([sys/epoll.h not found])])
AC_CHECK_HEADERS([fcntl.h], [], [AC_MSG_ERROR([fcntl.h not found])])
AC_CHECK_HEADERS([unistd.h], [], [AC_MSG_ERROR([unistd.h not found])])
AC_CHECK_HEADERS([sys/socket.h], [], [AC_MSG_ERROR([sys/socket.h not found])])
AC_CHECK_HEADERS([netinet/in.h], [], [AC_MSG_ERROR([netinet/in.h not found])])
AC_CHECK_HEADERS([arpa/inet.h], [], [AC_MSG_ERROR([arpa/inet.h not found])])
AC_CHECK_HEADERS([sys/types.h], [], [AC_MSG_ERROR([sys/types.h not found])])
AC_CHECK_HEADERS([sys/stat.h], [], [AC_MSG_ERROR([sys/stat.h not found])])
AC_CHECK_HEADERS([stdlib.h], [], [AC_MSG_ERROR([stdlib.h not found])])
AC_CHECK_HEADERS([stdio.h], [], [AC_MSG_ERROR([stdio.h not found])])
AC_CHECK_HEADERS([string.h], [], [AC_MSG_ERROR([string.h not found])])
AC_CHECK_HEADERS([errno.h], [], [AC_MSG_ERROR([errno.h not found])])
AC_CHECK_HEADERS([signal.h], [], [AC_MSG_ERROR([signal.h not found])])
AC_CHECK_HEADERS([sys/time.h], [], [AC_MSG_ERROR([sys/time.h not found])])
AC_CHECK_HEADERS([time.h], [], [AC_MSG_ERROR([time.h not found])])
AC_CHECK_HEADERS([sys/mman.h], [], [AC_MSG_ERROR([sys/mman.h not found])])

# Generate Makefiles for subdirectories
AC_CONFIG_FILES([
    Makefile 
    src/Makefile
])

# Output the configuration summary
COMMON_CFLAGS="-Werror -std=c99"
AC_SUBST(COMMON_CFLAGS)

AC_OUTPUT
