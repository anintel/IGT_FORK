#include <cjson/cJSON.h>
#include <fcntl.h>
#include <stdio.h>
#include <drm/drm.h>
#include <drm/i915_drm.h>
#include <unistd.h>

#include "display.h"
#include "node.h"
#include "utils.h"
#include "utils.h"
#include "utils.h"

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define MMIO_BASE_ADDR 0x00000000 // Example base address (replace with actual base)

/**
 * @brief Displays address information on a given pad window.
 *
 * This function takes a JSON object containing address information and displays it
 * in a formatted manner on the provided pad window. The address information is expected
 * to be in a specific JSON format.
 *
 * @param pad The pad window where the address information will be displayed.
 * @param address_json The JSON object containing the address information.
 * @param line A pointer to the current line number in the pad window.
 *
 * The JSON object should be an array of address items, where each item contains:
 * - "@Start" or "Start" or "Address": The address string.
 * - "Symbol": The symbol associated with the address (optional).
 * - "Projects": The projects associated with the address (optional).
 *
 * The function will display the address information in a tabular format with columns
 * for Address, Symbol, and Projects. If the JSON format is invalid or an error occurs
 * while processing the JSON, an error message will be displayed in red text.
 */
void displayAddress(WINDOW *pad, cJSON *address_json, int *line)
{

    int width = getmaxx(pad);

    width -= 2; /* Padding for the Sides */
    width -= 4; /* Draw the table for the Columns */

    int col_widths[3] = {(int)(width * 0.10f) - 2, (int)(width * 0.25f) - 2, (int)(width * 0.65f) - 2};

    if (!address_json || !cJSON_IsArray(address_json))
    {
        log_message(LOG_ERROR, "Invalid Address JSON format");
        print_red_text(pad, (*line)++, 1, "Invalid Address JSON format");
        return;
    }

    log_message(LOG_INFO, "Displaying Address Information");

    print_bold_text(pad, (*line)++, 1, "Address Information:");
    (*line)++;

    wattron(pad, A_BOLD);
    mvwprintw(pad, (*line)++, 1, "| %-*s | %-*s | %-*s |", col_widths[0], "Address", col_widths[1], "Symbol", col_widths[2], "Projects");
    wattroff(pad, A_BOLD);

    int address_count = cJSON_GetArraySize(address_json);
    for (int i = 0; i < address_count; i++)
    {
        cJSON *address_item = cJSON_GetArrayItem(address_json, i);
        if (!address_item)
        {
            print_red_text(pad, (*line)++, 1, "Failed to get address item");
            return;
        }

        cJSON *address = cJSON_GetObjectItem(address_item, "@Start");
        if (!address || !cJSON_IsString(address))
            address = cJSON_GetObjectItem(address_item, "Start");
        if (!address || !cJSON_IsString(address))
            address = cJSON_GetObjectItem(address_item, "Address");
        if (!address || !cJSON_IsString(address))
        {
            print_red_text(pad, (*line)++, 1, "Invalid address format");
            return;
        }

        cJSON *symbol = cJSON_GetObjectItem(address_item, "Symbol");
        cJSON *projects = cJSON_GetObjectItem(address_item, "Projects");

        char *addr_str = address ? address->valuestring : "N/A";
        char *symbol_str = symbol ? cJSON_GetStringValue(symbol) : "N/A";
        char *projects_str = projects ? cJSON_GetStringValue(projects) : "N/A";

        char *wrapped_addr = malloc(col_widths[0] + 1);
        char *wrapped_symbol = malloc(col_widths[1] + 1);
        char *wrapped_projects = malloc(col_widths[2] + 1);

        int addr_len, symbol_len, projects_len;

        while (*addr_str || *symbol_str || *projects_str)
        {
            addr_len = (int)strlen(addr_str) > col_widths[0] ? col_widths[0] - 1 : (int)strlen(addr_str);
            symbol_len = (int)strlen(symbol_str) > col_widths[1] ? col_widths[1] - 1 : (int)strlen(symbol_str);
            projects_len = (int)strlen(projects_str) > col_widths[2] ? col_widths[2] - 1 : (int)strlen(projects_str);

            memset(wrapped_addr, 0, col_widths[0]);
            memset(wrapped_symbol, 0, col_widths[1]);
            memset(wrapped_projects, 0, col_widths[2]);

            snprintf(wrapped_addr, col_widths[0] + 1, "%-*.*s", col_widths[0], addr_len, addr_str);
            snprintf(wrapped_symbol, col_widths[1] + 1, "%-*.*s", col_widths[1], symbol_len, symbol_str);
            snprintf(wrapped_projects, col_widths[2] + 1, "%-*.*s", col_widths[2], projects_len, projects_str);

            mvwprintw(pad, (*line)++, 1, "| %s | %s | %s |", wrapped_addr, wrapped_symbol, wrapped_projects);

            addr_str += addr_len;
            symbol_str += symbol_len;
            projects_str += projects_len;

            while (*addr_str == ' ')
            {
                addr_str++;
            }
            while (*symbol_str == ' ')
            {
                symbol_str++;
            }
            while (*projects_str == ' ')
            {
                projects_str++;
            }
        }
    }
}

