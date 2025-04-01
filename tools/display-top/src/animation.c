#include <ncurses.h>
#include <string.h>
#include <unistd.h> // for usleep

// Function to draw large D using ASCII characters
void draw_large_d(WINDOW *win, int start_y, int start_x) {
    const char *letter_d[] = {
        "**** ",
        "*   *",
        "*   *",
        "*   *",
        "**** "
    };
    
    for (int i = 0; i < 5; i++) {
        mvwprintw(win, start_y + i, start_x, "%s", letter_d[i]);
    }
}

void run_display_animation() {
    // Initialize ncurses
    initscr();
    start_color();
    cbreak();
    noecho();
    curs_set(0); // Hide cursor
    timeout(0);  // Non-blocking input
    
    // Initialize color pair
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    
    // Get screen dimensions
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // Create window using full screen size
    WINDOW *win = newwin(max_y, max_x, 0, 0);
    
    // Set colors and draw border
    wbkgd(win, COLOR_PAIR(1));
    wattron(win, COLOR_PAIR(2));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(2));
    
    // Calculate center position for D
    int text_y = (max_y - 5) / 2;  // 5 is height of D
    int text_x = (max_x - 5) / 2;  // 5 is width of D (changed from 7 since we adjusted the design)
    
    // Animation loop
    bool visible = true;
    while (true) {
        // Check for key press
        int ch = getch();
        if (ch != ERR) {  // If a key was pressed
            break;
        }
        
        // Clear window
        wclear(win);
        box(win, 0, 0);
        
        // Draw D when visible
        if (visible) {
            wattron(win, COLOR_PAIR(2));
            draw_large_d(win, text_y, text_x);
            wattroff(win, COLOR_PAIR(2));
        }
        
        wrefresh(win);
        
        // Toggle visibility and wait
        visible = !visible;
        usleep(500000); // 0.5 second delay
    }
    
    // Cleanup
    delwin(win);
    endwin();
}
