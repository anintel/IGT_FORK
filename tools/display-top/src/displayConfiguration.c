#include "display.h"
#include "utils.h"

void displayCrtc(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;

    if (drm_fd < 0)
    {
        print_red_text(pad, line++, 1, "Failed to open DRM Device!\n");
        goto err;
    }

    drmModeRes *resources = drmModeGetResources(drm_fd);
    if (!resources)
    {
        print_red_text(pad, line++, 1, "Failed to get DRM resources\n");
        goto err;
    }

    int crtc_index = -1;
    char *endptr;

    crtc_index = strtol(node->name + 4, &endptr, 10);
    if (*endptr != '\0' || crtc_index < 0 || crtc_index >= resources->count_crtcs)
    {
        print_red_text(pad, line++, 1, "Invalid CRTC name: %s\n", node->name);
        drmModeFreeResources(resources);
        goto err;
    }

    drmModeCrtc *crtc = drmModeGetCrtc(drm_fd, resources->crtcs[crtc_index]);
    if (!crtc)
    {
        print_red_text(pad, line++, 1, "Failed to get CRTC %d\n", crtc_index);
        drmModeFreeResources(resources);
        goto err;
    }

    mvwprintw(pad, line++, 1, "CRTC ID: %d", crtc->crtc_id);
    mvwprintw(pad, line++, 1, "Framebuffer ID: %d", crtc->buffer_id);
    mvwprintw(pad, line++, 1, "Position(X, Y): (%d, %d)", crtc->x, crtc->y);
    mvwprintw(pad, line++, 1, "Mode: ");
    if (crtc->mode_valid)
        print_green_text(pad, -1, -1, "VALID");
    else
        print_red_text(pad, -1, -1, "INVALID");

    if (crtc->mode_valid)
    {
        drmModeModeInfo *mode = &crtc->mode;
        mvwprintw(pad, line++, 3, "Name: %s", mode->name);
        mvwprintw(pad, line++, 3, "Resolution: %dx%d", mode->hdisplay, mode->vdisplay);
        mvwprintw(pad, line++, 3, "Refresh Rate: %.2f Hz", mode->clock / (float)(mode->htotal * mode->vtotal));
        mvwprintw(pad, line++, 3, "Type: 0x%x", mode->type);
        mvwprintw(pad, line++, 3, "Flags: 0x%x", mode->flags);
    }

    mvwprintw(pad, line++, 1, "Gamma size: %u", crtc->gamma_size);

    drmModeObjectProperties *props = drmModeObjectGetProperties(drm_fd, crtc->crtc_id, DRM_MODE_OBJECT_CRTC);
    if (!props)
    {
        print_red_text(pad, line++, 1, "Failed to get properties for CRTC %u\n", crtc->crtc_id);
        drmModeFreeCrtc(crtc);
        drmModeFreeResources(resources);
        goto err;
    }

    drmModePlaneRes *plane_resources = drmModeGetPlaneResources(drm_fd);
    if (!plane_resources)
    {
        print_red_text(pad, line++, 3, "Failed to get plane resources");
        return;
    }

    line++;
    mvwprintw(pad, line++, 1, "Possible Planes for CRTC %d:", crtc->crtc_id);
    line++;

    wattron(pad, A_DIM);
    for (size_t i = 0; i < plane_resources->count_planes; i++)
    {
        drmModePlane *plane = drmModeGetPlane(drm_fd, plane_resources->planes[i]);
        if (plane)
        {
            if (plane->possible_crtcs & (1 << crtc_index))
            {
                mvwprintw(pad, line++, 3, "Plane %ld: ID %d, %s",
                          i, plane->plane_id, plane->crtc_id ? "Active" : "Inactive");
            }
            drmModeFreePlane(plane);
        }
        else
        {
            print_red_text(pad, line++, 3, "Plane %ld: Failed to get plane", i);
        }
    }
    wattroff(pad, A_DIM);
    drmModeFreePlaneResources(plane_resources);

    line++;
    mvwprintw(pad, line++, 1, "Properties for CRTC %u:", crtc->crtc_id);
    line++;

    wattron(pad, A_DIM);
    for (uint32_t i = 0; i < props->count_props; i++)
    {
        drmModePropertyPtr prop = drmModeGetProperty(drm_fd, props->props[i]);
        if (prop)
        {
            uint64_t value = props->prop_values[i];
            char value_str[256] = {0};

            if (prop->flags & DRM_MODE_PROP_BLOB)
            {
                snprintf(value_str, sizeof(value_str), "blob = %lu", value);
            }
            else if (prop->flags & DRM_MODE_PROP_ENUM)
            {
                snprintf(value_str, sizeof(value_str), "enum (");
                for (int j = 0; j < prop->count_enums; j++)
                {
                    if (j > 0)
                    {
                        snprintf(value_str + strlen(value_str), sizeof(value_str) - strlen(value_str), ", ");
                    }
                    snprintf(value_str + strlen(value_str), sizeof(value_str) - strlen(value_str), "%s", prop->enums[j].name);
                }
                snprintf(value_str + strlen(value_str), sizeof(value_str) - strlen(value_str), ") = %s", prop->enums[value].name);
            }
            else if (prop->flags & DRM_MODE_PROP_RANGE)
            {
                snprintf(value_str, sizeof(value_str), "range [%lu, %lu] = %lu",
                         prop->values[0], prop->values[1], value);
            }
            else if (prop->flags & DRM_MODE_PROP_OBJECT)
            {
                snprintf(value_str, sizeof(value_str), "object %s = %lu",
                         get_drm_object_type_name(prop->values[0]), value);
            }
            else
            {
                snprintf(value_str, sizeof(value_str), "%lu", value);
            }

            const char *immutability = (prop->flags & DRM_MODE_PROP_IMMUTABLE) ? " immutable" : "";
            const char *atomicity = (prop->flags & DRM_MODE_PROP_ATOMIC) ? " atomic" : " ";

            wattroff(pad, A_DIM);
            mvwprintw(pad, line++, 2, "%d. %s", i, prop->name);
            wattron(pad, A_DIM);
            wprintw(pad, "%s%s: ", immutability, atomicity);

            int remaining_width = getmaxx(pad) - getcurx(pad);
            if ((int)strlen(value_str) > remaining_width)
            {
                char truncated_value[remaining_width + 1];
                strncpy(truncated_value, value_str, remaining_width);
                truncated_value[remaining_width] = '\0';
                wprintw(pad, "%s", truncated_value);
                mvwprintw(pad, line++, 2, "%s", value_str + remaining_width);
            }
            else
            {
                wprintw(pad, "%s", value_str);
            }

            drmModeFreeProperty(prop);
        }
    }
    wattroff(pad, A_DIM);

    drmModeFreeObjectProperties(props);

    mvwprintw(pad, ++line, 1, "Press 'e' to go back.");

    *content_line = line;
    drmModeFreeCrtc(crtc);
    drmModeFreeResources(resources);
    return;

err:
    wrefresh(pad);
    wgetch(pad);
    return;
}

