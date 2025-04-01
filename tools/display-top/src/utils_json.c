#include "utils.h"
#include <limits.h>

/**
 * @brief Constructs the full file path for a given filename relative to the project's data directory.
 *
 * This function determines the executable's directory, navigates to the project root,
 * and appends the given filename to the "data" directory within the project root.
 *
 * @param filename The name of the file for which the full path is to be constructed.
 * @return A pointer to a static buffer containing the full file path, or NULL if an error occurs.
 */
char *get_file_path(const char *filename)
{
    static char file_path[PATH_MAX];
    char exe_path[PATH_MAX];

    if (realpath("/proc/self/exe", exe_path) == NULL)
    {
        perror("realpath() error");
        return NULL;
    }

    char exe_path_copy[PATH_MAX];
    strncpy(exe_path_copy, exe_path, PATH_MAX - 1);
    exe_path_copy[PATH_MAX - 1] = '\0';

    char *dir = dirname(exe_path_copy);

    if (snprintf(file_path, sizeof(file_path), "%s/../data/%s", dir, filename) >= (int)sizeof(file_path))
    {
        fprintf(stderr, "Path truncated! Increase buffer size.\n");
    }

    return file_path;
}

/**
 * @brief Extracts and formats valid value strings from a cJSON object.
 *
 * This function processes a cJSON object to extract valid value strings
 * and formats them into a single string. The input cJSON object can be
 * either an array or an object. If the input is an array, the function
 * iterates through each item in the array, extracts the "Name" and "Value"
 * attributes, and appends them to the result string in the format "Name: Value\n".
 * If the input is an object, it directly extracts the "Name" and "Value"
 * attributes and formats them similarly.
 *
 * @param validValue A pointer to a cJSON object containing valid values.
 *                   The object can be either an array or an object with
 *                   "@attributes" containing "Name" and "Value".
 * @return A pointer to a dynamically allocated string containing the
 *         formatted valid values. The caller is responsible for freeing
 *         the allocated memory. Returns NULL if memory allocation fails
 *         or if the input is invalid.
 */
