#include "display.h"
#include "utils.h"
#include "data.h"

#define DPCD_SIZE 1 /* 1 byte for register */
#define DEBUGFS_PATH "/dev"

/**
 * @brief Reads DPCD (DisplayPort Configuration Data) registers.
 *
 * This function reads a specified number of bytes from the DPCD registers
 * starting at a given offset and stores the data in the provided buffer.
 *
 * @param aux_path The path to the AUX device file.
 * @param buffer A pointer to the buffer where the read data will be stored.
 * @param size The number of bytes to read from the DPCD registers.
 * @param offset The starting offset in the DPCD registers from which to begin reading.
 * @return An integer indicating the success or failure of the read operation.
 *         Typically, 0 indicates success, while a negative value indicates an error.
 */
int read_dpcd_registers(const char *aux_path, unsigned char *buffer, size_t size, uint64_t offset)
{
    FILE *fp = fopen(aux_path, "rb");
    if (!fp)
    {
        log_message(LOG_ERROR, "Failed to open DPCD AUX path: %s", aux_path);
        return -1;
    }

    if (fseek(fp, offset, SEEK_SET) != 0)
    {
        log_message(LOG_ERROR, "Failed to seek to offset %ld in %s", offset, aux_path);
        fclose(fp);
        return -1;
    }

    size_t bytes_read = fread(buffer, 1, size, fp);
    fclose(fp);

    if (bytes_read != size)
    {
        log_message(LOG_WARNING, "Expected %zu bytes but read %zu bytes", size, bytes_read);
    }

    return (int)bytes_read;
}

/**
 * @brief Displays DPCD (DisplayPort Configuration Data) information on a given pad window.
 *
 * This function takes a pad window, a JSON object representing the DPCD register data,
 * and a pointer to an integer representing the current line number. It displays the
 * DPCD data on the pad window, starting from the specified line.
 *
 * @param pad A pointer to the pad window where the DPCD data will be displayed.
 * @param reg A cJSON object containing the DPCD register data.
 * @param line A pointer to an integer representing the current line number. This will be
 *             updated as the function writes data to the pad window.
 */
void displayDPCDdata(WINDOW *pad, cJSON *reg, int *line)
{
    if (!cJSON_IsObject(reg))
    {
        print_red_text(pad, (*line)++, 1, "Invalid JSON object");
        return;
    }

    cJSON *bitField = cJSON_GetObjectItem(reg, "bitfields");
    if (!bitField)
    {
        print_red_text(pad, (*line)++, 1, "Bitfields not found in Register");
        return;
    }

    print_bold_text(pad, (*line)++, 1, "Bitfield Information: ");
    (*line)++;

    int width = getmaxx(pad); // Removed unused variable 'height'

    width -= 2; /* Padding for the Sides */
    width -= 3; /* Draw the table for the Columns */

    int col_widths[2] = {(int)(width * 0.30f) - 2, (int)(width * 0.70f) - 2};

    if (bitField)
    {
        int bitFieldCount = 0;
        if (cJSON_IsArray(bitField))
            bitFieldCount = cJSON_GetArraySize(bitField);
        else
            bitFieldCount = 1;

        wattron(pad, A_BOLD);
        mvwprintw(pad, (*line)++, 1, "| %-*s | %-*s |", col_widths[0], "BitField", col_widths[1], "Value & Description");
        wattroff(pad, A_BOLD);

        for (int i = 0; i < bitFieldCount; i++)
        {
            cJSON *bitFieldItem = NULL;
            if (cJSON_IsArray(bitField))
                bitFieldItem = cJSON_GetArrayItem(bitField, i);
            else
            {
                bitFieldItem = bitField;
            }
            cJSON *name = cJSON_GetObjectItem(bitFieldItem, "name");
            cJSON *value = cJSON_GetObjectItem(bitFieldItem, "value");

            char *name_buffer = name && cJSON_IsString(name) ? strdup(name->valuestring) : strdup("NA");
            char *value_buffer = value && cJSON_IsString(value) ? strdup(value->valuestring) : strdup("NA");

            mvwprintw(pad, (*line)++, 1, "| %-*s | %-*s |", col_widths[0], name_buffer, col_widths[1], value_buffer);

            free(value_buffer);
            free(name_buffer);
        }
    }
}