void displayConnector(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;

    if (drm_fd < 0)
    {
        print_red_text(pad, line++, 1, "Failed to open DRM Device!\n");
        goto err;
    }

    drmModeRes *resources = drmModeGetResources(drm_fd);
    if (!resources)
    {
        print_red_text(pad, line++, 1, "Failed to get DRM resources\n");
        goto err;
    }

    int connector_index = -1;
    char *endptr;

    connector_index = strtol(node->name + 9, &endptr, 10);

    if (*endptr != '\0' || connector_index < 0 || connector_index >= resources->count_connectors)
    {
        mvwprintw(pad, line++, 1, "Invalid Connector name: %s\n", node->name);
        drmModeFreeResources(resources);
        goto err;
    }

    drmModeConnector *connector = drmModeGetConnector(drm_fd, resources->connectors[connector_index]);
    if (!connector)
    {
        mvwprintw(pad, line++, 1, "Failed to get Connector %d\n", connector_index);
        drmModeFreeResources(resources);
        goto err;
    }

    mvwprintw(pad, line++, 1, "Connector ID: %d", connector->connector_id);
    mvwprintw(pad, line++, 1, "No. of Encoders: %d", connector->count_encoders);
    mvwprintw(pad, line++, 1, "Encoder ID: %d", connector->encoder_id);

    line++;
    mvwprintw(pad, line++, 1, "Possible Encoders: ");
    for (int i = 0; i < connector->count_encoders; i++)
    {
        mvwprintw(pad, line++, 3, "Encoder %d: %d", i, connector->encoders[i]);
    }

    line++;
    mvwprintw(pad, line++, 1, "Connection: ");
    if (connector->connection == DRM_MODE_CONNECTED)
        print_green_text(pad, -1, -1, "Connected");
    else
        print_red_text(pad, -1, -1, "Disconnected");

    line++;
    mvwprintw(pad, line++, 1, "Connector Type: %s", get_connector_type_name(connector->connector_type));
    mvwprintw(pad, line++, 1, "Connector Type ID: %d", connector->connector_type_id);

    mvwprintw(pad, line++, 1, "Subpixel Order: %d", connector->subpixel);
    mvwprintw(pad, line++, 1, "MM Height: %d", connector->mmHeight);
    mvwprintw(pad, line++, 1, "MM Width: %d", connector->mmWidth);

    mvwprintw(pad, line++, 1, "Modes: %d", connector->count_modes);
    wattron(pad, A_DIM);

    line++;
    for (int i = 0; i < connector->count_modes; i++)

        if (connector->modes && i < connector->count_modes)
        {
            const drmModeModeInfo *mode = &connector->modes[i];

            double refresh_rate = 0.0;
            if (mode->vrefresh > 0)
            {
                refresh_rate = mode->vrefresh;
            }
            else if (mode->clock > 0 && mode->vtotal > 0)
            {
                refresh_rate = (double)(mode->clock * 1000) / (mode->htotal * mode->vtotal);
            }

            const char *preferred = (mode->type & DRM_MODE_TYPE_PREFERRED) ? " preferred" : "";
            const char *driver = (mode->type & DRM_MODE_TYPE_DRIVER) ? " driver" : "";

            const char *phsync = (mode->flags & DRM_MODE_FLAG_PHSYNC) ? " phsync" : " nhsync";
            const char *pvsync = (mode->flags & DRM_MODE_FLAG_PVSYNC) ? " pvsync" : " nvsync";

            const char *interlace = (mode->flags & DRM_MODE_FLAG_INTERLACE) ? " interlace" : "";

            const char *aspect_ratio = "";
            double ratio = (double)mode->hdisplay / mode->vdisplay;
            if (fabs(ratio - 16.0 / 9.0) < 0.01)
            {
                aspect_ratio = " 16:9";
            }
            else if (fabs(ratio - 4.0 / 3.0) < 0.01)
            {
                aspect_ratio = " 4:3";
            }
            else
            {
                aspect_ratio = "";
            }

            mvwprintw(pad, line++, 2,
                      "%ux%u @%.2f%s%s%s%s%s%s",
                      mode->hdisplay, mode->vdisplay, refresh_rate,
                      driver, preferred, phsync, pvsync, interlace, aspect_ratio);
        }

    wattroff(pad, A_DIM);

    mvwprintw(pad, line++, 1, "No. of Properties: %d", connector->count_props);

    wattron(pad, A_DIM);
    for (int i = 0; i < connector->count_props; i++)
    {
        drmModePropertyPtr prop = drmModeGetProperty(drm_fd, connector->props[i]);
        if (prop)
        {
            uint64_t value = connector->prop_values[i];

            // Determine property type and format value accordingly
            char value_str[256] = {0};

            if (prop->flags & DRM_MODE_PROP_BLOB)
            {
                snprintf(value_str, sizeof(value_str), "blob = %lu", value);
            }
            else if (prop->flags & DRM_MODE_PROP_ENUM)
            {

                snprintf(value_str, sizeof(value_str), "enum (");
                for (int j = 0; j < prop->count_enums; j++)
                {
                    if (j > 0)
                    {
                        snprintf(value_str + strlen(value_str), sizeof(value_str) - strlen(value_str), ", ");
                    }
                    snprintf(value_str + strlen(value_str), sizeof(value_str) - strlen(value_str), "%s", prop->enums[j].name);
                }
                snprintf(value_str + strlen(value_str), sizeof(value_str) - strlen(value_str), ") = %s", prop->enums[value].name);
            }
            else if (prop->flags & DRM_MODE_PROP_RANGE)
            {
                snprintf(value_str, sizeof(value_str), "range [%lu, %lu] = %lu",
                         prop->values[0], prop->values[1], value);
            }
            else if (prop->flags & DRM_MODE_PROP_OBJECT)
            {
                snprintf(value_str, sizeof(value_str), "object %s = %lu",
                         get_drm_object_type_name(prop->values[0]), value);
            }
            else
            {
                snprintf(value_str, sizeof(value_str), "%lu", value);
            }

            // Determine if the property is immutable or atomic
            const char *immutability = (prop->flags & DRM_MODE_PROP_IMMUTABLE) ? " immutable" : "";
            const char *atomicity = (prop->flags & DRM_MODE_PROP_ATOMIC) ? " atomic" : " ";

            // Print the property with all details
            wattroff(pad, A_DIM);
            mvwprintw(pad, line++, 2, "%d. %s", i, prop->name);
            wattron(pad, A_DIM);
            wprintw(pad, "%s%s: ", immutability, atomicity);

            // Handle line overflow
            int remaining_width = getmaxx(pad) - getcurx(pad);
            if ((int)strlen(value_str) > remaining_width)
            {
                char truncated_value[remaining_width + 1];
                strncpy(truncated_value, value_str, remaining_width);
                truncated_value[remaining_width] = '\0';
                wprintw(pad, "%s", truncated_value);
                mvwprintw(pad, line++, 2, "%s", value_str + remaining_width);
            }
            else
            {
                wprintw(pad, "%s", value_str);
            }

            drmModeFreeProperty(prop);
        }
    }
    wattroff(pad, A_DIM);

    mvwprintw(pad, ++line, 1, "Press 'e' to go back.");

    *content_line = line;
    drmModeFreeConnector(connector);
    drmModeFreeResources(resources);
    return;

err:
    wrefresh(pad);
    wgetch(pad);
    return;
}

