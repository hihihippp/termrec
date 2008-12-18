#include "config.h"
#include "export.h"

export void vtcur_dump(vt100 vt);
export void vtcur_resize(vt100 vt, int sx, int sy);
export void vtcur_attach(vt100 vt, WINDOW *win, int dump);
export void init_curses();
