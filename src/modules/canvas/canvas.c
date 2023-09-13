#include "canvas.h"
#include "../geometry/geometry.h"

ECS_COMPONENT_DECLARE(GxCanvas);

static
void GxCanvasInit(ecs_iter_t *it) {
    if (it->count > 1) {
        ecs_abort(ECS_INVALID_OPERATION, 
            "only one canvas component per world is supported");
        return;
    }

    sg_setup(&(sg_desc) {
        .context.depth_format = SG_PIXELFORMAT_NONE,
        .buffer_pool_size = 16384,
        .logger = { slog_func }
    });

    sgl_setup(&(sgl_desc_t){
        .logger.func = slog_func
    });

    sfetch_setup(&(sfetch_desc_t){
        .num_channels = 1,
        .num_lanes = 4,
        .logger.func = slog_func,
    });

    ecs_world_t *world = it->world;
    GxCanvas *rc = ecs_get_mut(world, it->entities[0], GxCanvas);
    rc->width = 0;
    rc->height = 0;
    rc->geometry = gx_geometry_init(world, rc);
    rc->dpi_scale = sapp_dpi_scale();

    ecs_modified(world, it->entities[0], GxCanvas);
}

static
void GxFetch(ecs_iter_t *it) {
    sfetch_dowork();
}

static
void GxCanvasPopulateGeometry2D(ecs_iter_t *it) {
    GxCanvas *canvas = ecs_field(it, GxCanvas, 2);
    gx_geometry_populate(it->world, canvas->geometry, canvas);
}

static
void GxCanvasBeginDraw(ecs_iter_t *it) {
    EcsCanvas *c = ecs_field(it, EcsCanvas, 1);
    GxCanvas *rc = ecs_field(it, GxCanvas, 2);
    int32_t prev_width = rc->width, prev_height = rc->height;
    rc->actual_width = sapp_width();
    rc->actual_height = sapp_height();
    rc->width = rc->actual_width / rc->dpi_scale;
    rc->height = rc->actual_height / rc->dpi_scale;

    rc->viewport.left = 0;
    rc->viewport.right = rc->width;
    rc->viewport.top = rc->height;
    rc->viewport.bottom = 0;

    if (prev_width != rc->width || prev_height != rc->height) {
        /* Resize offscreen buffers */
    }

    sg_pass_action pa = {0};
    pa.colors[0].load_action = SG_LOADACTION_CLEAR;
    pa.colors[0].clear_value.r = c->background_color.r;
    pa.colors[0].clear_value.g = c->background_color.g;
    pa.colors[0].clear_value.b = c->background_color.b;
    pa.colors[0].clear_value.a = 1.0;
    sg_begin_default_pass(&pa, rc->actual_width, rc->actual_height);
}

static
void GxCanvasDrawGeometry(ecs_iter_t *it) {
    gx_geometry_draw(it->world, ecs_field(it, GxCanvas, 2));
}

static
void GxCanvasEndDraw(ecs_iter_t *it) {
    sgl_draw();
    sg_end_pass();
}

static
void GxCanvasCommit(ecs_iter_t *it) {
    sg_commit();
}

void FlecsGxCanvasImport(
    ecs_world_t *world)
{
    ECS_MODULE(world, FlecsGxCanvas);
    ECS_IMPORT(world, FlecsComponentsGui);
    ECS_IMPORT(world, FlecsGxGeometry);

    ecs_set_name_prefix(world, "Gx");

    ECS_COMPONENT_DEFINE(world, GxCanvas);

    ecs_struct(world, {
        .entity = ecs_id(GxCanvas),
        .members = {
            {"width", ecs_id(ecs_i32_t)},
            {"height", ecs_id(ecs_i32_t)}
        }
    });

    ECS_SYSTEM(world, GxCanvasInit, EcsOnStart, 
        flecs.components.gui.Canvas);

    ecs_system(world, {
        .entity = ecs_id(GxCanvasInit),
        .no_readonly = true
    });

    ECS_SYSTEM(world, GxFetch, EcsOnLoad, 0);

    ECS_SYSTEM(world, GxCanvasPopulateGeometry2D, EcsOnStore,
        flecs.components.gui.Canvas, Canvas);

    ECS_SYSTEM(world, GxCanvasBeginDraw, EcsOnStore, 
        flecs.components.gui.Canvas, Canvas);

    ECS_SYSTEM(world, GxCanvasDrawGeometry, EcsOnStore,
        flecs.components.gui.Canvas, Canvas);

    ECS_SYSTEM(world, GxCanvasEndDraw, EcsOnStore, 
        flecs.components.gui.Canvas, Canvas);

    ECS_SYSTEM(world, GxCanvasCommit, EcsOnStore, 0);
}