void displayEncoder(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;

    if (drm_fd < 0)
    {
        mvwprintw(pad, line++, 1, "Failed to open DRM Device!\n");
        goto err;
    }

    drmModeRes *resources = drmModeGetResources(drm_fd);
    if (!resources)
    {
        mvwprintw(pad, line++, 1, "Failed to get DRM resources\n");
        goto err;
    }

    int encoder_id = -1;
    char *endptr;

    encoder_id = strtol(node->name + 7, &endptr, 10);
    if (*endptr != '\0' || encoder_id < 0 || encoder_id >= resources->count_encoders)
    {
        mvwprintw(pad, line++, 1, "Invalid Encoder name: %s\n", node->name);
        drmModeFreeResources(resources);
        goto err;
    }

    drmModeEncoder *encoder = drmModeGetEncoder(drm_fd, resources->encoders[encoder_id]);
    if (!encoder)
    {
        mvwprintw(pad, line++, 1, "Failed to get Encoder %d\n", encoder_id);
        drmModeFreeResources(resources);
        goto err;
    }

    line++;
    mvwprintw(pad, line++, 1, "Encoder Type: ");
    if (encoder->encoder_type == DRM_MODE_ENCODER_NONE)
        print_red_text(pad, -1, -1, "NONE");
    else
        print_bold_text(pad, -1, -1, get_encoder_type_name(encoder->encoder_type));
    line++;

    mvwprintw(pad, line++, 1, "Encoder ID: %d", encoder->encoder_id);
    mvwprintw(pad, line++, 1, "CRTC ID: %d", encoder->crtc_id);
    mvwprintw(pad, line++, 1, "Possible CRTCs: %d", encoder->possible_crtcs);
    mvwprintw(pad, line++, 1, "Possible Clones: %d", encoder->possible_clones);

    mvwprintw(pad, ++line, 1, "Press 'e' to go back.");

    *content_line = line;
    drmModeFreeEncoder(encoder);
    drmModeFreeResources(resources);
    return;

err:
    wrefresh(pad);
    wgetch(pad);
    return;
}

