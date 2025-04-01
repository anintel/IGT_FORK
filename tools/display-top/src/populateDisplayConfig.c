#include "populate.h"

int createCrtcNodes(drmModeRes *resources, Node *parentNode, int filter);
int createConnectorNodes(drmModeRes *resources, Node *parentNode, int filter);
int createEncoderNodes(drmModeRes *resources, Node *parentNode, int filter);
int createFramebufferNodes(drmModeRes *resources, Node *parentNode, int filter);
int createPlaneNodes(Node *parentNode, int filter);

/**
 * @file populateDisplayConfig.c
 * @brief This file contains functions for creating DRM resource nodes.
 *
 * The functions in this file iterate through the available DRM resources
 * (CRTCs, connectors, encoders, or framebuffers) and create nodes for each resource.
 * It also creates associated nodes for each resource type.
 *
 * Functions:
 * @brief Creates DRM resource nodes.
 *
 * This function iterates through the available DRM resources (CRTCs, connectors, encoders, or framebuffers)
 * and creates nodes for each resource. It also creates associated nodes for each resource type.
 *
 * @param drm_fd File descriptor for the DRM device.
 * @param resources Pointer to the DRM resources structure.
 * @param parentNode Pointer to the parent node to which the resource nodes will be added.
 * @param filter If not -1, only the resource with this index will be processed.
 * @return The number of resource nodes created.
 *
 * Usage:
 * - Call the functions with appropriate parameters to create and manage DRM resource nodes.
 *
 * @note Ensure that the file descriptor for the DRM device is valid before calling these functions.
 */

int createCrtcNodes(drmModeRes *resources, Node *parentNode, int filter)
{
    int crtcCount = resources->count_crtcs;
    log_message(LOG_INFO, "Creating CRTC nodes");

    for (int i = 0; i < crtcCount; ++i)
    {
        if (filter != -1 && i != filter)
        {
            continue;
        }

        char crtcName[10];
        snprintf(crtcName, sizeof(crtcName), "CRTC%d", i % 100);
        Node *crtcNode = createNode(crtcName, displayCrtc, parentNode);

        drmModeCrtc *crtc = drmModeGetCrtc(drm_fd, resources->crtcs[i]);
        if (!crtc)
        {
            log_message(LOG_ERROR, "drmModeGetCrtc failed for CRTC %d", i % 100);
            drmModeFreeResources(resources);
            return 0;
        }

        createPlaneNodes(crtcNode, i);
        createEncoderNodes(resources, crtcNode, i);
        createConnectorNodes(resources, crtcNode, i);

        addChild(parentNode, crtcNode);

        drmModeFreeCrtc(crtc);
    }

    return crtcCount;
}

int createConnectorNodes(drmModeRes *resources, Node *parentNode, int filter)
{
    int connectorCount = resources->count_connectors;
    log_message(LOG_INFO, "Creating Connector nodes");

    for (int i = 0; i < connectorCount; ++i)
    {
        if (filter != -1 && i != filter)
        {
            continue;
        }

        char connectorName[15];
        snprintf(connectorName, sizeof(connectorName), "Connector%d", i % 100);
        Node *connectorNode = createNode(connectorName, displayConnector, parentNode);
        addChild(parentNode, connectorNode);
    }

    return connectorCount;
}

int createEncoderNodes(drmModeRes *resources, Node *parentNode, int filter)
{
    int encoderCount = resources->count_encoders;
    log_message(LOG_INFO, "Creating Encoder nodes");

    for (int i = 0; i < encoderCount; ++i)
    {
        if (filter != -1 && i != filter)
        {
            continue;
        }

        char encoderName[15];
        snprintf(encoderName, sizeof(encoderName), "Encoder%d", i % 100);
        Node *encoderNode = createNode(encoderName, displayEncoder, parentNode);
        addChild(parentNode, encoderNode);
    }

    return encoderCount;
}

int createFramebufferNodes(drmModeRes *resources, Node *parentNode, int filter)
{
    int framebufferCount = resources->count_fbs;
    log_message(LOG_INFO, "Creating Framebuffer nodes for %d framebuffers", framebufferCount);

    for (int i = 0; i < framebufferCount; ++i)
    {
        if (filter != -1 && i != filter)
        {
            continue;
        }

        char framebufferName[15];
        snprintf(framebufferName, sizeof(framebufferName), "Framebuffer%d", i % 100);
        Node *framebufferNode = createNode(framebufferName, displayFramebuffer, parentNode);
        addChild(parentNode, framebufferNode);
    }

    return framebufferCount;
}

