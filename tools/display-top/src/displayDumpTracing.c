#include "display.h"
#include "utils.h"

void displayDumpTracing(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;
    FILE *trace_fp, *dump_fp;
    char buffer[512];

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char dump_file_path[256];
    strftime(dump_file_path, sizeof(dump_file_path), DUMP_DIR "/trace_dump_%Y-%m-%d-%H.%M.%S.txt", t);

    EnsureTracingOn();
    ensureDumpDirectory();

    trace_fp = fopen(TRACE_PATH, "r");
    if (!trace_fp)
    {
        log_message(LOG_ERROR, "Error opening trace file: %s", strerror(errno));
        *content_line = line;
        return;
    }

    dump_fp = fopen(dump_file_path, "w");
    if (!dump_fp)
    {
        log_message(LOG_ERROR, "Error creating dump file: %s", strerror(errno));
        fclose(trace_fp);
        *content_line = line;
        return;
    }
    
    const char *json_file_path = MESON_SOURCE_ROOT "/tools/display-top/data/Registers.json";
    char *json_data = read_file(json_file_path);
    if (!json_data)
    {
        log_message(LOG_ERROR, "Error reading JSON file");
        fclose(trace_fp);
        fclose(dump_fp);
        *content_line = line;
        return;
    }

    cJSON *json = cJSON_Parse(json_data);
    if (!json)
    {
        log_message(LOG_ERROR, "Error parsing JSON");
        free(json_data);
        fclose(trace_fp);
        fclose(dump_fp);
        *content_line = line;
        return;
    }

    int width = getmaxx(pad);
    width -= 8;

    int col_widths[7] = {
        width * 0.10f,
        width * 0.05f,
        width * 0.10f,
        width * 0.08f,
        width * 0.10f,
        width * 0.17f,
        width * 0.40f};

    wclear(pad);
    wattron(pad, A_BOLD);

    mvwprintw(pad, line++, 1, "Ftrace Fromatted & Dumped!");

    int total_lines = 0;
    while (fgets(buffer, sizeof(buffer), trace_fp) != NULL)
    {
        if (strstr(buffer, "i915_reg_rw") != NULL)
        {
            total_lines++;
        }
    }
    rewind(trace_fp);

    // Create a new window for the progress bar
    WINDOW *progress_win = newwin(getmaxy(stdscr), getmaxx(stdscr), 0, 0);
    wbkgd(progress_win, COLOR_PAIR(6));
    wclear(progress_win);
    box(progress_win, 0, 0);
    print_bold_text(progress_win, 1, 1, "DUMPING IS IN PROGRESS :)");
    wrefresh(progress_win);

    size_t log_count = 0;
    int processed_lines = 0;
    while (fgets(buffer, sizeof(buffer), trace_fp) != NULL)
    {
        if (strstr(buffer, "i915_reg_rw") == NULL)
            continue;

        char process[32], op[8];
        int pid, cpu, reg, val1, val2, len;
        double timestamp;

        if (sscanf(buffer, "%31[^-]-%d  [%d] %*s %lf: i915_reg_rw: %7s reg=0x%x, len=%d, val=(0x%x, 0x%x)",
                   process, &pid, &cpu, &timestamp, op, &reg, &len, &val1, &val2) == 9)
        {
            strip_whitespace(process);
            strip_whitespace(op);

            char value_str[64];
            snprintf(value_str, sizeof(value_str), "(0x%X, 0x%X)", val1, val2);
            const char *reg_name = getRegisterNameFromJson(reg, json);
            if (!reg_name)
            {
                reg_name = "Unknown";
            }

            fprintf(dump_fp, "| %-*s | %-*d | %-*.6f | %-*s | 0x%-*X | %-*s | %-*s |\n",
                    col_widths[0], process,
                    col_widths[1], cpu,
                    col_widths[2], timestamp,
                    col_widths[3], op,
                    col_widths[4] - 2, reg,
                    col_widths[5], value_str,
                    col_widths[6], reg_name);

            log_count++;
            processed_lines++;

            // Update progress bar
            float progress = (float)processed_lines / total_lines;
            int bar_width = getmaxx(progress_win) - 14; // Space for brackets & percentage
            int pos = bar_width * progress;

            wattron(progress_win, A_BOLD);
            mvwprintw(progress_win, 3, 2, "[");
            wattroff(progress_win, A_BOLD);

            for (int i = 0; i < bar_width; i++)
            {
                if (i < pos - 1) // Filled progress
                {
                    wattron(progress_win, COLOR_PAIR(7));
                    waddch(progress_win, ' ');
                    wattroff(progress_win, COLOR_PAIR(7));
                }
                else if (i == pos - 1) // Moving arrow indicator
                {
                    wattron(progress_win, COLOR_PAIR(7) | A_BOLD);
                    waddch(progress_win, ' ');
                    wattroff(progress_win, COLOR_PAIR(7) | A_BOLD);
                }
                else // Empty space
                {
                    wattron(progress_win, COLOR_PAIR(2));
                    waddch(progress_win, ' ');
                    wattroff(progress_win, COLOR_PAIR(2));
                }
            }

            wattron(progress_win, A_BOLD);
            waddch(progress_win, ']');
            wattroff(progress_win, A_BOLD);

            // Display percentage
            wattron(progress_win, A_BOLD);
            mvwprintw(progress_win, 3, bar_width + 4, "%3d%%", (int)(progress * 100));
            wattroff(progress_win, A_BOLD);

            wrefresh(progress_win);
        }
    }

    mvwprintw(pad, line + 2, 1, "Dump saved to: %s", dump_file_path);
    *content_line = line;

    fclose(trace_fp);
    fclose(dump_fp);
    cJSON_Delete(json);
    free(json_data);
    delwin(progress_win);
}