void displayFramebuffer(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;

    if (drm_fd < 0)
    {
        mvwprintw(pad, line++, 1, "Failed to open DRM Device!\n");
        goto err;
    }

    drmModeRes *resources = drmModeGetResources(drm_fd);
    if (!resources)
    {
        mvwprintw(pad, line++, 1, "Failed to get DRM resources\n");
        goto err;
    }

    int framebuffer_id = -1;
    char *endptr;

    framebuffer_id = strtol(node->name + 11, &endptr, 10);
    if (*endptr != '\0' || framebuffer_id < 0 || framebuffer_id >= resources->count_fbs)
    {
        mvwprintw(pad, line++, 1, "Invalid Framebuffer name: %s\n", node->name);
        drmModeFreeResources(resources);
        goto err;
    }

    drmModeFB *fb = drmModeGetFB(drm_fd, resources->fbs[framebuffer_id]);
    if (!fb)
    {
        mvwprintw(pad, line++, 1, "Failed to get Framebuffer %d\n", framebuffer_id);
        drmModeFreeResources(resources);
        goto err;
    }

    mvwprintw(pad, line++, 1, "Framebuffer ID: %d", fb->fb_id);
    mvwprintw(pad, line++, 1, "Width: %d", fb->width);
    mvwprintw(pad, line++, 1, "Height: %d", fb->height);
    mvwprintw(pad, line++, 1, "Pitch: %d", fb->pitch);
    mvwprintw(pad, line++, 1, "BPP: %d", fb->bpp);
    mvwprintw(pad, line++, 1, "Depth: %d", fb->depth);
    mvwprintw(pad, line++, 1, "Handle: %d", fb->handle);

    mvwprintw(pad, ++line, 1, "Press 'e' to go back.");

    *content_line = line;
    drmModeFreeFB(fb);
    drmModeFreeResources(resources);
    return;

err:
    wrefresh(pad);
    wgetch(pad);
    return;
}

