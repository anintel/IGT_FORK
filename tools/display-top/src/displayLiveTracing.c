#include "display.h"

void displayLiveTracing(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;
    FILE *trace;
    char buffer[256];

    print_bold_text(pad, line++, 1, "Live Tracing");
    line++;

    trace = popen("cat /sys/kernel/debug/tracing/trace", "r");
    if (!trace)
    {
        log_message(LOG_ERROR, "Failed to open trace file: %s", strerror(errno));
        *content_line = line;
        return;
    }

    int width = getmaxx(pad);

    width -= 2;
    width -= 6;

    int col_widths[7] = {
        (int)(width * 0.10f) - 2,
        (int)(width * 0.05f) - 2,
        (int)(width * 0.12f) - 2,
        (int)(width * 0.06f) - 2,
        (int)(width * 0.10f) - 2,
        (int)(width * 0.17f) - 2,
        (int)(width * 0.40f) - 2};

    const char *json_file_path = MESON_SOURCE_ROOT "/tools/display-top/data/Registers.json";

    char *json_data = read_file(json_file_path);
    if (!json_data)
    {
        log_message(LOG_ERROR, "Error opening JSON file: %s", strerror(errno));
        *content_line = line;
        return;
    }

    cJSON *json = cJSON_Parse(json_data);
    if (!json)
    {
        log_message(LOG_ERROR, "Error parsing JSON: %s", cJSON_GetErrorPtr());
        free(json_data);
        *content_line = line;
        return;
    }

    char *log_lines[1000];
    size_t log_count = 0;

    while (fgets(buffer, sizeof(buffer), trace) != NULL)
    {
        if (strstr(buffer, "i915_reg_rw") == NULL)
            continue;

        char process[32], op[8];
        int pid, cpu, reg, val1, val2, len;
        double timestamp;

        if (sscanf(buffer, "%31[^-]-%d  [%d] %*[^0-9] %lf: i915_reg_rw: %7s reg=0x%x, len=%d, val=(0x%x, 0x%x)",
                   process, &pid, &cpu, &timestamp, op, &reg, &len, &val1, &val2) == 9)
        {
            strip_whitespace(process);
            strip_whitespace(op);

            char value_str[64];
            snprintf(value_str, sizeof(value_str), "(0x%X, 0x%X)", val1, val2);
            const char *reg_name = getRegisterNameFromJson(reg, json);

            char *log_line = malloc(1024);
            snprintf(log_line, 1024, "| %-*s | %-*d | %-*.6f | %-*s | 0x%-*X | %-*s | %-*s |",
                     col_widths[0], process,
                     col_widths[1], cpu,
                     col_widths[2], timestamp,
                     col_widths[3], op,
                     col_widths[4] - 2, reg,
                     col_widths[5], value_str,
                     col_widths[6], reg_name);

            if (log_count < 1000)
            {
                log_lines[log_count++] = log_line;
            }
            else
            {
                free(log_lines[0]);
                memmove(log_lines, log_lines + 1, (999) * sizeof(char *));
                log_lines[999] = log_line;
            }
        }
    }

    wclear(pad);
    line = 0;

    wattron(pad, A_BOLD);
    mvwprintw(pad, line++, 1, "| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |",
              col_widths[0], "Process",
              col_widths[1], "CPU",
              col_widths[2], "Timestamp",
              col_widths[3], "Op",
              col_widths[4], "Reg",
              col_widths[5], "Value",
              col_widths[6], "Name");
    wattroff(pad, A_BOLD);

    for (size_t i = 0; i < log_count; i++)
    {
        mvwprintw(pad, line++, 1, "%s", log_lines[i]);
    }

    wrefresh(pad);

    for (size_t i = 0; i < log_count; i++)
    {
        free(log_lines[i]);
    }

    pclose(trace);
    cJSON_Delete(json);
    free(json_data);

    *content_line = line;
}
