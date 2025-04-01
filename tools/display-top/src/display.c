#include "display.h"
#include "data.h"
#include "utils.h"

#define DRM_DEVICE "/dev/dri/card0"

void displayWin(WINDOW *win, Node *node)
{
    Node *head = node;

    int is_running;
    int highlighted_index = 0;

    bool focus_on_menu = true;
    bool refresh = true;

    int win_height, win_width;

    int content_height;
    int content_y_start;

    int mpx;
    int menu_pad_pos;
    int menu_pad_width;
    int menu_pad_height;
    WINDOW *menu_pad = NULL;

    int dpx;
    int display_pad_pos;
    int display_pad_width;
    int display_pad_height;
    WINDOW *display_pad = NULL;

    int count;

    int ch;

    while (1)
    {
        wclear(win);
        box(win, 0, 0);
        wbkgd(win, COLOR_PAIR(2));
        getmaxyx(win, win_height, win_width);

        if (head->displayFunction == NULL && head->childrenSize == 0)
        {
            mvwprintw(win, 4, 2, "It's so Empty at %s :(", head->name);
            mvwprintw(win, 8, 2, "Press 'e' to go back.");
            wrefresh(win);
        }

        is_running = 1;

        log_message(LOG_INFO, "Displaying %s", head->name);

        while (is_running)
        {
            if (check_size_change(win, &win_height, &win_width) == 1 || refresh)
            {
                log_message(LOG_INFO, "Window size changed. Refreshing display.");
                refresh = false;

                content_height = win_height - 6;
                content_y_start = 4;

                mpx = 1;
                menu_pad_pos = 0;
                menu_pad_height = head->childrenSize > content_height ? head->childrenSize + 5 : content_height;
                menu_pad_width = (int)((win_width - 3) * 0.3);

                dpx = menu_pad_width + 2;
                display_pad_pos = 0;
                display_pad_height = 1000;
                display_pad_width = win_width - 3 - menu_pad_width;

                if (menu_pad != NULL)
                {
                    delwin(menu_pad);
                    menu_pad = NULL;
                }

                menu_pad = newpad(menu_pad_height, menu_pad_width);
                // wbkgd(menu_pad, COLOR_PAIR(4));
                prefresh(menu_pad, menu_pad_pos, 0, content_y_start, 1, content_height + content_y_start - 1, menu_pad_width + 1);

                if (head->displayFunction != NULL)
                {
                    if (display_pad != NULL)
                    {
                        delwin(display_pad);
                        display_pad = NULL;
                    }

                    count = display_pad_height;
                    display_pad = newpad(display_pad_height, display_pad_width);
                    // wbkgd(display_pad, COLOR_PAIR(5));

                    // add content to the display pad
                    head->displayFunction(display_pad, head, &count);

                    if (count == 0)
                    {
                        log_message(LOG_INFO, "Exiting %s", head->name);
                        if (head != NULL && head->parent != NULL)
                        {
                            head = head->parent;
                            focus_on_menu = true;
                            highlighted_index = 0;
                            display_pad_pos = 0;
                            is_running = 0;
                        }
                        else
                        {
                            return;
                        }
                        continue;
                    }

                    // adjust display pad size to fit the content only
                    if (count < display_pad_height)
                        display_pad_height = count + 5;

                    // update & refresh the pad
                    prefresh(display_pad, display_pad_pos, 0, content_y_start, dpx, content_height + content_y_start - 1, 2 + menu_pad_width + display_pad_width);
                }

                wclear(win);
                box(win, 0, 0);

                move(1, 2);
                wattron(win, A_BOLD);
                printPath(head);
                wattroff(win, A_BOLD);

                mvwprintw(win, 1, win_width - strlen("[TAB] Switch Focus [S] Search [ESC] Exit [P] Page Dump [D] Recursive Dump [R] Refresh") - 2, "[TAB] Switch Focus [S] Search [ESC] Exit [P] Page Dump [D] Recursive Dump [R] Refresh");
                mvwhline(win, 2, 1, ACS_HLINE, getmaxx(win) - 2);
                mvwvline(win, 3, menu_pad_width + 1, ACS_VLINE, win_height - 4);

                wrefresh(win);
            }

            if (head->displayFunction != NULL)
            {
                if (display_pad_pos > 0)
                    print_bold_text(win, 3, menu_pad_width + 2, "...");
                else
                    mvwprintw(win, 3, menu_pad_width + 2, "   ");

                if (display_pad_pos < display_pad_height - content_height)
                    print_bold_text(win, content_height + content_y_start, menu_pad_width + 2, "...");
                else
                    mvwprintw(win, content_height + content_y_start, menu_pad_width + 2, "   ");

                print_dim_text(win, 3, win_width - 15, "%d lines", display_pad_height);
                prefresh(display_pad, display_pad_pos, 0, content_y_start, dpx, content_height + content_y_start - 1, 2 + menu_pad_width + display_pad_width);
            }

            if (focus_on_menu)
            {
                print_bold_text(win, 3, win_width - 5, "   ");
                wattron(win, A_REVERSE);
                print_bold_text(win, 3, menu_pad_width - 3, "***");
                wattroff(win, A_REVERSE);
            }
            else
            {
                print_bold_text(win, 3, menu_pad_width - 3, "   ");
                wattron(win, A_REVERSE);
                print_bold_text(win, 3, win_width - 5, "***");
                wattroff(win, A_REVERSE);
            }

            if (head->childrenSize > 0)
            {
                if (head->displayFunction == NULL)
                    focus_on_menu = true;
                print_dim_text(win, 3, menu_pad_width - 15, "%d Menus", head->childrenSize);
                for (int i = 0; i < head->childrenSize; ++i)
                {
                    if (highlighted_index == i && focus_on_menu)
                        wattron(menu_pad, A_REVERSE);
                    mvwprintw(menu_pad, i, 1, "%d. %s", i + 1, head->children[i].name);
                    wattroff(menu_pad, A_REVERSE);
                }
            }

            if (highlighted_index == head->childrenSize && focus_on_menu)
                wattron(menu_pad, A_REVERSE);

            if (head->parent != NULL)
                print_dim_text(menu_pad, head->childrenSize, 1, "Go Back");
            else
                print_dim_text(menu_pad, head->childrenSize, 1, "Exit");

            wattroff(menu_pad, A_REVERSE);

            prefresh(menu_pad, menu_pad_pos, 0, content_y_start, mpx, content_height + content_y_start - 1, menu_pad_width + 1);

            ch = wgetch(win);
            switch (ch)
            {
            case KEY_UP:

                if (!focus_on_menu && display_pad_pos > 0)
                    display_pad_pos--;

                else if (focus_on_menu)
                {
                    if (highlighted_index == 0)
                    {
                        highlighted_index = head->childrenSize;
                        menu_pad_pos = menu_pad_height - content_height;
                    }
                    else
                    {
                        highlighted_index--;

                        if (highlighted_index > menu_pad_height - content_height)
                        {
                            menu_pad_pos = menu_pad_height - content_height;
                        }
                        else
                        {
                            menu_pad_pos--;
                        }
                    }
                }
                break;

            case KEY_DOWN:
                if (!focus_on_menu && display_pad_pos < display_pad_height - content_height)
                    display_pad_pos++;

                else if (focus_on_menu)
                {
                    if (highlighted_index == head->childrenSize)
                    {
                        highlighted_index = 0;
                        menu_pad_pos = 0;
                    }
                    else
                    {
                        highlighted_index++;

                        if (highlighted_index > menu_pad_height - content_height)
                        {
                            menu_pad_pos = menu_pad_height - content_height;
                        }
                        else
                        {
                            menu_pad_pos++;
                        }
                    }
                }
                break;

            case 's':
            case 'S':
                log_message(LOG_INFO, "Searching %s", head->name);
                head = displaySearchBar(head);
                refresh = true;
                break;

            case 'p':
            case 'P':
                singleDump(head, "./Dump", true);
                refresh = true;
                break;

            case 'd':
            case 'D':
                displayDumpMenu(head);
                refresh = true;
                break;

            case 27:
                log_message(LOG_INFO, "Exiting %s", head->name);
                if (head != NULL && head->parent != NULL)
                {
                    head = head->parent;
                    focus_on_menu = true;
                    highlighted_index = 0;
                    display_pad_pos = 0;
                    is_running = 0;
                }
                else
                {
                    return;
                }
                break;

            case 'r':
            case 'R':
                log_message(LOG_INFO, "Refreshing %s", head->name);
                refresh = true;
                break;

            case '\n':
                if (focus_on_menu && head != NULL)
                {
                    if (highlighted_index >= 0 && highlighted_index < head->childrenSize)
                    {
                        head = &head->children[highlighted_index];
                        highlighted_index = 0;
                        display_pad_pos = 0;
                        is_running = 0;
                        refresh = true;
                    }
                    else if (highlighted_index == head->childrenSize)
                    {
                        if (head->parent != NULL)
                        {
                            head = head->parent;
                            highlighted_index = 0;
                            display_pad_pos = 0;
                            is_running = 0;
                            refresh = true;
                        }
                        else
                        {
                            return;
                        }
                    }
                }
                break;

            case '\t':
                focus_on_menu = !focus_on_menu;
                break;
            }
        }

        refresh = true;

        if (display_pad != NULL)
        {
            delwin(display_pad);
            display_pad = NULL;
        }
        if (menu_pad != NULL)
        {
            delwin(menu_pad);
            menu_pad = NULL;
        }
    }
}
