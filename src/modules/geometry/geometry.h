#ifndef FLECS_GX_GEOMETRY_H
#define FLECS_GX_GEOMETRY_H

#include "../canvas/canvas.h"

#undef ECS_META_IMPL
#ifndef FLECS_GX_GEOMETRY_IMPL
#define ECS_META_IMPL EXTERN
#endif

struct gx_geometry2_data_t {
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
};

struct gx_geometry2_t {
    gx_geometry2_data_t rect_data;
    sg_pipeline pip;
};

struct gx_text_t {
    /* ECS query for finding text entities */
    ecs_query_t *query;

    /* Fontstash context */
    FONScontext* fs;

    /* Regular font */
    int font_regular;
    uint8_t font_regular_data[256 * 1024];

    /* Bold font */
    int font_bold;
    uint8_t font_bold_data[256 * 1024];

    /* Italic font */
    int font_italic;
    uint8_t font_italic_data[256 * 1024];
};

struct gx_geometry_t {
    gx_geometry2_t geometry2;
    gx_text_t text;
};

ECS_STRUCT(GxTransform2Computed, {
    vec3 position;
    vec3 scale;
    float rotation;
    EcsAlign align;
    float padding;
ECS_PRIVATE
    mat4 mat;
});

ECS_STRUCT(GxStyleComputed, {
    vec3 color;
    vec3 stroke_color;
    float stroke_width;
});

gx_geometry_t* gx_geometry_init(
    ecs_world_t *world,
    GxCanvas *canvas);

void gx_geometry_populate(
    ecs_world_t *world,
    gx_geometry_t*ctx,
    GxCanvas *canvas);

void gx_geometry_draw(
    ecs_world_t *world,
    GxCanvas *canvas);

void FlecsGxGeometryImport(
    ecs_world_t *world);

#endif
