#include "text.h"

static 
int gx_round_pow2(float v) {
    uint32_t vi = ((uint32_t) v) - 1;
    for (uint32_t i = 0; i < 5; i++) {
        vi |= (vi >> (1 << i));
    }
    return (int)(vi + 1);
}

static 
void gx_font_regular_loaded(const sfetch_response_t* response) {
    if (response->fetched) {
        gx_text_t *ctx = *(gx_text_t**)response->user_data;
        ctx->font_regular = fonsAddFontMem(ctx->fs, "sans", 
            (void*)response->data.ptr, (int)response->data.size,  false);
    }
}

static 
void gx_font_bold_loaded(const sfetch_response_t* response) {
    if (response->fetched) {
        gx_text_t *ctx = *(gx_text_t**)response->user_data;
        ctx->font_bold = fonsAddFontMem(ctx->fs, "sans", 
            (void*)response->data.ptr, (int)response->data.size,  false);
    }
}

static 
void gx_font_italic_loaded(const sfetch_response_t* response) {
    if (response->fetched) {
        gx_text_t *ctx = *(gx_text_t**)response->user_data;
        ctx->font_italic = fonsAddFontMem(ctx->fs, "sans", 
            (void*)response->data.ptr, (int)response->data.size,  false);
    }
}

void gx_text_init(
    ecs_world_t *world,
    gx_text_t *ctx,
    GxCanvas *canvas)
{
    int atlas_dim = gx_round_pow2(512.0f * canvas->dpi_scale);

    ctx->query = ecs_query(world, {
        .filter.terms = {
            { .id = ecs_id(EcsText), .inout = EcsIn },
            { .id = ecs_id(EcsPosition2), .inout = EcsIn },
            { .id = ecs_id(EcsRgb), .inout = EcsIn, .oper = EcsOptional },
            { .id = ecs_id(EcsFontSize), .inout = EcsIn, .oper = EcsOptional },
            { .id = ecs_pair_t(EcsFontStyle, EcsWildcard), .oper = EcsOptional }
        }
    });

    FONScontext* fons_context = sfons_create(&(sfons_desc_t){
        .width = atlas_dim,
        .height = atlas_dim,
    });

    ctx->fs = fons_context;
    ctx->font_regular = -1;
    ctx->font_bold = -1;
    ctx->font_italic = -1;

    sfetch_send(&(sfetch_request_t){
        .path = "etc/flecs-gx/fonts/DroidSerif-Regular.ttf",
        .callback = gx_font_regular_loaded,
        .buffer = SFETCH_RANGE(ctx->font_regular_data),
        .user_data.ptr = &ctx,
        .user_data.size = sizeof(gx_text_t*)
    });

    sfetch_send(&(sfetch_request_t){
        .path = "etc/flecs-gx/fonts/DroidSerif-Bold.ttf",
        .callback = gx_font_bold_loaded,
        .buffer = SFETCH_RANGE(ctx->font_bold_data),
        .user_data.ptr = &ctx,
        .user_data.size = sizeof(gx_text_t*)
    });

    sfetch_send(&(sfetch_request_t){
        .path = "etc/flecs-gx/fonts/DroidSerif-Italic.ttf",
        .callback = gx_font_italic_loaded,
        .buffer = SFETCH_RANGE(ctx->font_italic_data),
        .user_data.ptr = &ctx,
        .user_data.size = sizeof(gx_text_t*)
    });
}

void gx_text_draw(
    ecs_world_t *world,
    gx_text_t *ctx,
    GxCanvas *canvas)
{
    float dpi = canvas->dpi_scale;
    gx_viewport_t *vp = &canvas->viewport;
    int32_t default_size = 32.0f;
    uint32_t white = sfons_rgba(255, 255, 255, 255);
    fonsClearState(ctx->fs);

    sgl_defaults();
    sgl_matrix_mode_projection();
    sgl_ortho(vp->left * dpi, vp->right * dpi, vp->top * dpi, vp->bottom * dpi,
         -1.0f, +1.0f);
        
    fonsSetAlign(ctx->fs, FONS_ALIGN_CENTER | FONS_ALIGN_MIDDLE);

    ecs_iter_t it = ecs_query_iter(world, ctx->query);
    while (ecs_iter_next(&it)) {
        EcsText *text = ecs_field(&it, EcsText, 1);
        EcsPosition2 *position = ecs_field(&it, EcsPosition2, 2);
        EcsRgb *rgb = ecs_field(&it, EcsRgb, 3);
        EcsFontSize *font_size = ecs_field(&it, EcsFontSize, 4);

        EcsFontStyle fs = EcsFontStyleRegular;
        if (ecs_field_is_set(&it, 5)) {
            ecs_entity_t constant = ecs_pair_second(world, ecs_field_id(&it, 5));
            fs = *ecs_get_pair_second(world, constant, EcsConstant, ecs_i32_t);
        }

        int font = -1;
        switch(fs) {
        case EcsFontStyleRegular:
            font = ctx->font_regular;
            break;
        case EcsFontStyleBold:
            font = ctx->font_bold;
            break;
        case EcsFontStyleItalic:
            font = ctx->font_italic;
            break;
        };

        if (font == -1) {
            continue; // font isn't loaded
        }

        fonsSetFont(ctx->fs, font);

        for (int32_t i = 0; i < it.count; i ++) {
            int32_t size = default_size;
            if (font_size) {
                size = font_size[i].value;
            }
            fonsSetSize(ctx->fs, size * dpi);

            uint32_t color = white;
            if (rgb) {
                color = sfons_rgba(rgb[i].r * 255, 
                    rgb[i].g * 255, rgb[i].b * 255, 255);
                
            }
            fonsSetColor(ctx->fs, color);

            float x = position[i].x * dpi;
            float y = position[i].y * dpi;
            fonsDrawText(ctx->fs, x, y, text[i].value, NULL);
        }
    }

    sfons_flush(ctx->fs);
}
