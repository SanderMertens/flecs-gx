#ifndef _MSC_VER
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#ifndef __APPLE__
#define SOKOL_IMPL
#define SOKOL_GL_IMPL
#define SOKOL_FETCH_IMPL
#define FONTSTASH_IMPLEMENTATION
#endif

#define SOKOL_NO_ENTRY
#include "app.h"

typedef struct {
    ecs_world_t *world;
    ecs_app_desc_t *desc;
} app_ctx_t;

static app_ctx_t app_ctx;

/* Convert keycodes from sokol to flecs.components.input */
static
int app_key_code(
    int sg_key) 
{
    switch(sg_key) {
    case SAPP_KEYCODE_SPACE: return ECS_KEY_SPACE;
    case SAPP_KEYCODE_APOSTROPHE: return ECS_KEY_APOSTROPHE;
    case SAPP_KEYCODE_COMMA: return ECS_KEY_COMMA;
    case SAPP_KEYCODE_MINUS: return ECS_KEY_MINUS;
    case SAPP_KEYCODE_PERIOD: return ECS_KEY_PERIOD;
    case SAPP_KEYCODE_SLASH: return ECS_KEY_SLASH;
    case SAPP_KEYCODE_0: return ECS_KEY_0;
    case SAPP_KEYCODE_1: return ECS_KEY_1;
    case SAPP_KEYCODE_2: return ECS_KEY_2;
    case SAPP_KEYCODE_3: return ECS_KEY_3;
    case SAPP_KEYCODE_4: return ECS_KEY_4;
    case SAPP_KEYCODE_5: return ECS_KEY_5;
    case SAPP_KEYCODE_6: return ECS_KEY_6;
    case SAPP_KEYCODE_7: return ECS_KEY_7;
    case SAPP_KEYCODE_8: return ECS_KEY_8;
    case SAPP_KEYCODE_9: return ECS_KEY_9;
    case SAPP_KEYCODE_SEMICOLON: return ECS_KEY_SEMICOLON;
    case SAPP_KEYCODE_EQUAL: return ECS_KEY_EQUAL;
    case SAPP_KEYCODE_A: return ECS_KEY_A;
    case SAPP_KEYCODE_B: return ECS_KEY_B;
    case SAPP_KEYCODE_C: return ECS_KEY_C;
    case SAPP_KEYCODE_D: return ECS_KEY_D;
    case SAPP_KEYCODE_E: return ECS_KEY_E;
    case SAPP_KEYCODE_F: return ECS_KEY_F;
    case SAPP_KEYCODE_G: return ECS_KEY_G;
    case SAPP_KEYCODE_H: return ECS_KEY_H;
    case SAPP_KEYCODE_I: return ECS_KEY_I;
    case SAPP_KEYCODE_J: return ECS_KEY_J;
    case SAPP_KEYCODE_K: return ECS_KEY_K;
    case SAPP_KEYCODE_L: return ECS_KEY_L;
    case SAPP_KEYCODE_M: return ECS_KEY_M;
    case SAPP_KEYCODE_N: return ECS_KEY_N;
    case SAPP_KEYCODE_O: return ECS_KEY_O;
    case SAPP_KEYCODE_P: return ECS_KEY_P;
    case SAPP_KEYCODE_Q: return ECS_KEY_Q;
    case SAPP_KEYCODE_R: return ECS_KEY_R;
    case SAPP_KEYCODE_S: return ECS_KEY_S;
    case SAPP_KEYCODE_T: return ECS_KEY_T;
    case SAPP_KEYCODE_U: return ECS_KEY_U;
    case SAPP_KEYCODE_V: return ECS_KEY_V;
    case SAPP_KEYCODE_W: return ECS_KEY_W;
    case SAPP_KEYCODE_X: return ECS_KEY_X;
    case SAPP_KEYCODE_Y: return ECS_KEY_Y;
    case SAPP_KEYCODE_Z: return ECS_KEY_Z;
    case SAPP_KEYCODE_LEFT_BRACKET: return ECS_KEY_LEFT_BRACKET;
    case SAPP_KEYCODE_BACKSLASH: return ECS_KEY_BACKSLASH;
    case SAPP_KEYCODE_RIGHT_BRACKET: return ECS_KEY_RIGHT_BRACKET;
    case SAPP_KEYCODE_GRAVE_ACCENT: return ECS_KEY_GRAVE_ACCENT;
    case SAPP_KEYCODE_ESCAPE: return ECS_KEY_ESCAPE;
    case SAPP_KEYCODE_ENTER: return ECS_KEY_RETURN;
    case SAPP_KEYCODE_TAB: return ECS_KEY_TAB;
    case SAPP_KEYCODE_BACKSPACE: return ECS_KEY_BACKSPACE;
    case SAPP_KEYCODE_INSERT: return ECS_KEY_INSERT;
    case SAPP_KEYCODE_DELETE: return ECS_KEY_DELETE;
    case SAPP_KEYCODE_RIGHT: return ECS_KEY_RIGHT;
    case SAPP_KEYCODE_LEFT: return ECS_KEY_LEFT;
    case SAPP_KEYCODE_DOWN: return ECS_KEY_DOWN;
    case SAPP_KEYCODE_UP: return ECS_KEY_UP;
    case SAPP_KEYCODE_PAGE_UP: return ECS_KEY_PAGE_UP;
    case SAPP_KEYCODE_PAGE_DOWN: return ECS_KEY_PAGE_DOWN;
    case SAPP_KEYCODE_HOME: return ECS_KEY_HOME;
    case SAPP_KEYCODE_END: return ECS_KEY_END;
    case SAPP_KEYCODE_LEFT_SHIFT: return ECS_KEY_LEFT_SHIFT;
    case SAPP_KEYCODE_LEFT_CONTROL: return ECS_KEY_LEFT_CTRL;
    case SAPP_KEYCODE_LEFT_ALT: return ECS_KEY_LEFT_ALT;
    case SAPP_KEYCODE_RIGHT_SHIFT: return ECS_KEY_RIGHT_SHIFT;
    case SAPP_KEYCODE_RIGHT_CONTROL: return ECS_KEY_RIGHT_CTRL;
    case SAPP_KEYCODE_RIGHT_ALT: return ECS_KEY_RIGHT_ALT;
    default:
        return ECS_KEY_UNKNOWN;
    }
}