void displayPlane(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;

    if (drm_fd < 0)
    {
        mvwprintw(pad, line++, 1, "Failed to open DRM Device!\n");
        goto err;
    }

    drmModePlaneRes *plane_resources = drmModeGetPlaneResources(drm_fd);
    if (!plane_resources)
    {
        mvwprintw(pad, line++, 1, "Failed to get DRM plane resources\n");
        goto err;
    }

    int plane_index = -1;
    char *endptr;

    plane_index = strtol(node->name + 5, &endptr, 10);
    if (*endptr != '\0' || plane_index < 0 || plane_index >= (int)plane_resources->count_planes)
    {
        mvwprintw(pad, line++, 1, "Invalid Plane name: %s\n", node->name);
        drmModeFreePlaneResources(plane_resources);
        goto err;
    }

    drmModePlane *plane = drmModeGetPlane(drm_fd, plane_resources->planes[plane_index]);
    if (!plane)
    {
        mvwprintw(pad, line++, 1, "Failed to get Plane %d\n", plane_index);
        drmModeFreePlaneResources(plane_resources);
        goto err;
    }

    mvwprintw(pad, line++, 1, "Plane Index: %d", plane_index);
    mvwprintw(pad, line++, 1, "Plane ID: %d", plane->plane_id);
    mvwprintw(pad, line++, 1, "CRTC ID: %d", plane->crtc_id);
    mvwprintw(pad, line++, 1, "Framebuffer ID: %d", plane->fb_id);
    mvwprintw(pad, line++, 1, "Possible CRTCs: %d", plane->possible_crtcs);
    mvwprintw(pad, line++, 1, "Gamma size: %u", plane->gamma_size);
    mvwprintw(pad, line++, 1, "CRTC Position: (%d, %d)", plane->crtc_x, plane->crtc_y);

    mvwprintw(pad, line++, 1, "Possible CRTC IDs: ");
    for (int i = 0; i < 32; i++)
    {
        if (plane->possible_crtcs & (1 << i))
        {
            mvwprintw(pad, line++, 3, "CRTC %d", i);
        }
    }
    line++;

    mvwprintw(pad, line++, 1, "Number of formats: %u", plane->count_formats);
    line++;

    drmModeObjectProperties *props = drmModeObjectGetProperties(drm_fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);
    if (!props)
    {
        mvwprintw(pad, line++, 1, "Failed to get properties for Plane %u\n", plane->plane_id);
        drmModeFreePlane(plane);
        drmModeFreePlaneResources(plane_resources);
        goto err;
    }

    mvwprintw(pad, line++, 1, "Properties for Plane %u:", plane->plane_id);
    line++;

    for (uint32_t i = 0; i < props->count_props; i++)
    {
        drmModePropertyPtr prop = drmModeGetProperty(drm_fd, props->props[i]);
        if (prop)
        {
            uint64_t value = props->prop_values[i];

            // Determine property type and format value accordingly
            char value_str[256] = {0};

            if (prop->flags & DRM_MODE_PROP_BLOB)
            {
                snprintf(value_str, sizeof(value_str), "blob = %lu", value);
            }
            else if (prop->flags & DRM_MODE_PROP_ENUM)
            {
                snprintf(value_str, sizeof(value_str), "enum (");
                for (int j = 0; j < prop->count_enums; j++)
                {
                    if (j > 0)
                    {
                        snprintf(value_str + strlen(value_str), sizeof(value_str) - strlen(value_str), ", ");
                    }
                    snprintf(value_str + strlen(value_str), sizeof(value_str) - strlen(value_str), "%s", prop->enums[j].name);
                }
                snprintf(value_str + strlen(value_str), sizeof(value_str) - strlen(value_str), ") = %s", prop->enums[value].name);
            }
            else if (prop->flags & DRM_MODE_PROP_RANGE)
            {
                snprintf(value_str, sizeof(value_str), "range [%lu, %lu] = %lu",
                         prop->values[0], prop->values[1], value);
            }
            else if (prop->flags & DRM_MODE_PROP_OBJECT)
            {
                snprintf(value_str, sizeof(value_str), "object %s = %lu",
                         get_drm_object_type_name(prop->values[0]), value);
            }
            else
            {
                snprintf(value_str, sizeof(value_str), "%lu", value);
            }

            // Determine if the property is immutable or atomic
            const char *immutability = (prop->flags & DRM_MODE_PROP_IMMUTABLE) ? " immutable" : "";
            const char *atomicity = (prop->flags & DRM_MODE_PROP_ATOMIC) ? " atomic" : " ";

            // Print the property with all details
            wattroff(pad, A_DIM);
            mvwprintw(pad, line++, 2, "%d. %s", i, prop->name);
            wattron(pad, A_DIM);
            wprintw(pad, "%s%s: ", immutability, atomicity);

            // Handle line overflow
            int remaining_width = getmaxx(pad) - getcurx(pad);
            if ((int)strlen(value_str) > remaining_width)
            {
                char truncated_value[remaining_width + 1];
                strncpy(truncated_value, value_str, remaining_width);
                truncated_value[remaining_width] = '\0';
                wprintw(pad, "%s", truncated_value);
                mvwprintw(pad, line++, 2, "%s", value_str + remaining_width);
            }
            else
            {
                wprintw(pad, "%s", value_str);
            }

            drmModeFreeProperty(prop);
        }
    }

    drmModeFreeObjectProperties(props);

    mvwprintw(pad, ++line, 1, "Press 'e' to go back.");

    *content_line = line;
    drmModeFreePlane(plane);
    drmModeFreePlaneResources(plane_resources);
    return;

err:
    wrefresh(pad);
    wgetch(pad);
    return;
}

