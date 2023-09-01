#include "canvas.h"
#include "draw2d.h"

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

    ecs_world_t *world = it->world;
    GxCanvas *rc = ecs_get_mut(world, it->entities[0], GxCanvas);
    rc->width = 0;
    rc->height = 0;
    gx_draw2d_init(world, rc);

    ecs_modified(world, it->entities[0], GxCanvas);
}

static
void GxCanvasPopulateGeometry2D(ecs_iter_t *it) {
    gx_draw2d_populate_geometry(it->world, ecs_field(it, GxCanvas, 2));
}

static
void GxCanvasBeginDraw(ecs_iter_t *it) {
    EcsCanvas *c = ecs_field(it, EcsCanvas, 1);
    GxCanvas *rc = ecs_field(it, GxCanvas, 2);
    int32_t prev_width = rc->width, prev_height = rc->height;
    rc->width = sapp_width();
    rc->height = sapp_height();

    if (prev_width != rc->width || prev_height != rc->height) {
        /* Resize offscreen buffers */
    }

    sg_pass_action pa = {0};
    pa.colors[0].load_action = SG_LOADACTION_CLEAR;
    pa.colors[0].clear_value.r = c->background_color.r;
    pa.colors[0].clear_value.g = c->background_color.g;
    pa.colors[0].clear_value.b = c->background_color.b;
    pa.colors[0].clear_value.a = 1.0;
    sg_begin_default_pass(&pa, rc->width, rc->height);
}

static
void GxCanvasDraw2D(ecs_iter_t *it) {
    gx_draw2d_draw(it->world, ecs_field(it, GxCanvas, 2));
}

static
void GxCanvasEndDraw(ecs_iter_t *it) {
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

    ECS_SYSTEM(world, GxCanvasPopulateGeometry2D, EcsOnStore,
        flecs.components.gui.Canvas, Canvas);

    ECS_SYSTEM(world, GxCanvasBeginDraw, EcsOnStore, 
        flecs.components.gui.Canvas, Canvas);

    ECS_SYSTEM(world, GxCanvasDraw2D, EcsOnStore,
        flecs.components.gui.Canvas, Canvas);

    ECS_SYSTEM(world, GxCanvasEndDraw, EcsOnStore, 
        flecs.components.gui.Canvas, Canvas);

    ECS_SYSTEM(world, GxCanvasCommit, EcsOnStore, 0);
}