static
ecs_key_state_t* app_key_get(
    EcsInput *input, 
    int key_code) 
{
    return &input->keys[key_code];
}

static
void app_key_down(
    ecs_key_state_t *key)
{
    if (key->state) {
        key->down = false;
    } else {
        key->down = true;
    }

    key->state = true;
    key->current = true;
}

static
void app_key_up(
    ecs_key_state_t *key)
{
    key->up = true;
    key->current = false;
}

static
void app_key_reset(
    ecs_key_state_t *state)
{
    if (!state->current) {
        state->state = 0;
        state->down = 0;
        state->up = 0;
    } else if (state->state) {
        state->down = 0;
        state->up = 0;
    }
}

static
void app_keys_reset(
    EcsInput *input)
{
    int k;
    for (k = 0; k < 128; k ++) {
        app_key_reset(&input->keys[k]);
    }
}

static
void app_mouse_down(
    ecs_key_state_t *mouse)
{
    if (mouse->state) {
        mouse->down = false;
    } else {
        mouse->down = true;
    }

    mouse->state = true;
    mouse->current = true;
}

static
void app_mouse_up(
    ecs_key_state_t *mouse)
{
    mouse->up = true;
    mouse->current = false;
}

static
void app_mouse_button_reset(
    ecs_key_state_t *mouse)
{
    if (!mouse->current) {
        mouse->state = 0;
        mouse->up = 0;
        mouse->down = 0;
    } else if (mouse->state) {
        mouse->up = 0;
        mouse->down = 0;
    }
}

static
void app_mouse_reset(
    EcsInput *input)
{
    app_mouse_button_reset(&input->mouse.left);
    app_mouse_button_reset(&input->mouse.right);
    input->mouse.moved = false;
}

static
ecs_entity_t app_get_canvas(
    const ecs_world_t *world) 
{
    ecs_entity_t result = 0;

    ecs_iter_t it = ecs_term_iter(world, &(ecs_term_t) {
        .id = ecs_id(EcsCanvas)
    });

    if (ecs_term_next(&it)) {
        result = it.entities[0];
        ecs_iter_fini(&it);
    }

    return result;
}