void displayInformats(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;

    if (drm_fd < 0)
    {
        mvwprintw(pad, line++, 1, "Failed to open DRM Device!\n");
        goto err;
    }

    drmModePlaneRes *plane_resources = drmModeGetPlaneResources(drm_fd);
    if (!plane_resources)
    {
        mvwprintw(pad, line++, 1, "Failed to get DRM plane resources\n");
        goto err;
    }

    // Extract the plane index from node->name
    int plane_index = -1;
    if (strncmp(node->name, "IN_FORMATS", 10) == 0)
    {
        char *endptr;
        plane_index = strtol(node->name + 10, &endptr, 10);
        if (*endptr != '\0' || plane_index < 0 || plane_index >= (int)plane_resources->count_planes)
        {
            mvwprintw(pad, line++, 1, "Invalid Plane Index: %s\n", node->name);
            drmModeFreePlaneResources(plane_resources);
            goto err;
        }
    }
    else
    {
        mvwprintw(pad, line++, 1, "Invalid format for node->name: %s\n", node->name);
        drmModeFreePlaneResources(plane_resources);
        goto err;
    }

    drmModePlane *plane = drmModeGetPlane(drm_fd, plane_resources->planes[plane_index]);
    if (!plane)
    {
        mvwprintw(pad, line++, 1, "Failed to get Plane %d\n", plane_index);
        drmModeFreePlaneResources(plane_resources);
        goto err;
    }

    mvwprintw(pad, line++, 1, "Plane Index: %d", plane_index);
    mvwprintw(pad, line++, 1, "Plane ID: %d", plane->plane_id);

    drmModeObjectProperties *props = drmModeObjectGetProperties(drm_fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);
    if (!props)
    {
        mvwprintw(pad, line++, 1, "Failed to get properties for Plane %u\n", plane->plane_id);
        drmModeFreePlane(plane);
        drmModeFreePlaneResources(plane_resources);
        goto err;
    }

    mvwprintw(pad, line++, 1, "IN_FORMATS for Plane %u:", plane->plane_id);
    line++;

    for (uint32_t i = 0; i < props->count_props; i++)
    {
        drmModePropertyPtr prop = drmModeGetProperty(drm_fd, props->props[i]);
        if (prop)
        {
            uint64_t value = props->prop_values[i];

            if (prop->flags & DRM_MODE_PROP_BLOB && strcmp(prop->name, "IN_FORMATS") == 0)
            {
                drmModePropertyBlobRes *blob = drmModeGetPropertyBlob(drm_fd, value);

                if (!blob)
                {
                    log_message(LOG_ERROR, "drmModeGetPropertyBlob");
                    return;
                }

                const struct drm_format_modifier_blob *data = blob->data;

                uint32_t *fmts = (uint32_t *)((char *)data + data->formats_offset);
                struct drm_format_modifier *mods = (struct drm_format_modifier *)((char *)data + data->modifiers_offset);

                for (uint32_t j = 0; j < data->count_modifiers; j++)
                {

                    mvwprintw(pad, line++, 1, "Modifier %u: %s", j, get_basic_modifier_str(mods[j].modifier));
                    mvwprintw(pad, line++, 3, "Formats: ");
                    wattron(pad, A_DIM);
                    for (uint32_t k = 0; k < data->count_formats; k++)
                    {
                        if (mods[j].formats & (1ULL << k))
                        {
                            mvwprintw(pad, line++, 5, "%s (0x%08x)", get_format_str(fmts[k]), fmts[k]);
                        }
                    }
                    wattroff(pad, A_DIM);
                }

                drmModeFreePropertyBlob(blob);
            }
        }
    }
    drmModeFreeObjectProperties(props);
    drmModeFreePlane(plane);
    drmModeFreePlaneResources(plane_resources);
    mvwprintw(pad, ++line, 1, "Press 'e' to go back.");

    *content_line = line;
    return;

err:
    wrefresh(pad);
    return;
}

