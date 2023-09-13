#ifndef FLECS_GX_CANVAS_H
#define FLECS_GX_CANVAS_H

#include "../../private_api.h"

typedef struct {
    float left;
    float right;
    float top;
    float bottom;
} gx_viewport_t;

typedef struct {
    int32_t width;
    int32_t height;
    int32_t actual_width;
    int32_t actual_height;
    float dpi_scale;
    gx_viewport_t viewport;
    gx_geometry_t *geometry;
} GxCanvas;

extern ECS_COMPONENT_DECLARE(GxCanvas);

void FlecsGxCanvasImport(
    ecs_world_t *world);

#endif