char *getValidValueString(cJSON *validValue)
{
    if (!validValue)
        return NULL;

    size_t buffer_size = 1024;
    char *valid_value_buffer = (char *)malloc(buffer_size);
    if (!valid_value_buffer)
    {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    valid_value_buffer[0] = '\0';
    size_t current_length = 0;

    if (cJSON_IsArray(validValue))
    {
        int valid_value_count = cJSON_GetArraySize(validValue);
        for (int i = 0; i < valid_value_count; i++)
        {
            cJSON *item = cJSON_GetArrayItem(validValue, i);
            if (item)
            {
                cJSON *attrs = cJSON_GetObjectItem(item, "@attributes");
                if (attrs)
                {
                    cJSON *value = cJSON_GetObjectItem(attrs, "Value");
                    cJSON *name = cJSON_GetObjectItem(attrs, "Name");

                    if (value && value->valuestring && name && name->valuestring)
                    {
                        size_t needed = current_length + strlen(name->valuestring) + strlen(value->valuestring) + 5;
                        if (needed >= buffer_size)
                        {
                            char *new_buffer = (char *)realloc(valid_value_buffer, needed + 1);
                            if (!new_buffer)
                            {
                                printf("Memory reallocation failed!\n");
                                free(valid_value_buffer);
                                return NULL;
                            }
                            valid_value_buffer = new_buffer;
                            buffer_size = needed + 1;
                        }

                        current_length += snprintf(valid_value_buffer + current_length, buffer_size - current_length, "%s: %s, ", name->valuestring, value->valuestring);
                    }
                }
            }
        }
    }
    else if (cJSON_IsObject(validValue))
    {
        cJSON *valid_value_attr = cJSON_GetObjectItem(validValue, "@attributes");
        if (valid_value_attr)
        {
            cJSON *value = cJSON_GetObjectItem(valid_value_attr, "Value");
            cJSON *name = cJSON_GetObjectItem(valid_value_attr, "Name");

            if (value && value->valuestring && name && name->valuestring)
            {
                size_t needed = strlen(name->valuestring) + strlen(value->valuestring) + 5;
                if (needed >= buffer_size)
                {
                    char *new_buffer = (char *)realloc(valid_value_buffer, needed + 1);
                    if (!new_buffer)
                    {
                        printf("Memory reallocation failed!\n");
                        free(valid_value_buffer);
                        return NULL;
                    }
                    valid_value_buffer = new_buffer;
                    buffer_size = needed + 1;
                }

                snprintf(valid_value_buffer, buffer_size, "%s: %s, ", name->valuestring, value->valuestring);
            }
        }
    }

    return valid_value_buffer;
}

/**
 * @brief Extracts and concatenates text from a cJSON object representing a description.
 *
 * This function processes a cJSON object to extract text from the "#text" field and
 * from each element of the "p" array, concatenating them into a single string.
 * Each extracted text is followed by a newline character.
 *
 * @param Description A pointer to a cJSON object containing the description data.
 *                    The object is expected to have a "#text" field and/or a "p" array.
 *
 * @return A dynamically allocated string containing the concatenated description text.
 *         If the Description object is NULL, or if memory allocation fails, or if no
 *         text is found, the function returns NULL. The caller is responsible for
 *         freeing the returned string.
 */
char *getDescriptionString(cJSON *Description)
{
    if (!Description)
        return NULL;

    size_t buffer_size = 128;
    char *description_buffer = (char *)malloc(buffer_size);
    if (!description_buffer)
    {
        printf("Memory allocation error.\n");
        return NULL;
    }
    description_buffer[0] = '\0';
    size_t current_length = 0;

    cJSON *text = cJSON_GetObjectItem(Description, "#text");
    if (text && cJSON_IsString(text))
    {
        const char *text_value = cJSON_GetStringValue(text);
        if (text_value)
        {
            size_t needed = current_length + strlen(text_value) + 2;
            if (needed >= buffer_size)
            {
                char *temp = realloc(description_buffer, needed);
                if (!temp)
                {
                    printf("Memory allocation error.\n");
                    free(description_buffer);
                    return NULL;
                }
                description_buffer = temp;
                buffer_size = needed;
            }
            strcat(description_buffer, text_value);
            current_length = strlen(description_buffer);
        }
    }

    cJSON *p = cJSON_GetObjectItem(Description, "p");
    if (p && cJSON_IsArray(p))
    {
        cJSON *p_item;
        cJSON_ArrayForEach(p_item, p)
        {
            cJSON *p_text = cJSON_GetObjectItem(p_item, "#text");
            if (p_text && cJSON_IsString(p_text))
            {
                const char *p_text_value = cJSON_GetStringValue(p_text);
                if (p_text_value)
                {
                    size_t needed = current_length + strlen(p_text_value) + 2;
                    if (needed >= buffer_size)
                    {
                        char *temp = realloc(description_buffer, needed);
                        if (!temp)
                        {
                            printf("Memory allocation error.\n");
                            free(description_buffer);
                            return NULL;
                        }
                        description_buffer = temp;
                        buffer_size = needed;
                    }
                    strcat(description_buffer, p_text_value);
                    current_length = strlen(description_buffer);
                }
            }
        }
    }

    if (description_buffer[0] == '\0')
    {
        free(description_buffer);
        return NULL;
    }

    return description_buffer;
}

uint64_t hex_string_to_address(const char *hex_str)
{
    uint64_t address;

    if (hex_str[strlen(hex_str) - 1] == 'h' || hex_str[strlen(hex_str) - 1] == 'H')
    {
        char temp[strlen(hex_str)];
        strncpy(temp, hex_str, strlen(hex_str) - 1);
        temp[strlen(hex_str) - 1] = '\0';
        sscanf(temp, "%lx", &address);
    }
    else
    {
        sscanf(hex_str, "%lx", &address);
    }

    return address;
}

/**
 * Recursively searches for a JSON object by name.
 *
 * @param parent The parent JSON object.
 * @param name The name of the object to find.
 * @param result A pointer to store the found JSON object.
 */
void getJsonObject(cJSON *parent, const char *name, cJSON **result)
{
    if (!parent || !name || !result || *result)
    {
        return;
    }

    cJSON *item = cJSON_GetObjectItem(parent, name);
    if (item)
    {
        *result = item;
        return;
    }

    cJSON *child = NULL;
    cJSON_ArrayForEach(child, parent)
    {
        getJsonObject(child, name, result);
        if (*result)
            return;
    }
}

uint64_t getAddress(cJSON *Address)
{
    uint64_t address = 0;

    if (!Address)
    {
        return 0;
    }

    cJSON *address_item = cJSON_GetArrayItem(Address, 0);
    if (!address_item)
    {
        return 0;
    }

    cJSON *address_json = cJSON_GetObjectItem(address_item, "@Start");
    if (!address_json || !cJSON_IsString(address_json))
    {
        address_json = cJSON_GetObjectItem(address_item, "Start");
    }
    if (!address_json || !cJSON_IsString(address_json))
    {
        address_json = cJSON_GetObjectItem(address_item, "Address");
    }
    if (!address_json || !cJSON_IsString(address_json))
    {
        return 0;
    }

    address = hex_string_to_address(address_json->valuestring);

    return address;
}

/**
 * @brief Pretty prints a cJSON object to a ncurses window pad.
 *
 * This function recursively traverses a cJSON object and prints its contents
 * to a specified ncurses window pad in a human-readable format. It handles
 * different JSON types including objects, arrays, strings, numbers, booleans,
 * and null values.
 *
 * @param pad A pointer to the ncurses window pad where the JSON will be printed.
 * @param json A pointer to the cJSON object to be pretty-printed.
 * @param line A pointer to an integer representing the current line number in the pad.
 * @param depth An integer representing the current depth of the JSON object in the hierarchy.
 */
void printJson(WINDOW *pad, cJSON *json, int *line, int depth)
{
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, json)
    {
        wmove(pad, *line, 0);
        wattron(pad, A_DIM);
        for (int i = 0; i < depth; i++)
        {
            if (i == depth - 1)
            {
                wprintw(pad, "|--");
            }
            else
            {
                wprintw(pad, "|   ");
            }
        }
        wattroff(pad, A_DIM);

        if (cJSON_IsObject(item) || cJSON_IsArray(item))
        {
            wattron(pad, A_BOLD);
            wprintw(pad, "%s:", item->string ? item->string : (cJSON_IsObject(item) ? "Object" : "Array"));
            wattroff(pad, A_BOLD);
            (*line)++;
            printJson(pad, item, line, depth + 1);
        }
        else if (cJSON_IsString(item))
        {
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), "%s: %s", item->string ? item->string : "Value", cJSON_GetStringValue(item));
            int max_x = getmaxx(pad);
            for (size_t i = 0; i < strlen(buffer); i += max_x)
            {
                mvwprintw(pad, (*line)++, 2, "%.*s", max_x, buffer + i);
            }
        }
        else if (cJSON_IsNumber(item))
        {
            wprintw(pad, "%s: %d", item->string ? item->string : "Value", item->valueint);
            (*line)++;
        }
        else if (cJSON_IsBool(item))
        {
            wprintw(pad, "%s: %s", item->string ? item->string : "Value", cJSON_IsTrue(item) ? "true" : "false");
            (*line)++;
        }
        else if (cJSON_IsNull(item))
        {
            wprintw(pad, "%s: null", item->string ? item->string : "Value");
            (*line)++;
        }
    }
}

