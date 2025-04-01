#include "utils.h"

    void
    print_bold_text(WINDOW *win, int line, int col, const char *text, ...)
{
    va_list args;
    va_start(args, text);

    wattron(win, A_BOLD);
    wmove(win, line, col);
    vw_printw(win, text, args);
    wattroff(win, A_BOLD);

    va_end(args);
}

void print_dim_text(WINDOW *win, int line, int col, const char *text, ...)
{
    va_list args;
    va_start(args, text);

    wattron(win, A_DIM);
    wmove(win, line, col);
    vw_printw(win, text, args);
    wattroff(win, A_DIM);

    va_end(args);
}

void print_red_text(WINDOW *win, int line, int col, const char *text, ...)
{
    va_list args;
    va_start(args, text);

    wattron(win, COLOR_PAIR(3));
    wmove(win, line, col);
    vw_printw(win, text, args);
    wattroff(win, COLOR_PAIR(3));

    va_end(args);
}

void print_green_text(WINDOW *win, int line, int col, const char *text, ...)
{
    va_list args;
    va_start(args, text);

    wattron(win, COLOR_PAIR(2));
    wmove(win, line, col);
    vw_printw(win, text, args);
    wattroff(win, COLOR_PAIR(2));

    va_end(args);
}

void setString(char *dest, const char *src, size_t size)
{
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}

int check_size_change(WINDOW *win, int *height, int *width)
{
    int new_height, new_width;
    getmaxyx(win, new_height, new_width);
    move(1, 25);
    if (new_height != *height || new_width != *width)
    {
        *height = new_height;
        *width = new_width;
        return 1;
    }
    return 0;
}

void print_wrapped_text(WINDOW *pad, int *line, int start, int size, const char *text, bool enclose_with_pipe)
{
    if (!text || !pad || !line || size <= 0)
        return;

    const char *ptr = text;

    while (*ptr)
    {
        int len = ((int)strlen(ptr) > size) ? size - 1 : (int)strlen(ptr);

        /* To ensure truncation doesn't break any words */
        if (len < (int)strlen(ptr) && ptr[len] != ' ' && ptr[len - 1] != ' ')
        {
            int adjust_len = len;
            while (adjust_len > 0 && ptr[adjust_len] != ' ')
                adjust_len--;

            if (adjust_len > 0)
                len = adjust_len;
        }

        if (enclose_with_pipe)
            mvwprintw(pad, (*line)++, start, "| %-*.*s |", size, len, ptr);
        else
            mvwprintw(pad, (*line)++, start, "%-*.*s", size, len, ptr);

        ptr += len;
        while (*ptr == ' ')
            ptr++;
    }
}