/**
 * @brief Displays a bit group on a given pad window.
 *
 * This function takes a pad window, a cJSON object representing a bit group,
 * and a pointer to an integer representing the current line number. It displays
 * the bit group on the specified pad window and updates the line number accordingly.
 *
 * @param pad A pointer to the WINDOW structure where the bit group will be displayed.
 * @param bitGroup A cJSON object representing the bit group to be displayed.
 * @param line A pointer to an integer representing the current line number. This
 *             will be updated as the bit group is displayed.
 */
void displayBitGroup(WINDOW *pad, cJSON *bitGroup, cJSON *Address, int *line)
{
    unsigned int value = read_register_value(getAddress(Address));

    cJSON *bitField = cJSON_GetObjectItem(bitGroup, "BitField");
    print_bold_text(pad, (*line)++, 1, "BitGroup Information: ");
    (*line)++;

    log_message(LOG_INFO, "Bitgroup display is started");

    int width, height;
    getmaxyx(pad, height, width);

    width -= 2; /* Padding for the Sides */
    width -= 6; /* Draw the table for the Columns */

    int col_widths[5] = {(int)(width * 0.10f) - 2, (int)(width * 0.15f) - 2, (int)(width * 0.15f) - 2, (int)(width * 0.50f) - 2, (int)(width * 0.10f) - 2};

    if (bitField)
    {
        int bitFieldCount = 0;
        if (cJSON_IsArray(bitField))
            bitFieldCount = cJSON_GetArraySize(bitField);
        else
            bitFieldCount = 1;

        wattron(pad, A_BOLD);
        mvwprintw(pad, (*line)++, 1, "| %-*s | %-*s | %-*s | %-*s | %-*s |", col_widths[0], "Bits", col_widths[1], "Value", col_widths[2], "Valid Value", col_widths[3], "Name & Description", col_widths[4], "Access");
        wattroff(pad, A_BOLD);

        int desc_line = 0;

        for (int i = 0; i < bitFieldCount; i++)
        {
            cJSON *bitFieldItem = NULL;
            if (cJSON_IsArray(bitField))
                bitFieldItem = cJSON_GetArrayItem(bitField, i);
            else
                bitFieldItem = bitField;

            cJSON *attributes = cJSON_GetObjectItem(bitFieldItem, "@attributes");

            cJSON *ValidValue = cJSON_GetObjectItem(bitFieldItem, "ValidValue");
            char *valid_value_buffer = ValidValue && getValidValueString(ValidValue) ? strdup(getValidValueString(ValidValue)) : strdup("NA");

            cJSON *Description = cJSON_GetObjectItem(bitFieldItem, "Description");
            char *description_buffer = Description && getDescriptionString(Description) ? strdup(getDescriptionString(Description)) : strdup("NA");
            int descline = 0;

            if (attributes)
            {
                cJSON *Bits = cJSON_GetObjectItem(attributes, "Bits");
                cJSON *Name = cJSON_GetObjectItem(attributes, "Name");
                cJSON *Access = cJSON_GetObjectItem(attributes, "Access");

                char *bits_buffer = NULL, *name_buffer = NULL, *access_buffer = NULL, *value_buffer = NULL;

                bits_buffer = (Bits && Bits->valuestring) ? strdup(Bits->valuestring) : strdup("NA");
                name_buffer = (Name && Name->valuestring) ? strdup(Name->valuestring) : strdup("NA");
                access_buffer = (Access && Access->valuestring) ? strdup(Access->valuestring) : strdup("NA");

                if (Bits && Bits->valuestring)
                {
                    char *formatted_value = format_bits_from_register(value, bits_buffer);
                    value_buffer = formatted_value ? strdup(formatted_value) : strdup("NA");
                }
                else
                {
                    value_buffer = strdup("NA");
                }

                char *orig_bits_buffer = bits_buffer;
                char *orig_value_buffer = value_buffer;
                char *orig_valid_value_buffer = valid_value_buffer;
                char *orig_name_buffer = name_buffer;
                char *orig_access_buffer = access_buffer;

                // Allocate dynamically to avoid stack overflow
                char *wrapped_bits = malloc(col_widths[0] + 1);
                char *wrapped_value = malloc(col_widths[1] + 1);
                char *wrapped_valid_value = malloc(col_widths[2] + 1);
                char *wrapped_name = malloc(col_widths[3] + 1);
                char *wrapped_access = malloc(col_widths[4] + 1);

                int bits_len, value_len, valid_value_len, name_len, access_len;

                while (*bits_buffer || *name_buffer || *access_buffer || *valid_value_buffer)
                {
                    bits_len = (int)strlen(bits_buffer) > col_widths[0] ? col_widths[0] - 1 : (int)strlen(bits_buffer);
                    value_len = (int)strlen(value_buffer) > col_widths[1] ? col_widths[1] - 1 : (int)strlen(value_buffer);
                    valid_value_len = (int)strlen(valid_value_buffer) > col_widths[2] ? col_widths[2] - 1 : (int)strlen(valid_value_buffer);
                    name_len = (int)strlen(name_buffer) > col_widths[3] ? col_widths[3] - 1 : (int)strlen(name_buffer);
                    access_len = (int)strlen(access_buffer) > col_widths[4] ? col_widths[4] - 1 : (int)strlen(access_buffer);

                    memset(wrapped_bits, 0, col_widths[0]);
                    memset(wrapped_value, 0, col_widths[1]);
                    memset(wrapped_valid_value, 0, col_widths[2]);
                    memset(wrapped_name, 0, col_widths[3]);
                    memset(wrapped_access, 0, col_widths[4]);

                    snprintf(wrapped_bits, col_widths[0] + 1, "%-*.*s", col_widths[0], bits_len, bits_buffer);
                    snprintf(wrapped_value, col_widths[1] + 1, "%-*.*s", col_widths[1], value_len, value_buffer);
                    snprintf(wrapped_valid_value, col_widths[2] + 1, "%-*.*s", col_widths[2], valid_value_len, valid_value_buffer);
                    snprintf(wrapped_name, col_widths[3] + 1, "%-*.*s", col_widths[3], name_len, name_buffer);
                    snprintf(wrapped_access, col_widths[4] + 1, "%-*.*s", col_widths[4], access_len, access_buffer);

                    mvwprintw(pad, (*line)++, 1, "| %s | %s | %s | %s | %s |", wrapped_bits, wrapped_value, wrapped_valid_value, wrapped_name, wrapped_access);

                    // Move the pointer forward safely
                    bits_buffer += bits_len;
                    value_buffer += value_len;
                    valid_value_buffer += valid_value_len;
                    name_buffer += name_len;
                    access_buffer += access_len;

                    // Skip spaces safely
                    while (*bits_buffer == ' ')
                        bits_buffer++;
                    while (*value_buffer == ' ')
                        value_buffer++;
                    while (*valid_value_buffer == ' ')
                        valid_value_buffer++;
                    while (*name_buffer == ' ')
                        name_buffer++;
                    while (*access_buffer == ' ')
                        access_buffer++;

                    if (!*name_buffer && desc_line == 0)
                        desc_line = *line;
                }

                if (Description)
                {
                    wattron(pad, A_DIM);
                    print_wrapped_text(pad, &desc_line, (1 + col_widths[0] + 2 + col_widths[1] + 3 + col_widths[2] + 4), col_widths[3], description_buffer, true);
                    wattroff(pad, A_DIM);
                }

                // Free the original dynamically allocated buffers
                free(orig_bits_buffer);
                free(orig_value_buffer);
                free(orig_valid_value_buffer);
                free(orig_name_buffer);
                free(orig_access_buffer);

                if (desc_line > *line)
                {
                    *line = desc_line;
                }
                desc_line = 0;

                (*line)++;
            }
            else
            {
                print_red_text(pad, (*line)++, 2, "BitField info not found in Register");
                log_message(LOG_ERROR, "BitField info not found in Register");
            }
        }
    }
}