/**
 * @brief Prints the description from a cJSON object to a specified window pad.
 *
 * This function takes a cJSON object containing a description and prints it to a specified
 * window pad. It starts printing from a given line and continues for a specified size.
 *
 * @param pad A pointer to the WINDOW structure where the description will be printed.
 * @param Description A pointer to the cJSON object containing the description to be printed.
 * @param line A pointer to an integer representing the current line number in the pad.
 * @param start An integer representing the starting position in the description.
 * @param size An integer representing the number of characters to print from the description.
 */
void displayDescription(WINDOW *pad, cJSON *Description, int *line, int start, int size)
{
    if (!Description || !line || !pad)
    {
        return;
    }

    cJSON *text = cJSON_GetObjectItem(Description, "#text");
    if (text && cJSON_IsString(text))
    {
        print_wrapped_text(pad, line, start, size, cJSON_GetStringValue(text), true);
    }

    cJSON *p = cJSON_GetObjectItem(Description, "p");
    if (p && cJSON_IsArray(p))
    {
        cJSON *p_item = NULL;
        cJSON_ArrayForEach(p_item, p)
        {
            cJSON *p_text = cJSON_GetObjectItem(p_item, "#text");
            if (p_text && cJSON_IsString(p_text))
            {
                print_wrapped_text(pad, line, start, size, cJSON_GetStringValue(p_text), false);
            }
        }
    }
}

char *format_bits_from_register(uint32_t value, const char *bits)
{
    int start = 0;
    int end = 0;

    if (strchr(bits, ':') == NULL)
    {
        if (sscanf(bits, "%d", &end) != 1)
            return "NA";
        start = end;
    }
    else
    {
        if (sscanf(bits, "%d:%d", &start, &end) != 2)
            return "NA";
    }

    if (start > end)
    {
        int temp = start;
        start = end;
        end = temp;
    }

    int length = end - start + 1;

    if (length == 1)
    {
        char *result = (char *)malloc(3);
        if (result == NULL)
        {
            return "NA";
        }
        result[0] = (value & (1 << start)) ? '1' : '0';
        result[1] = 'b';
        result[2] = '\0';
        return result;
    }
    char *result = (char *)malloc(length + 2);
    if (result == NULL)
    {
        return "NA";
    }

    for (int i = 0; i < length; i++)
    {
        result[i] = (value & (1 << (end - i))) ? '1' : '0';
    }

    result[length] = 'b';
    result[length + 1] = '\0';
    return result;
}

