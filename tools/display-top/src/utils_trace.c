#include "utils.h"
#include "display.h"

void strip_whitespace(char *str)
{
    char *start = str;
    char *end;
    
    while (isspace((unsigned char)*start))
    start++;
    
    if (*start == 0)
    {
        *str = 0;
        return;
    }
    
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end))
    end--;
    
    *(end + 1) = 0;
    
    if (start != str)
    memmove(str, start, end - start + 2);
}


void EnsureTracingOn()
{
    FILE *fp;

    fp = fopen("/sys/kernel/debug/tracing/events/i915/i915_reg_rw/enable", "w");
    if (fp == NULL || fprintf(fp, "1") < 0 || fclose(fp) != 0)
    {
        log_message(LOG_ERROR, "Error enabling i915_reg_rw event\n");
    }

    fp = fopen("/sys/kernel/debug/tracing/current_tracer", "w");
    if (fp == NULL || fprintf(fp, "nop") < 0 || fclose(fp) != 0)
    {
        log_message(LOG_ERROR, "Error setting current tracer to nop\n");
    }

    fp = fopen("/sys/kernel/debug/tracing/tracing_on", "w");
    if (fp == NULL || fprintf(fp, "1") < 0 || fclose(fp) != 0)
    {
        log_message(LOG_ERROR, "Error enabling tracing\n");
    }

    log_message(LOG_INFO, "Tracing enabled\n");
}

RegisterCacheEntry *register_cache = NULL;

void add_to_cache(uint32_t reg_addr, const char *name)
{
    RegisterCacheEntry *entry = malloc(sizeof(RegisterCacheEntry));
    if (!entry)
        return;

    entry->reg_addr = reg_addr;
    strncpy(entry->name, name, sizeof(entry->name) - 1);
    entry->name[sizeof(entry->name) - 1] = '\0';

    HASH_ADD_INT(register_cache, reg_addr, entry);
}

const char *lookup_cache(uint32_t reg_addr)
{
    RegisterCacheEntry *entry = NULL;
    HASH_FIND_INT(register_cache, &reg_addr, entry);
    return entry ? entry->name : NULL;
}

const char *getRegisterNameFromJson(uint32_t reg_addr, cJSON *json)
{
    if (!json)
        return "-";

    const char *cached_name = lookup_cache(reg_addr);
    if (cached_name)
        return cached_name;

    cJSON *child = NULL;
    cJSON_ArrayForEach(child, json)
    {
        cJSON *address = cJSON_GetObjectItem(child, "Address");
        if (address && cJSON_IsString(address))
        {
            uint32_t json_addr = (uint32_t)strtol(address->valuestring, NULL, 16);
            if (json_addr == reg_addr)
            {
                cJSON *name = cJSON_GetObjectItem(child, "Name");
                if (name && cJSON_IsString(name))
                {
                    add_to_cache(reg_addr, name->valuestring);
                    return name->valuestring;
                }
            }
        }
    }

    add_to_cache(reg_addr, "-");
    return "-";
}

void free_cache()
{
    RegisterCacheEntry *entry, *tmp;
    HASH_ITER(hh, register_cache, entry, tmp)
    {
        HASH_DEL(register_cache, entry);
        free(entry);
    }
}

void ensureDumpDirectory() {

    struct stat st;

    // Check if the directory exists
    if (stat(DUMP_DIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        log_message(LOG_INFO, "Directory %s already exists.\n", DUMP_DIR);
        return;
    }

    // Try to create the directory
    if (mkdir(DUMP_DIR, 0755) == 0) {
        log_message(LOG_INFO, "Directory %s created successfully.\n", DUMP_DIR);
    } else {
        log_message(LOG_ERROR, "mkdir failed: %s\n", strerror(errno));
    }
}