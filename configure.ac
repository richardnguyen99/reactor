#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Initialize autoconf
AC_INIT([reactor], [0.0.1])
AC_PREREQ([2.69])

# Check for automake
AM_INIT_AUTOMAKE([foreign subdir-objects])

LT_INIT

# Configure the build process
# AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

# Check for libraries or dependencies (if needed)
# Example: AC_CHECK_LIB([library_name], [function_name], [action-if-found], [action-if-not-found])

# Generate Makefiles for subdirectories
AC_CONFIG_FILES([
    Makefile 
    src/Makefile
])

# Output the configuration summary
AC_OUTPUT
