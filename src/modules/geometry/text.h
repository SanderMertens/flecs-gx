#ifndef FLECS_GX_TEXT_H
#define FLECS_GX_TEXT_H

#include "geometry.h"

void gx_text_init(
    ecs_world_t *world,
    gx_text_t *ctx,
    GxCanvas *canvas);

void gx_text_draw(
    ecs_world_t *world,
    gx_text_t *text,
    GxCanvas *canvas);

#endif
