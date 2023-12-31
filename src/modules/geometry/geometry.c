#define FLECS_GX_GEOMETRY_IMPL
#include "geometry.h"
#include "geometry2.h"
#include "rounded_rect.h"
#include "text.h"

void gx_geometry_matvp_make(
    mat4 mat_vp, 
    gx_viewport_t *vp) 
{
    glm_ortho(vp->left, vp->right, vp->top, vp->bottom, -1.0, 1.0, mat_vp);
}

gx_geometry_t* gx_geometry_init(
    ecs_world_t *world,
    GxCanvas *canvas)
{
    gx_geometry_t *ctx = ecs_os_calloc_t(gx_geometry_t);

    gx_geometry2_init(world, &ctx->geometry2);
    gx_rounded_rect_init(world, &ctx->rounded_rect);
    gx_text_init(world, &ctx->text, canvas);

    return ctx;
}

void gx_geometry_populate(
    ecs_world_t *world,
    gx_geometry_t*ctx,
    GxCanvas *canvas)
{
    gx_geometry2_populate(world, &ctx->geometry2, canvas);
    gx_rounded_rect_populate(world, &ctx->rounded_rect, canvas);
}

void gx_geometry_draw(
    ecs_world_t *world,
    GxCanvas *canvas)
{
    gx_geometry_t *ctx = canvas->geometry;

    gx_geometry2_draw(&ctx->geometry2, canvas);
    gx_rounded_rect_draw(world, &ctx->rounded_rect, canvas);
    gx_text_draw(world, &ctx->text, canvas);
}

static
void AddGxTransformComputed(ecs_iter_t *it) {
    for (int32_t i = 0; i < it->count; i ++) {
        ecs_set(it->world, it->entities[i], GxTransform2Computed, {0});
    }
}

static
void AddGxStyleComputed(ecs_iter_t *it) {
    for (int32_t i = 0; i < it->count; i ++) {
        ecs_set(it->world, it->entities[i], GxStyleComputed, {0});
    }
}

void FlecsGxGeometryImport(
    ecs_world_t *world)
{
    ECS_MODULE(world, FlecsGxGeometry);

    ECS_META_COMPONENT(world, GxTransform2Computed);
    ECS_META_COMPONENT(world, GxStyleComputed);

    ECS_SYSTEM(world, AddGxTransformComputed, EcsOnLoad,
        flecs.components.transform.Position2 ||
        flecs.components.gui.Align ||
        flecs.components.geometry.Rectangle ||
        flecs.components.geometry.Line2,
        !GxTransform2Computed,
        !(flecs.meta.Constant, *)); // filter out Align enum constants

    ECS_SYSTEM(world, AddGxStyleComputed, EcsOnLoad,
        flecs.components.geometry.Rectangle ||
        flecs.components.geometry.Line2,
        !GxStyleComputed);

    gx_geometry2_import(world);
}