char *get_mmio_path(Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    char buffer[1024];
    buffer[0] = '\0';

    Node *current = node;
    while (current)
    {
        if (strcmp(current->name, "MMIO Registers") == 0)
        {
            break;
        }

        char temp[1024];
        size_t written = snprintf(temp, sizeof(temp), "%s%s%s", current->name, buffer[0] ? " > " : "", buffer);
        if (written >= sizeof(temp))
        {
            log_message(LOG_WARNING, "Truncated output in temp buffer");
        }

        temp[sizeof(temp) - 1] = '\0';
        strncpy(buffer, temp, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        current = current->parent;
    }

    char *path = (char *)malloc(strlen(buffer) + 1);
    if (path)
    {
        strcpy(path, buffer);
    }

    return path;
}

off_t get_mmio_base()
{
    int fd = open("/dev/dri/card0", O_RDWR);
    if (fd < 0)
    {
        log_message(LOG_ERROR, "Failed to open DRM device");
        return -1;
    }

    struct drm_i915_getparam gp = {0};
    int mmio_base;

    gp.param = I915_PARAM_MMAP_VERSION;
    gp.value = &mmio_base;

    if (ioctl(fd, DRM_IOCTL_I915_GETPARAM, &gp) < 0)
    {
        log_message(LOG_ERROR, "Failed to get MMIO base address");
        close(fd);
        return -1;
    }

    close(fd);
    return mmio_base;
}

char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        log_message(LOG_ERROR, "File opening failed");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = (char *)malloc(length + 1);
    if (data)
    {
        fread(data, 1, length, file);
        data[length] = '\0';
    }

    fclose(file);
    return data;
}

uint32_t read_mmio_register(uint64_t address)
{
    int mem_fd;
    void *mmio;
    uint32_t value;

    mem_fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if (mem_fd == -1)
    {
        log_message(LOG_ERROR, "Error opening /dev/mem");
        return 0;
    }

    mmio = mmap(NULL, sizeof(uint32_t), PROT_READ, MAP_SHARED, mem_fd, address & ~((uint64_t)0xFFF));
    if (mmio == MAP_FAILED)
    {
        log_message(LOG_ERROR, "Error mapping memory");
        close(mem_fd);
        return 0;
    }

    value = *((volatile uint32_t *)(mmio + (address & 0xFFF)));

    munmap(mmio, sizeof(uint32_t));
    close(mem_fd);

    return value;
}

uint32_t read_register_value(off_t register_offset)
{
    struct pci_access *pacc;
    struct pci_dev *dev = NULL;
    int fd;
    void *map_base;
    volatile unsigned int *reg_addr;
    unsigned int reg_value = 0;

    pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);

    for (struct pci_dev *d = pacc->devices; d; d = d->next)
    {
        pci_fill_info(d, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);
        log_message(LOG_INFO, "Device: %04x:%04x, Class: 0x%06x, Base Addr: 0x%lx",
                    d->vendor_id, d->device_id, d->device_class, d->base_addr[0]);

        if ((d->vendor_id == 0x8086) && ((d->device_class & 0xFF00) == 0x0300))
        {
            dev = d;
            break;
        }
    }

    if (!dev)
    {
        log_message(LOG_ERROR, "Intel VGA device not found");
        pci_cleanup(pacc);
        return 0;
    }

    off_t mmio_base = dev->base_addr[0] & PCI_ADDR_MEM_MASK;
    mmio_base &= ~0xFFF;

    fd = open("/dev/mem", O_RDONLY);
    if (fd == -1)
    {
        log_message(LOG_ERROR, "Failed to open /dev/mem. Try running as root.");
        pci_cleanup(pacc);
        return 0;
    }

    map_base = mmap(NULL, 0x1000, PROT_READ, MAP_SHARED, fd, mmio_base);
    if (map_base == MAP_FAILED)
    {
        log_message(LOG_ERROR, "Failed to map MMIO region");
        close(fd);
        pci_cleanup(pacc);
        return 0;
    }

    size_t page_size = sysconf(_SC_PAGE_SIZE);
    size_t mmap_size = ((register_offset / page_size) + 1) * page_size;

    map_base = mmap(NULL, mmap_size, PROT_READ, MAP_SHARED, fd, mmio_base);
    if (map_base == MAP_FAILED)
    {
        log_message(LOG_ERROR, "Failed to map MMIO region");
        close(fd);
        pci_cleanup(pacc);
        return 0;
    }

    reg_addr = (volatile unsigned int *)((char *)map_base + register_offset);
    reg_value = *reg_addr;
    log_message(LOG_INFO, "Register 0x%lx Value: 0x%x", register_offset, reg_value);

    munmap(map_base, 0x1000);
    close(fd);
    pci_cleanup(pacc);

    return reg_value;
}
