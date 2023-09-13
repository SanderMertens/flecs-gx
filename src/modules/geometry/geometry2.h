#ifndef FLECS_GX_GEOMETRY2_H
#define FLECS_GX_GEOMETRY2_H

#include "geometry.h"

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