void displayFormats(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;

    if (drm_fd < 0)
    {
        mvwprintw(pad, line++, 1, "Failed to open DRM Device!\n");
        goto err;
    }

    drmModePlaneRes *plane_resources = drmModeGetPlaneResources(drm_fd);
    if (!plane_resources)
    {
        mvwprintw(pad, line++, 1, "Failed to get DRM plane resources\n");
        goto err;
    }

    // Extract the plane index from node->name
    int plane_index = -1;
    if (strncmp(node->name, "FORMATS", 7) == 0)
    {
        char *endptr;
        plane_index = strtol(node->name + 7, &endptr, 10);
        if (*endptr != '\0' || plane_index < 0 || plane_index >= (int)plane_resources->count_planes)
        {
            mvwprintw(pad, line++, 1, "Invalid Plane Index: %s\n", node->name);
            drmModeFreePlaneResources(plane_resources);
            goto err;
        }
    }
    else
    {
        mvwprintw(pad, line++, 1, "Invalid format for node->name: %s\n", node->name);
        drmModeFreePlaneResources(plane_resources);
        goto err;
    }

    drmModePlane *plane = drmModeGetPlane(drm_fd, plane_resources->planes[plane_index]);
    if (!plane)
    {
        mvwprintw(pad, line++, 1, "Failed to get Plane %d\n", plane_index);
        drmModeFreePlaneResources(plane_resources);
        goto err;
    }

    mvwprintw(pad, line++, 1, "Plane ID: %d", plane->plane_id);

    line++;
    mvwprintw(pad, line++, 1, "No. of Plane formats %u:", plane->count_formats);
    line++;

    if (plane->count_formats > 0)
    {
        wattron(pad, A_DIM);
        for (uint32_t i = 0; i < plane->count_formats; i++)
        {
            mvwprintw(pad, line++, 3, "%s (0x%08x)", get_format_str(plane->formats[i]), plane->formats[i]);
        }
        line++;
        wattroff(pad, A_DIM);
    }

    drmModeFreePlane(plane);
    drmModeFreePlaneResources(plane_resources);

    mvwprintw(pad, ++line, 1, "Press 'e' to go back.");

    *content_line = line;
    return;

err:
    wrefresh(pad);
    return;
}