void displayMMIO(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;

    char json_file_path[] = "./data/local/Singleson.json";
    char *json_data = read_file(json_file_path);
    if (json_data == NULL)
    {
        print_red_text(pad, line++, 1, "Error opening JSON file: %s", strerror(errno));
        *content_line = line;
        return;
    }

    cJSON *json = cJSON_Parse(json_data);
    if (!json)
    {
        print_red_text(pad, line++, 1, "Error parsing JSON: %s", cJSON_GetErrorPtr());
        free(json_data);
        *content_line = line;
        return;
    }

    const char *path = get_mmio_path(node);
    if (!path)
    {
        print_red_text(pad, line++, 1, "Error getting MMIO path");
        cJSON_Delete(json);
        free(json_data);
        *content_line = line;
        return;
    }

    char *pathCopy = strdup(path);
    if (!pathCopy)
    {
        print_red_text(pad, line++, 1, "Memory allocation error.");
        cJSON_Delete(json);
        free(json_data);
        *content_line = line;
        return;
    }

    char *token = strtok(pathCopy, " > ");
    cJSON *reg = json;
    cJSON *Address = NULL;
    cJSON *BitGroup = NULL;

    while (token)
    {
        if (!reg)
        {
            print_red_text(pad, line++, 1, "Register not found for token: %s", token);
            free(pathCopy);
            cJSON_Delete(json);
            free(json_data);
            *content_line = line;
            return;
        }

        cJSON *content = cJSON_GetObjectItem(reg, "content");
        if (!content)
        {
            print_red_text(pad, line++, 1, "Content not found in Register");
            free(pathCopy);
            cJSON_Delete(json);
            free(json_data);
            *content_line = line;
            return;
        }

        reg = content;
        reg = cJSON_GetObjectItem(reg, token);
        if (!reg)
        {
            print_red_text(pad, line++, 1, "Token not found: %s", token);
            break;
        }

        token = strtok(NULL, " > ");
    }

    free(pathCopy);
    if (reg)
    {
        mvwprintw(pad, line++, 1, "Register found: %s", path);
        mvwprintw(pad, line++, 1, "Current JSON object: %s", reg->string ? reg->string : "Unnamed");

        cJSON *content = cJSON_GetObjectItem(reg, "content");
        cJSON *Register = cJSON_GetObjectItem(content, "Register");

        getJsonObject(Register, "Address", &Address);
        getJsonObject(Register, "BitGroup", &BitGroup);
    }
    else
    {
        print_red_text(pad, line++, 1, "Register not found: %s", path);
        cJSON_Delete(json);
        free(json_data);
        *content_line = line;
        return;
    }

    line++;
    unsigned int value = 0;
    if (Address)
    {
        uint64_t address = getAddress(Address); // Fetch the address from JSON
        if (address != 0)
        {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "Fetched Address: 0x%lx", address);
            print_green_text(pad, line++, 1, buffer);

            value = read_register_value(address);
            print_green_text(pad, line++, 1, "Register Value: 0x%x", value);
        }
        else
        {
            print_red_text(pad, line++, 1, "Error: Address not found in JSON.");
        }
    }

    line++;
    if (Address)
        displayAddress(pad, Address, &line);
    else
        print_red_text(pad, line++, 1, "Address is not found");

    line++;

    if (BitGroup)
        displayBitGroup(pad, BitGroup, Address, &line);
    else
        print_red_text(pad, line++, 1, "BitGroup is not found");

    cJSON_Delete(json);
    free(json_data);

    *content_line = line;

    log_message(LOG_INFO, "MMIO Information displayed successfully");
}
