AC_DEFUN([AC_CURSESW], [
AC_MSG_CHECKING([for wide curses])
AC_COMPILE_IFELSE([
#define _XOPEN_SOURCE_EXTENDED
#include <ncursesw/curses.h>

int foo() {add_wch(0);}
  ],
  [
    AC_MSG_RESULT([yes])
    AC_DEFINE([HAVE_CURSESW], [1], [Define if you have working wide-char curses.])
    ac_cv_have_cursesw=yes
  ],
  [
    AC_MSG_RESULT([no])
  ])
])
