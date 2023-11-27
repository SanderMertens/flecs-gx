#ifndef FLECS_GX_GEOMETRY2_H
#define FLECS_GX_GEOMETRY2_H

#include "geometry.h"

gx_mesh2_t gx_make_rect(void);

void gx_transform2(
    GxTransform2Computed *t,
    GxTransform2Computed *t_parent,
    int32_t count,
    GxCanvas *canvas);

void gx_align2(
    GxTransform2Computed *t,
    GxTransform2Computed *t_parent,
    int32_t count,
    GxCanvas *canvas);

void gx_scale2(
    GxTransform2Computed *t,
    int32_t count);

void gx_geometry2_import(
    ecs_world_t *world);

void gx_geometry2_init(
    ecs_world_t *world,
    gx_geometry2_t *ctx);

void gx_geometry2_populate(
    ecs_world_t *world,
    gx_geometry2_t*ctx,
    GxCanvas *canvas);

void gx_geometry2_draw(
    gx_geometry2_t *geometry,
    GxCanvas *canvas);

#endif
