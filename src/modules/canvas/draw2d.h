#ifndef FLECS_GX_DRAW2D_H
#define FLECS_GX_DRAW2D_H

#include "canvas.h"

void gx_draw2d_init(
    ecs_world_t *world,
    GxCanvas *canvas);

void gx_draw2d_populate_geometry(
    ecs_world_t *world,
    GxCanvas *canvas);

void gx_draw2d_draw(
    ecs_world_t *world,
    GxCanvas *canvas);

#endif
