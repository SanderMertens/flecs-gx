#include "event.h"

static
bool gx_is_point_inside(
    GxTransform2Computed *t,
    float x, 
    float y)
{
    float wh = t->scale[0] / 2;
    float hh = t->scale[1] / 2;
    float left = t->position[0] - wh;
    float right = t->position[0] + wh;
    float top = t->position[1] - hh;
    float bottom = t->position[1] + hh;

    return x > left && x < right && y > top && y < bottom;
}

static
void gx_event_emit(
    ecs_world_t *world,
    ecs_entity_t entity,
    EcsInputState *state,
    const EcsEventListener *listener,
    ecs_iter_action_t callback)
{
    ecs_iter_t it = {0};

    it.real_world = ECS_CONST_CAST(ecs_world_t*, ecs_get_world(world));
    it.world = world;
    it.table = ecs_get_table(world, entity);
    it.count = 1;
    it.entities = (ecs_entity_t[]){ entity };
    it.ids = (ecs_id_t[]){ ecs_id(EcsInputState) };
    it.columns = (int32_t[]){ 1 };
    it.sources = (ecs_entity_t[]){ 0 };
    it.ptrs = (void*[]){ state };
    it.binding_ctx = listener->binding_ctx;
    it.ctx = listener->ctx;
    
    callback(&it); // callback in EventListener
}

static
bool gx_on_mouse_move(
    ecs_world_t *world,
    ecs_entity_t entity,
    GxTransform2Computed *t,
    EcsEventListener *l,
    EcsInputState *s,
    float mouse_x,
    float mouse_y,
    bool entered)
{
    bool mouse_inside = gx_is_point_inside(t, mouse_x, mouse_y);

    if (mouse_inside || s->drag) {
        s->mouse_x = mouse_x;
        s->mouse_y = mouse_y;
    }

    if (mouse_inside) {
        if (!entered && !s->hover) {
            s->hover = true;
            if (l->on_enter) {
                gx_event_emit(world, entity, s, l, l->on_enter);
                entered = true;
            }
        } else {
            if (entered) {
                /* Can't hover over two elements at the same time */
                if (s->hover && l->on_leave) {
                    s->hover = false;
                    gx_event_emit(world, entity, s, l, l->on_leave);
                }
            } else {
                entered = true;
            }
        }
    }

    if (s->drag) {
        gx_event_emit(world, entity, s, l, l->on_drag);
    } else if (s->hover && s->lmb_down && l->on_drag) {
        /* Set drag anchor for more precise control over position 
         * while dragging. */
        s->drag_anchor_x = mouse_x - t->position[0];
        s->drag_anchor_y = mouse_y - t->position[1];
        gx_event_emit(world, entity, s, l, l->on_drag);
    }

    /* If mouse is not inside element and we're not dragging, lose hover */
    if (!mouse_inside && !s->drag) {
        if (s->hover) {
            s->hover = false;
            s->lmb_down = false;
            s->rmb_down = false;
            if (l->on_leave) {
                gx_event_emit(world, entity, s, l, l->on_leave);
            }
        }
    }

    return entered;
}

static
bool gx_on_mouse_left_down(
    ecs_world_t *world,
    ecs_entity_t entity,
    GxTransform2Computed *t,
    EcsEventListener *l,
    EcsInputState *s,
    float mouse_x,
    float mouse_y)
{
    if (gx_is_point_inside(t, mouse_x, mouse_y)) {
        s->lmb_down = true;
        if (l->on_lmb_down) {
            gx_event_emit(world, entity, s, l, l->on_lmb_down);
            return true;
        }
    }
    return false;
}

static
void gx_on_mouse_left_up(
    ecs_world_t *world,
    ecs_entity_t entity,
    GxTransform2Computed *t,
    EcsEventListener *l,
    EcsInputState *s,
    float mouse_x,
    float mouse_y)
{
    if (s->lmb_down) {
        s->lmb_down = false;
        s->drag = false;
        if (gx_is_point_inside(t, mouse_x, mouse_y) && l->on_lmb_up) {
            gx_event_emit(world, entity, s, l, l->on_lmb_up);
        }
    }
}

static
bool gx_on_mouse_right_down(
    ecs_world_t *world,
    ecs_entity_t entity,
    GxTransform2Computed *t,
    EcsEventListener *l,
    EcsInputState *s,
    float mouse_x,
    float mouse_y)
{
    if (gx_is_point_inside(t, mouse_x, mouse_y)) {
        s->rmb_down = true;
        if (l->on_rmb_down) {
            gx_event_emit(world, entity, s, l, l->on_rmb_down);
            return true;
        }
    }
    return false;
}

