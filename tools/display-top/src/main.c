#include <ncurses.h>
#include <menu.h>

#include "node.h"
#include "data.h"
#include "display.h"
#include "populate.h"
#include "utils.h"

int main()
{
    init_log_system();

    initscr();

    cbreak();
    noecho();
    start_color();
    set_escdelay(0);
    keypad(stdscr, TRUE);

    /* init_pair(id, foreground, background) */
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_BLACK, COLOR_GREEN);
    init_pair(5, COLOR_BLACK, COLOR_RED);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);
    init_pair(7, COLOR_BLACK, COLOR_BLUE);

    // run_display_animation();
    open_primary_drm_device();

    populateData();

    displayWin(stdscr, root);

    endwin();

    close_primary_drm_device();
    close_log_system();
    // freeTree(root);
}
