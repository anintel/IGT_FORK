#include <dirent.h>
#include <cjson/cJSON.h>

#include "populate.h"

void processJSONHierarchy(cJSON *json, Node *parentNode)
{
    cJSON *entry = cJSON_GetObjectItem(json, "content");
    if (!entry || !cJSON_IsObject(entry))
    {
        fprintf(stderr, "Invalid JSON structure: missing 'content'.\n");
        return;
    }

    cJSON *child = NULL;
    cJSON_ArrayForEach(child, entry)
    {
        const char *name = child->string;
        cJSON *type = cJSON_GetObjectItem(child, "@type");

        if (!name || !type || !cJSON_IsString(type))
        {
            fprintf(stderr, "Invalid JSON entry: missing name or @type.\n");
            continue;
        }

        Node *currentNode = createNode(name, NULL, parentNode);

        if (strcmp(type->valuestring, "folder") == 0)
        {
            currentNode->displayFunction = NULL;
            processJSONHierarchy(child, currentNode);
            addChild(parentNode, currentNode);
        }
        else if (strcmp(type->valuestring, "file") == 0)
        {
            currentNode->displayFunction = displayMMIO;
            // createAddressNode(currentNode);
            addChild(parentNode, currentNode);
        }
    }
}

void createAddressNode(Node *parentNode)
{
    const char *jsonFilePath = "./data/RegisterReference.json";

    FILE *file = fopen(jsonFilePath, "r");
    if (!file)
    {
        log_message(LOG_ERROR, "Failed to open JSON file: %s.", jsonFilePath);
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *fileContent = (char *)malloc(fileSize + 1);
    if (!fileContent)
    {
        log_message(LOG_ERROR, "Failed to allocate memory for file content");
        fclose(file);
        return;
    }

    fread(fileContent, 1, fileSize, file);
    fileContent[fileSize] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(fileContent);
    if (!json)
    {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        free(fileContent);
        return;
    }

    cJSON *entry = cJSON_GetObjectItem(json, "cpma");
    if (!entry || !cJSON_IsArray(entry))
    {
        fprintf(stderr, "Invalid JSON structure: missing 'cpma'.\n");
        cJSON_Delete(json);
        free(fileContent);
        return;
    }

    cJSON *child = NULL;
    cJSON_ArrayForEach(child, entry)
    {
        cJSON *name = cJSON_GetObjectItem(child, "Name");
        if (name && cJSON_IsString(name) && strcmp(name->valuestring, parentNode->name) == 0)
        {
            Node *currentNode = createNode(name->valuestring, NULL, parentNode);
            currentNode->displayFunction = displayMMIO;
            addChild(parentNode, currentNode);
        }
    }

    cJSON_Delete(json);
    free(fileContent);
}

void initializeMMIO()
{
    log_message(LOG_INFO, "MMIO initialization started");
    Node *MMIO = createNode("MMIO Registers", NULL, root);
    MMIO->displayFunction = displayMMIOSummary;

    const char *jsonFilePath = "./data/local/Singleson.json";

    FILE *file = fopen(jsonFilePath, "r");
    if (!file)
    {
        log_message(LOG_ERROR, "Failed to open the Singleson JSON file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *fileContent = (char *)malloc(fileSize + 1);
    if (!fileContent)
    {
        log_message(LOG_ERROR, "Failed to allocate memory for file content");
        fclose(file);
        return;
    }

    fread(fileContent, 1, fileSize, file);
    fileContent[fileSize] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(fileContent);
    if (!json)
    {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        free(fileContent);
        return;
    }

    processJSONHierarchy(json, MMIO);

    cJSON_Delete(json);
    free(fileContent);

    addChild(root, MMIO);
    log_message(LOG_INFO, "MMIO initialized");
}
