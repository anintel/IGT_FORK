#include "data.h"
#include "populate.h"  

#define DEBUGFS_PATH "/dev"
#define DRM_PATH "/sys/class/drm"

char *find_dp_card(const char *dp_aux)
{
    struct dirent *entry;
    DIR *dp = opendir(DRM_PATH);
    if (!dp)
    {
        log_message(LOG_ERROR, "Failed to open DRM path");
        return NULL;
    }

    DPMapping **dp_mappings = get_dp_mapping_storage();
    int *dp_mapping_count = get_dp_mapping_count_ptr();

    char *result = NULL;
    while ((entry = readdir(dp)) != NULL)
    {
        if (strstr(entry->d_name, "card") && strstr(entry->d_name, "-DP"))
        {
            char aux_path[512];
            snprintf(aux_path, sizeof(aux_path), "%s/%s/%s", DRM_PATH, entry->d_name, dp_aux);

            if (access(aux_path, F_OK) == 0)
            {
                *dp_mappings = realloc(*dp_mappings, (*dp_mapping_count + 1) * sizeof(DPMapping));
                if (!*dp_mappings)
                {
                    log_message(LOG_ERROR, "Failed to allocate memory for DP mappings");
                    closedir(dp);
                    return NULL;
                }

                char *dp_name = strstr(entry->d_name, "-DP") + 1;
                char *formatted_name = strdup(dp_name);
                if (!formatted_name)
                {
                    log_message(LOG_ERROR, "Failed to allocate memory for formatted name");
                    closedir(dp);
                    return NULL;
                }

                for (char *p = formatted_name; *p; p++)
                {
                    if (*p == '-')
                    {
                        *p = ' ';
                    }
                }

                (*dp_mappings)[*dp_mapping_count].card_name = strdup(formatted_name);
                (*dp_mappings)[*dp_mapping_count].dp_name = strdup(dp_aux);
                if (!(*dp_mappings)[*dp_mapping_count].card_name || !(*dp_mappings)[*dp_mapping_count].dp_name)
                {
                    log_message(LOG_ERROR, "Failed to allocate memory for DP mapping names");
                    free(formatted_name);
                    closedir(dp);
                    return NULL;
                }
                (*dp_mapping_count)++;

                result = strdup(formatted_name); // Return formatted name like "DP 1", "DP 2"
                free(formatted_name);
                break;
            }
        }
    }
    closedir(dp);
    return result;
}

void processDPCDjsonHelper(cJSON *json, Node *parentNode)
{
    cJSON *reg = NULL;
    cJSON_ArrayForEach(reg, json)
    {
        const char *name = reg->string;
        if (name)
        {
            Node *currentNode = createNode(name, displayDPCD, parentNode);

            // Check if the current JSON entry contains an address
            cJSON *address = cJSON_GetObjectItem(reg, "address");
            if (!address)
            {
                currentNode->displayFunction = NULL;
            }

            // Recursively process if the current JSON entry is an object
            if (cJSON_IsObject(reg) && !address)
            {
                processDPCDjsonHelper(reg, currentNode);
            }
            addChild(parentNode, currentNode);
        }
    }
}

void processDPCDjson(Node *parentNode)
{
    log_message(LOG_INFO, "Opening DPCD json file");
    FILE *file = fopen("./data/dpcd.json", "r");
    if (!file)
    {
        log_message(LOG_ERROR, "Failed to open DPCD json file");
        return;
    }

    log_message(LOG_INFO, "Reading DPCD json file");
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *jsonContent = malloc(fileSize + 1);
    if (!jsonContent)
    {
        log_message(LOG_ERROR, "Failed to allocate memory for json content");
        fclose(file);
        return;
    }

    fread(jsonContent, 1, fileSize, file);
    jsonContent[fileSize] = '\0';
    fclose(file);

    log_message(LOG_INFO, "Parsing DPCD json content");
    cJSON *json = cJSON_Parse(jsonContent);
    if (!json)
    {
        log_message(LOG_ERROR, "Failed to parse DPCD json");
        log_message(LOG_ERROR, cJSON_GetErrorPtr());
        free(jsonContent);
        return;
    }

    processDPCDjsonHelper(json, parentNode);

    log_message(LOG_INFO, "Cleaning up");
    cJSON_Delete(json);
    free(jsonContent);
}

void initializeDPCD()
{
    if (access("./data/dpcd.json", F_OK) != 0)
    {
        log_message(LOG_ERROR, "DPCD json file does not exist");
        return;
    }

    Node *DPCD = createNode("DPCD Registers", NULL, root);
    if (!DPCD)
    {
        log_message(LOG_ERROR, "Failed to create DPCD node");
        log_message(LOG_ERROR, "malloc");
        return;
    }

    static char dpcd_path[256];
    DIR *dir = opendir(DEBUGFS_PATH);
    if (!dir)
    {
        log_message(LOG_ERROR, "Failed to open debugfs dri directory (Run as root?)");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strstr(entry->d_name, "drm_dp") != NULL)
        {
            // Ensure snprintf does not exceed the buffer size
            if (snprintf(dpcd_path, sizeof(dpcd_path), "%s/%s", DEBUGFS_PATH, entry->d_name) >= (int)sizeof(dpcd_path))
            {
                log_message(LOG_ERROR, "DPCD AUX path truncated: %s/%s", DEBUGFS_PATH, entry->d_name);
                continue;
            }

            log_message(LOG_INFO, "Found DPCD AUX path: %s", dpcd_path);

            char *dp_aux = find_dp_card(entry->d_name);
            if (!dp_aux)
            {
                log_message(LOG_ERROR, "Failed to find DP card for %s", entry->d_name);
                continue;
            }

            Node *dpcdNode = createNode(dp_aux, NULL, DPCD);
            if (!dpcdNode)
            {
                log_message(LOG_ERROR, "Failed to create node for %s", dp_aux);
                free(dp_aux);
                closedir(dir);
                return;
            }

            processDPCDjson(dpcdNode);
            addChild(DPCD, dpcdNode);
            free(dp_aux);
        }
    }

    closedir(dir);
    addChild(root, DPCD);
}