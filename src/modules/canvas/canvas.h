#ifndef FLECS_GX_CANVAS_H
#define FLECS_GX_CANVAS_H

#include "../../private_api.h"

typedef struct {
    /* ECS query for collecting objects */
    ecs_query_t *query;

    /* Instance data */
    ecs_vec_t color;
    ecs_vec_t transform;

    /* Number of instances */
    int32_t instance_count;

    /* Number of elements in index buffer */
    int32_t index_count;

    /* Vertex attributes */
    sg_buffer verts;
    sg_buffer idx;

    /* Instance attributes */
    sg_buffer color_buf;
    sg_buffer transform_buf;
} gx_geometry_2d_t;

typedef struct {
    gx_geometry_2d_t rect_data;
    sg_pipeline pip;
} gx_draw2d_ctx_t;

typedef struct {
    int32_t width;
    int32_t height;

    gx_draw2d_ctx_t draw2d_ctx;
} GxCanvas;

extern ECS_COMPONENT_DECLARE(GxCanvas);

void FlecsGxCanvasImport(
    ecs_world_t *world);

#endif