static
void gx_on_mouse_right_up(
    ecs_world_t *world,
    ecs_entity_t entity,
    GxTransform2Computed *t,
    EcsEventListener *l,
    EcsInputState *s,
    float mouse_x,
    float mouse_y)
{
    if (s->rmb_down) {
        s->rmb_down = false;
        if (gx_is_point_inside(t, mouse_x, mouse_y) && l->on_rmb_up) {
            gx_event_emit(world, entity, s, l, l->on_rmb_up);
        }
    }
}

static
void Dispatch(ecs_iter_t *system_it) {
    ecs_query_t *dispatch_query = system_it->ctx;
    ecs_world_t *world = system_it->world;
    EcsInput *input = ecs_field(system_it, EcsInput, 1);

    ecs_mouse_state_t *mouse = &input->mouse;
    if (!mouse->moved && !mouse->left.down && !mouse->left.up &&
        !mouse->right.down && !mouse->right.up)
    {
        return;
    }

    bool entered = false, lmb = false, rmb = false;

    ecs_iter_t it = ecs_query_iter(world, dispatch_query);
    while (ecs_query_next(&it)) {
        ecs_entity_t *entities = it.entities;
        GxTransform2Computed *transform = ecs_field(&it, GxTransform2Computed, 1);
        EcsEventListener *listeners = ecs_field(&it, EcsEventListener, 2);
        EcsInputState *input_state = ecs_field(&it, EcsInputState, 3);

        if (mouse->moved) {
            /* Mouse move events (initiates drag if combined with LMB down) */
            for (int32_t i = it.count - 1; i >= 0; i --) {
                entered |= gx_on_mouse_move(world, entities[i], 
                    &transform[i], &listeners[i], &input_state[i], 
                    input->mouse.view.x, input->mouse.view.y, entered);
            }
        }

        /* Left mouse button down */
        if (mouse->left.down && !lmb) {
            for (int32_t i = it.count - 1; i >= 0; i --) {
                if ((lmb = gx_on_mouse_left_down(world, entities[i], 
                    &transform[i], &listeners[i], &input_state[i], 
                    input->mouse.view.x, input->mouse.view.y)))
                {
                    break;
                }
            }
        }

        /* Left mouse button up */
        if (mouse->left.up) {
            for (int32_t i = it.count - 1; i >= 0; i --) {
                gx_on_mouse_left_up(world, entities[i], 
                    &transform[i], &listeners[i], &input_state[i], 
                    input->mouse.view.x, input->mouse.view.y);
            }
        }

        /* Right mouse button down */
        if (mouse->right.down && !rmb) {
            for (int32_t i = it.count - 1; i >= 0; i --) {
                if ((rmb = gx_on_mouse_right_down(world, entities[i], 
                    &transform[i], &listeners[i], &input_state[i], 
                    input->mouse.view.x, input->mouse.view.y)))
                {
                    break;
                }
            }
        }

        /* Right mouse button up */
        if (mouse->right.up) {
            for (int32_t i = it.count - 1; i >= 0; i --) {
                gx_on_mouse_right_up(world, entities[i], 
                    &transform[i], &listeners[i], &input_state[i], 
                    input->mouse.view.x, input->mouse.view.y);
            }
        }
    }

    /* Fallback on singleton handler in case no matching entity was found */
    if (mouse->left.down && !lmb) {
        const EcsEventListener *l = ecs_singleton_get(world, EcsEventListener);
        if (l && l->on_lmb_down) {
            EcsInputState *input_state = 
                ecs_get_mut(world, ecs_id(EcsEventListener), EcsInputState);
            gx_event_emit(world, ecs_id(EcsEventListener), input_state, 
                l, l->on_lmb_down);
        }
    }
}

void FlecsGxEventImport(
    ecs_world_t *world)
{
    ECS_MODULE(world, FlecsGxEvent);

    ecs_query_t *dispatch_query = ecs_query(world, {
        .filter.terms = {
            { .id = ecs_id(GxTransform2Computed), .inout = EcsIn },
            { .id = ecs_id(EcsEventListener),     .inout = EcsIn },
            { .id = ecs_id(EcsInputState),        .inout = EcsIn },
            { .id = ecs_id(GxTransform2Computed), .inout = EcsInOutNone, .src = {
                    .flags = EcsCascade|EcsDesc,
                    .trav = EcsChildOf
                },
                .oper = EcsOptional
            }
        },
        .filter.instanced = true
    });

    ecs_system(world, {
        .entity = ecs_entity(world, {
            .name = "Dispatch",
            .add = { ecs_dependson(EcsPostLoad) }
        }),
        .query.filter.terms = {
            { .id = ecs_id(EcsInput), .inout = EcsIn, .src.id = EcsVariable },
        },
        .callback = Dispatch,
        .ctx = dispatch_query
    });
}