int createPlaneNodes(Node *parentNode, int filter)
{
    drmModePlaneRes *planeResources = drmModeGetPlaneResources(drm_fd);
    if (!planeResources)
    {
        log_message(LOG_ERROR, "drmModeGetPlaneResources failed");
        return 0;
    }

    int planeCount = planeResources->count_planes;
    log_message(LOG_INFO, "Creating Plane nodes");

    for (int i = 0; i < planeCount; ++i)
    {
        drmModePlane *plane = drmModeGetPlane(drm_fd, planeResources->planes[i]);
        if (!plane)
        {
            log_message(LOG_ERROR, "drmModeGetPlane failed for Plane %d", i % 100);
            drmModeFreePlaneResources(planeResources);

            return 0;
        }

        if (filter != -1 && !(plane->possible_crtcs & (1 << filter)))
        {
            drmModeFreePlane(plane);
            continue;
        }

        char planeName[10];
        snprintf(planeName, sizeof(planeName), "Plane%d", i % 100);
        Node *planeNode = createNode(planeName, displayPlane, parentNode);

        char informatsName[15];
        snprintf(informatsName, sizeof(informatsName), "IN_FORMATS%d", i % 100);
        Node *informatsNode = createNode(informatsName, displayInformats, planeNode);
        addChild(planeNode, informatsNode);

        char outformatsName[15];
        snprintf(outformatsName, sizeof(outformatsName), "FORMATS%d", i % 100);
        Node *outformatsNode = createNode(outformatsName, displayFormats, planeNode);
        addChild(planeNode, outformatsNode);

        addChild(parentNode, planeNode);
        drmModeFreePlane(plane);
    }

    drmModeFreePlaneResources(planeResources);
    return planeCount;
}

void initializeDisplayConfig()
{
    log_message(LOG_INFO, "Initializing Display Configuration");
    Node *displayConfig = createNode("Display Configuration", NULL, root);
    if (!displayConfig)
    {
        log_message(LOG_ERROR, "Failed to create Display Configuration node");
        return;
    }

    if (drm_fd < 0)
    {
        log_message(LOG_ERROR, "Failed to open primary DRM device");
        return;
    }

    drmModeRes *resources = drmModeGetResources(drm_fd);
    if (!resources)
    {
        log_message(LOG_ERROR, "drmModeGetResources failed");
        return;
    }

    /* Create CRTC nodes */
    Node *crtcNodes = createNode("CRTCs", NULL, displayConfig);
    if (!crtcNodes)
    {
        log_message(LOG_ERROR, "Failed to create CRTC nodes");
        drmModeFreeResources(resources);
        return;
    }
    createCrtcNodes(resources, crtcNodes, -1);
    addChild(displayConfig, crtcNodes);

    /* Create Plane nodes */
    Node *planeNodes = createNode("Planes", NULL, displayConfig);
    if (!planeNodes)
    {
        log_message(LOG_ERROR, "Failed to create Plane nodes");
        drmModeFreeResources(resources);
        return;
    }
    createPlaneNodes(planeNodes, -1);
    addChild(displayConfig, planeNodes);

    /* Create Connector nodes */
    Node *connectorNodes = createNode("Connectors", NULL, displayConfig);
    if (!connectorNodes)
    {
        log_message(LOG_ERROR, "Failed to create Connector nodes");
        drmModeFreeResources(resources);
        return;
    }
    createConnectorNodes(resources, connectorNodes, -1);
    addChild(displayConfig, connectorNodes);

    /* Create Encoder nodes */
    Node *encoderNodes = createNode("Encoders", NULL, displayConfig);
    if (!encoderNodes)
    {
        log_message(LOG_ERROR, "Failed to create Encoder nodes");
        drmModeFreeResources(resources);
        return;
    }
    createEncoderNodes(resources, encoderNodes, -1);
    addChild(displayConfig, encoderNodes);

    /* Initialize Framebuffers */
    Node *framebufferNodes = createNode("Framebuffers", NULL, displayConfig);
    if (!framebufferNodes)
    {
        log_message(LOG_ERROR, "Failed to create Framebuffer nodes");
        drmModeFreeResources(resources);
        return;
    }
    createFramebufferNodes(resources, framebufferNodes, -1);
    addChild(displayConfig, framebufferNodes);

    addChild(root, displayConfig);
    drmModeFreeResources(resources);
}