void gotoCrtc(WINDOW *pad, Node *node, int *content_line)
{
    int line = 0;

    if (drm_fd < 0)
    {
        mvwprintw(pad, line++, 1, "Failed to open DRM Device!\n");
        goto err;
    }

    drmModeRes *resources = drmModeGetResources(drm_fd);
    if (!resources)
    {
        mvwprintw(pad, line++, 1, "Failed to get DRM resources\n");
        goto err;
    }

    int crtc_index = -1;
    char *endptr;

    crtc_index = strtol(node->name + 4, &endptr, 10);
    if (*endptr != '\0' || crtc_index < 0 || crtc_index >= resources->count_crtcs)
    {
        mvwprintw(pad, line++, 1, "Invalid CRTC name: %s\n", node->name);
        drmModeFreeResources(resources);
        goto err;
    }

    drmModeCrtc *crtc = drmModeGetCrtc(drm_fd, resources->crtcs[crtc_index]);
    if (!crtc)
    {
        mvwprintw(pad, line++, 1, "Failed to get CRTC %d\n", crtc_index);
        drmModeFreeResources(resources);
        goto err;
    }

    mvwprintw(pad, line++, 1, "CRTC ID: %d", crtc->crtc_id);
    mvwprintw(pad, line++, 1, "Framebuffer ID: %d", crtc->buffer_id);
    mvwprintw(pad, line++, 1, "Position(X, Y): (%d, %d)", crtc->x, crtc->y);
    mvwprintw(pad, line++, 1, "Mode: ");
    if (crtc->mode_valid)
        print_green_text(pad, -1, -1, "VALID");
    else
        print_red_text(pad, -1, -1, "INVALID");

    if (crtc->mode_valid)
    {
        drmModeModeInfo *mode = &crtc->mode;
        mvwprintw(pad, line++, 3, "Name: %s", mode->name);
        mvwprintw(pad, line++, 3, "Resolution: %dx%d", mode->hdisplay, mode->vdisplay);
        mvwprintw(pad, line++, 3, "Refresh Rate: %.2f Hz", mode->clock / (float)(mode->htotal * mode->vtotal));
        mvwprintw(pad, line++, 3, "Type: 0x%x", mode->type);
        mvwprintw(pad, line++, 3, "Flags: 0x%x", mode->flags);
    }

    mvwprintw(pad, line++, 1, "Gamma size: %u", crtc->gamma_size);

    drmModeFreeCrtc(crtc);
    drmModeFreeResources(resources);

    mvwprintw(pad, ++line, 1, "Press 'e' to go back.");

    *content_line = line;
    return;

err:
    wrefresh(pad);
    wgetch(pad);
    return;
}