/**
 * @brief Displays the DPCD (DisplayPort Configuration Data) information for a given node.
 *
 * This function finds the parent node with "DP" in its name, retrieves the corresponding
 * DPCD AUX path, reads the DPCD JSON file to get the register offset, and then reads
 * and displays the DPCD register data.
 *
 * @param pad The window pad where the information will be displayed.
 * @param node The node for which the DPCD information is to be displayed.
 * @param content_line Pointer to an integer that will be updated with the current line number.
 *
 * The function performs the following steps:
 * 1. Finds the parent node with "DP" in its name.
 * 2. Retrieves the corresponding DPCD AUX path from the DP mappings.
 * 3. Reads the DPCD JSON file to get the register offset for the node.
 * 4. Reads the DPCD register data from the DPCD AUX path.
 * 5. Displays the DPCD register data and additional information on the pad.
 *
 * If any step fails, an appropriate error message is displayed on the pad.
 */
void displayDPCD(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;

    /* find which connector it belongs to */
    Node *parent = node;
    while (parent != NULL && strstr(parent->name, "DP ") == NULL)
    {
        parent = parent->parent;
    }

    /* If no parent with 'DP' in its name is found, display an error and return */
    if (parent == NULL)
    {
        print_red_text(pad, line++, 1, "No parent with 'DP' found");
        *content_line = line;
        return;
    }

    /* Get the DP mappings and count */
    DPMapping *mappings = *get_dp_mapping_storage();
    int count = *get_dp_mapping_count_ptr();

    char dpcd_aux_path[256] = "";

    /* Find the corresponding DP name for the given parent->name */
    for (int i = 0; i < count; i++)
    {
        if (strcmp(mappings[i].card_name, parent->name) == 0)
        {
            snprintf(dpcd_aux_path, sizeof(dpcd_aux_path), "/dev/%s", mappings[i].dp_name);
            break;
        }
    }

    print_bold_text(pad, line++, 1, "DPCD AUX Path: %s\n", dpcd_aux_path);

    /* Fetch the corresponding json information for the node */
    const char dpcd_json_path[] = "./data/dpcd.json";

    char *json_data = read_file(dpcd_json_path);
    if (!json_data)
    {
        print_red_text(pad, line++, 1, "Failed to read DPCD JSON file");
        *content_line = line;
        return;
    }

    cJSON *json = cJSON_Parse(json_data);
    if (!json)
    {
        print_red_text(pad, line++, 1, "Failed to parse DPCD JSON content");
        free(json_data);
        *content_line = line;
        return;
    }
    cJSON *node_item = cJSON_GetObjectItemCaseSensitive(json, node->name);
    if (!node_item)
    {
        cJSON *child = NULL;
        cJSON_ArrayForEach(child, json)
        {
            node_item = cJSON_GetObjectItemCaseSensitive(child, node->name);
            if (node_item)
            {
                break;
            }
        }
    }

    if (!node_item)
    {
        print_red_text(pad, line++, 1, "Node item '%s' not found in JSON", node->name);
        cJSON_Delete(json);
        free(json_data);
        *content_line = line;
        return;
    }

    /* read the offset address for the register */
    uint64_t offset = 0;
    cJSON *Address = cJSON_GetObjectItem(node_item, "address");
    if (Address)
    {
        offset = hex_string_to_address(Address->valuestring);
    }
    else
    {
        print_red_text(pad, line++, 1, "Address not found in Register");
        return;
    }

    line++;
    print_bold_text(pad, line++, 1, "DPCD Register: %s", node->name);
    print_green_text(pad, line++, 1, "DPCD Register Offset: 0x%lx", offset);
    line++;
    unsigned char dpcd_buffer[DPCD_SIZE] = {0};
    if (read_dpcd_registers(dpcd_aux_path, dpcd_buffer, DPCD_SIZE, offset) > 0)
    {
        print_bold_text(pad, line++, 1, "DPCD Data:");
        print_bold_text(pad, line++, 1, "-------------------------------------------------");
        for (int i = 0; i < DPCD_SIZE; i++)
        {
            mvwprintw(pad, line++, 1, "0x%02x (", dpcd_buffer[i]);
            for (int bit = 7; bit >= 0; bit--)
            {
                wprintw(pad, "%d", (dpcd_buffer[i] >> bit) & 1);
            }
            wprintw(pad, ")");
        }
        print_bold_text(pad, line++, 1, "-------------------------------------------------");
    }
    else
    {
        print_red_text(pad, line++, 1, "Failed to read DPCD registers");
    }

    line++;
    displayDPCDdata(pad, node_item, &line);

    *content_line = line; // Removed unused label 'out'
    return;
}
