#ifndef FLECS_GX_ROUNDED_RECT_H
#define FLECS_GX_ROUNDED_RECT_H

#include "geometry.h"

void gx_rounded_rect_init(
    ecs_world_t *world,
    gx_rounded_rect_t *ctx);

void gx_rounded_rect_populate(
    ecs_world_t *world,
    gx_rounded_rect_t *ctx,
    GxCanvas *canvas);

void gx_rounded_rect_draw(
    ecs_world_t *world,
    gx_rounded_rect_t *geometry,
    GxCanvas *canvas);

#endif