static
void app_input_action(
    const sapp_event* evt, 
    app_ctx_t *ctx) 
{
    ecs_world_t *world = ctx->world;
    EcsInput *input = ecs_singleton_get_mut(world, EcsInput);

    switch (evt->type) {
    case SAPP_EVENTTYPE_MOUSE_DOWN:
        if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            app_mouse_down(&input->mouse.left);
        }
        
        if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
            app_mouse_down(&input->mouse.right);
        }
        break;
    case SAPP_EVENTTYPE_MOUSE_UP:
        if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            app_mouse_up(&input->mouse.left);
        }
        
        if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
            app_mouse_up(&input->mouse.right);
        }
        break;
    case SAPP_EVENTTYPE_MOUSE_MOVE: {
        float x = evt->mouse_x, y = evt->mouse_y;
        int f_w = evt->framebuffer_width, f_h = evt->framebuffer_height;

        if ((x > 0) && (y > 0) && (x < f_w) && (y < f_h)) {
            float x_scale = (float)evt->window_width / (float)evt->framebuffer_width;
            float y_scale = (float)evt->window_height / (float)evt->framebuffer_height;

            input->mouse.moved = true;
            input->mouse.window.x = evt->mouse_x * x_scale;
            input->mouse.window.y = evt->mouse_y * y_scale;
            input->mouse.view.x = input->mouse.window.x - ((float)evt->window_width / 2.0);
            input->mouse.view.y = input->mouse.window.y - ((float)evt->window_height / 2.0);
        }
        break;
    }
    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        break;
    case SAPP_EVENTTYPE_KEY_UP:
        app_key_up(app_key_get(input, app_key_code(evt->key_code)));
        break;
    case SAPP_EVENTTYPE_KEY_DOWN:
        app_key_down(app_key_get(input, app_key_code(evt->key_code)));
        break;
    case SAPP_EVENTTYPE_RESIZED: {
        ecs_entity_t canvas = app_get_canvas(world);
        EcsCanvas *canvas_data = ecs_get_mut(world, canvas, EcsCanvas);
        canvas_data->width = evt->window_width;
        canvas_data->height = evt->window_height;
        ecs_modified(world, canvas, EcsCanvas);
        break;
    }
    default:
        break;
    }
}

static
void app_frame_action(
    app_ctx_t *ctx) 
{
    if (ecs_should_quit(ctx->world)) {
        sapp_quit();
    }

    ecs_app_run_frame(ctx->world, ctx->desc);

    /* Reset input buffer */
    EcsInput *input = ecs_singleton_get_mut(ctx->world, EcsInput);
    app_keys_reset(input);
    app_mouse_reset(input);
}

static
int app_run_action(
    ecs_world_t *world,
    ecs_app_desc_t *desc)
{    
    app_ctx = (app_ctx_t){
        .world = world,
        .desc = desc
    };

    const char *title = "Flecs App";
    int width = 800, height = 600;

    /* Find canvas instance for width, height & title */
    ecs_entity_t canvas = app_get_canvas(world);
    if (canvas) {
        EcsCanvas *canvas_data = ecs_get_mut(world, canvas, EcsCanvas);
        if (canvas_data->width) width = canvas_data->width;
        if (canvas_data->height) height = canvas_data->height;
        if (!canvas_data->width || !canvas_data->height) {
            canvas_data->width = width;
            canvas_data->height = height;
            ecs_modified(world, canvas, EcsCanvas);
        }
        title = canvas_data->title;
    }

    bool high_dpi = true;
#ifdef __EMSCRIPTEN__
    high_dpi = false; /* high dpi doesn't work on mobile browsers */
#endif

    /* Initialize input component */
    ecs_singleton_set(world, EcsInput, { 0 });

    ecs_trace("renderer: starting app '%s'", title);

    /* Run app */
    sapp_run(&(sapp_desc) {
        .frame_userdata_cb = (void(*)(void*))app_frame_action,
        .event_userdata_cb = (void(*)(const sapp_event*, void*))app_input_action,
        .user_data = &app_ctx,
        .window_title = title,
        .width = width,
        .height = height,
        .sample_count = 1,
        .high_dpi = high_dpi
    });

    return 0;
}

void FlecsGxAppImport(
    ecs_world_t *world)
{
    ECS_MODULE(world, FlecsGxApp);
    ECS_IMPORT(world, FlecsComponentsGui);
    ECS_IMPORT(world, FlecsComponentsInput);

    ecs_set_name_prefix(world, "Sokol");

    ecs_app_set_run_action(app_run_action);
}
