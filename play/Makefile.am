bin_PROGRAMS += termplay

termplay_SOURCES = play/termplay.c play/player.h play/player.c
termplay_LDADD = libutils.la libcommon.la libtty.la @PTHREAD_LIBS@
termplay_LDFLAGS = @PTHREAD_CFLAGS@

if HAVE_CURSES
termplay_SOURCES += play/vtcurses.c play/vtcurses.h
termplay_LDADD += -lncursesw
endif
