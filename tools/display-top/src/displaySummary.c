#include "display.h"
#include "utils.h"

void displaySummary(WINDOW *pad, Node *node, int *content_line)
{
    wclear(pad);

    int line = 0;

    print_bold_text(pad, line++, 1,  "%s Summary", node->name);
    line++;

    print_dim_text(pad, line++, 1, "Fetching details for card: %s", find_drm_device(true));
    line++;

    if (drm_fd < 0)
    {
        mvwprintw(pad, line++, 2, "Failed to open DRM Device!\n");
        goto error;
    }

    drmModeRes *resources;
    drmModeCrtc *crtc;

    resources = drmModeGetResources(drm_fd);

    if (!resources)
    {
        mvwprintw(pad, line++, 2, "Failed to get DRM resources\n");
        goto error;
    }

    wattron(pad, A_BOLD);
    mvwprintw(pad, line++, 1, "CRTCs - %d", resources->count_crtcs);
    wattroff(pad, A_BOLD);

    int col_width = 10;
    mvwprintw(pad, line++, 2, "CRTC ID   |");
    for (int i = 0; i < resources->count_crtcs; i++)
    {
        crtc = drmModeGetCrtc(drm_fd, resources->crtcs[i]);

        if (crtc)
        {
            if (crtc->mode_valid)
            {
                wattron(pad, A_BOLD | COLOR_PAIR(2));
                mvwprintw(pad, line - 1, 13 + (i * (col_width + 1)), "%*d", col_width, crtc->crtc_id);
                wattroff(pad, A_BOLD | COLOR_PAIR(2));
                wprintw(pad, "|");
            }
            else
            {
                wattron(pad, A_BOLD | COLOR_PAIR(3));
                mvwprintw(pad, line - 1, 13 + (i * (col_width + 1)), "%*d", col_width, crtc->crtc_id);
                wattroff(pad, A_BOLD | COLOR_PAIR(3));
                wprintw(pad, "|");
            }
            drmModeFreeCrtc(crtc);
        }
        else
        {
            mvwprintw(pad, line - 1, 13 + (i * (col_width + 1)), "%*s|", col_width, "N/A");
        }
    }
    line++;

    wattron(pad, A_BOLD);
    mvwprintw(pad, line++, 1, "Connectors - %d", resources->count_connectors);
    wattroff(pad, A_BOLD);

    col_width = 10;
    mvwprintw(pad, line++, 2, "Connectors|");
    for (int i = 0; i < resources->count_connectors; i++)
    {
        drmModeConnector *connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
        if (connector)
        {
            if (connector->connection == DRM_MODE_CONNECTED)
            {
                wattron(pad, A_BOLD | COLOR_PAIR(2));
                mvwprintw(pad, line - 1, 13 + (i * (col_width + 1)), "%*d", col_width, connector->connector_id);
                wattroff(pad, A_BOLD | COLOR_PAIR(2));
                wprintw(pad, "|");
            }
            else
            {
                wattron(pad, A_BOLD | COLOR_PAIR(3));
                mvwprintw(pad, line - 1, 13 + (i * (col_width + 1)), "%*d", col_width, connector->connector_id);
                wattroff(pad, A_BOLD | COLOR_PAIR(3));
                wprintw(pad, "|");
            }
            drmModeFreeConnector(connector);
        }
        else
        {
            mvwprintw(pad, line - 1, 13 + (i * (col_width + 1)), "%*s|", col_width, "N/A");
        }
    }

    mvwprintw(pad, line++, 2, "Type      |");
    for (int i = 0; i < resources->count_connectors; i++)
    {
        drmModeConnector *connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
        if (connector)
        {
            const char *type_name = get_connector_type_name(connector->connector_type);
            mvwprintw(pad, line - 1, 13 + (i * (col_width + 1)), "%*s|", col_width, type_name);
            drmModeFreeConnector(connector);
        }
        else
        {
            mvwprintw(pad, line - 1, 13 + (i * (col_width + 1)), "%*s|", col_width, "N/A");
        }
    }
    line++;

    wattron(pad, A_BOLD);
    mvwprintw(pad, line++, 1, "Encoders - %d", resources->count_encoders);
    wattroff(pad, A_BOLD);

    col_width = 6;
    mvwprintw(pad, line++, 2, "Encoders|");
    for (int i = 0; i < resources->count_encoders; i++)
    {
        drmModeEncoder *encoder = drmModeGetEncoder(drm_fd, resources->encoders[i]);
        if (encoder)
        {
            mvwprintw(pad, line - 1, 11 + (i * (col_width + 1)), "%*d|", col_width, encoder->encoder_id);
            drmModeFreeEncoder(encoder);
        }
        else
        {
            mvwprintw(pad, line - 1, 11 + (i * (col_width + 1)), "%*s|", col_width, "N/A");
        }
    }

    mvwprintw(pad, line++, 2, "Type    |");
    for (int i = 0; i < resources->count_encoders; i++)
    {
        drmModeEncoder *encoder = drmModeGetEncoder(drm_fd, resources->encoders[i]);
        if (encoder)
        {
            mvwprintw(pad, line - 1, 11 + (i * (col_width + 1)), "%*s|", col_width, get_encoder_type_name(encoder->encoder_type));
            drmModeFreeEncoder(encoder);
        }
        else
        {
            mvwprintw(pad, line - 1, 11 + (i * (col_width + 1)), "%*s|", col_width, "N/A");
        }
    }

    mvwprintw(pad, line++, 2, "CRTCs   |");
    for (int i = 0; i < resources->count_encoders; i++)
    {
        drmModeEncoder *encoder = drmModeGetEncoder(drm_fd, resources->encoders[i]);
        if (encoder)
        {
            mvwprintw(pad, line - 1, 11 + (i * (col_width + 1)), "%*d|", col_width, encoder->crtc_id);
            drmModeFreeEncoder(encoder);
        }
        else
        {
            mvwprintw(pad, line - 1, 11 + (i * (col_width + 1)), "%*s|", col_width, "N/A");
        }
    }
    line++;

    drmVersionPtr version = drmGetVersion(drm_fd);
    if (version)
    {
        wattron(pad, A_BOLD);
        mvwprintw(pad, line++, 1, "DRM Driver: %s", version->name);
        wattroff(pad, A_BOLD);
        drmFreeVersion(version);
    }
    else
    {
        mvwprintw(pad, line++, 2, "Failed to get DRM version");
    }

    // Fetch and display supported DRM capabilities
    uint64_t value;

    drmGetCap(drm_fd, DRM_CAP_TIMESTAMP_MONOTONIC, &value);
    mvwprintw(pad, line++, 2, "DRM_CAP_TIMESTAMP_MONOTONIC: %lu", value);

    drmGetCap(drm_fd, DRM_CAP_CRTC_IN_VBLANK_EVENT, &value);
    mvwprintw(pad, line++, 2, "DRM_CAP_CRTC_IN_VBLANK_EVENT: %lu", value);

    drmGetCap(drm_fd, DRM_CAP_ASYNC_PAGE_FLIP, &value);
    mvwprintw(pad, line++, 2, "DRM_CAP_ASYNC_PAGE_FLIP: %lu", value);

    drmGetCap(drm_fd, DRM_CLIENT_CAP_ATOMIC, &value);
    mvwprintw(pad, line++, 2, "DRM_CLIENT_CAP_ATOMIC: %lu", value);

    drmGetCap(drm_fd, DRM_CAP_ADDFB2_MODIFIERS, &value);
    mvwprintw(pad, line++, 2, "DRM_CAP_ADDFB2_MODIFIERS: %lu", value);
    line++;

    drmDevicePtr devices[8];
    int device_count = drmGetDevices2(0, devices, 8);
    if (device_count < 0)
    {
        mvwprintw(pad, line++, 2, "Failed to get DRM devices");
    }
    else
    {
        for (int i = 0; i < device_count; i++)
        {
            if (devices[i]->available_nodes & (1 << DRM_NODE_PRIMARY))
            {
                wattron(pad, A_BOLD);
                mvwprintw(pad, line++, 1, "Device: PCI %04x:%04x", devices[i]->deviceinfo.pci->vendor_id, devices[i]->deviceinfo.pci->device_id);
                wattroff(pad, A_BOLD);
                char available_nodes[256] = "";
                if (devices[i]->available_nodes & (1 << DRM_NODE_PRIMARY))
                {
                    strcat(available_nodes, "primary, ");
                }
                if (devices[i]->available_nodes & (1 << DRM_NODE_RENDER))
                {
                    strcat(available_nodes, "render, ");
                }
                if (strlen(available_nodes) > 0)
                {
                    available_nodes[strlen(available_nodes) - 2] = '\0';
                }
                mvwprintw(pad, line++, 2, "Available nodes: %s", available_nodes);
            }
        }
        drmFreeDevices(devices, device_count);
    }

    drmModeFreeResources(resources);

    *content_line = line;
    return;

error:
    if (resources)
    {
        drmModeFreeResources(resources);
    }

    wrefresh(pad);
    *content_line = line;
    return;
}
