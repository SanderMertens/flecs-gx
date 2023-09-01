#include "draw2d.h"

#define POSITION_I 0
#define COLOR_I 1
#define TRANSFORM_I 2
#define LAYOUT_I_STR(i) #i
#define LAYOUT(loc) "layout(location=" LAYOUT_I_STR(loc) ") "

typedef struct gx_draw2d_vs_uniforms_t {
    mat4 mat_vp;
} gx_draw2d_vs_uniforms_t;

typedef struct gx_draw2d_vs_data_t {
    ecs_rgb_t *colors;
    mat4 *transforms;
} gx_draw2d_vs_data_t;

static
vec3 gx_draw2d_rect_verts[] = {
    {-0.5,  0.5, 0.0},
    { 0.5,  0.5, 0.0},
    { 0.5, -0.5, 0.0},
    {-0.5, -0.5, 0.0},
};

static
uint16_t gx_draw2d_rect_idx[] = {
    0, 1, 2,
    0, 2, 3
};

static
sg_pipeline gx_draw2d_init_pipeline(void) {
    char *vs = gx_shader_from_str(
        SOKOL_SHADER_HEADER
        "uniform mat4 u_mat_vp;\n"
        LAYOUT(POSITION_I)  "in vec3 v_position;\n"
        LAYOUT(COLOR_I)     "in vec3 i_color;\n"
        LAYOUT(TRANSFORM_I) "in mat4 i_mat_m;\n"
        "#include \"etc/flecs_gx/shaders/draw2d_vert.glsl\"\n"
    );

    char *fs = gx_shader_from_str(
        SOKOL_SHADER_HEADER
        "#include \"etc/flecs_gx/shaders/draw2d_frag.glsl\"\n"
    );

    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .vs.uniform_blocks = {
            [0] = {
                .size = sizeof(gx_draw2d_vs_uniforms_t),
                .uniforms = {
                    [0] = { .name="u_mat_vp", .type=SG_UNIFORMTYPE_MAT4 },
                },
            }
        },

        .vs.source = vs,
        .fs.source = fs
    });

    return sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .layout = {
            .buffers = {
                [COLOR_I] =     { .stride = 12, .step_func = SG_VERTEXSTEP_PER_INSTANCE },
                [TRANSFORM_I] = { .stride = 64, .step_func = SG_VERTEXSTEP_PER_INSTANCE }
            },

            .attrs = {
                /* Static geometry */
                [POSITION_I] =      { .buffer_index = POSITION_I,  .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT3 },

                /* Color buffer (per instance) */
                [COLOR_I] =         { .buffer_index = COLOR_I,     .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT3 },

                /* Matrix (per instance) */
                [TRANSFORM_I] =     { .buffer_index = TRANSFORM_I, .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT4 },
                [TRANSFORM_I + 1] = { .buffer_index = TRANSFORM_I, .offset = 16, .format = SG_VERTEXFORMAT_FLOAT4 },
                [TRANSFORM_I + 2] = { .buffer_index = TRANSFORM_I, .offset = 32, .format = SG_VERTEXFORMAT_FLOAT4 },
                [TRANSFORM_I + 3] = { .buffer_index = TRANSFORM_I, .offset = 48, .format = SG_VERTEXFORMAT_FLOAT4 }
            }
        },

        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },

        .cull_mode = SG_CULLMODE_BACK,
        .sample_count = 1
    });
}

static
void gx_draw2d_matvp_make(mat4 mat_vp, int32_t width, int32_t height) {
    int32_t half_width = width / 2;
    int32_t half_height = height / 2;
    glm_ortho(-half_width, half_width, -half_height, half_height, -1.0, 1.0, mat_vp);
}

static
void gx_draw2d_geometry_collect(
    ecs_world_t *world,
    gx_geometry_2d_t *geometry)
{
    ecs_vec_reset_t(NULL, &geometry->color, ecs_rgb_t);
    ecs_vec_reset_t(NULL, &geometry->transform, mat4);

    ecs_iter_t it = ecs_query_iter(world, geometry->query);
    while (ecs_query_next(&it)) {
        EcsRectangle *r = ecs_field(&it, EcsRectangle, 1);
        EcsPosition2 *p = ecs_field(&it, EcsPosition2, 2);
        EcsRgb *c = ecs_field(&it, EcsRgb, 3);

        ecs_rgb_t *color = ecs_vec_grow_t(
            NULL, &geometry->color, ecs_rgb_t, it.count);
        mat4 *transform = ecs_vec_grow_t(
            NULL, &geometry->transform, mat4, it.count);

        // Populate color
        if (c) {
            ecs_os_memcpy_n(color, c, ecs_rgb_t, it.count);
        } else {
            ecs_os_memset_n(color, 0, ecs_rgb_t, it.count);
        }

        // Populate transform matrix
        if (p) {
            for (int32_t i = 0; i < it.count; i ++) {
                vec3 p3 = {p[i].x, p[i].y, 0.0};
                glm_translate_make(transform[i], p3);
            }
        } else {
            for (int32_t i = 0; i < it.count; i ++) {
                glm_mat4_identity(transform[i]);
            }
        }

        // Scale to dimensions of rectangle
        for (int32_t i = 0; i < it.count; i ++) {
            vec3 p3 = {r[i].width, r[i].height, 1.0};
            glm_scale(transform[i], p3);
        }
    }
}

static
void gx_draw2d_geometry_update(
    gx_geometry_2d_t *geometry)
{
    int32_t new_size = ecs_vec_count(&geometry->color);
    int32_t old_size = geometry->instance_count;
    if (new_size != old_size) {
        if (old_size) {
            sg_destroy_buffer(geometry->color_buf);
            sg_destroy_buffer(geometry->transform_buf);
        }

        geometry->color_buf = sg_make_buffer(&(sg_buffer_desc){
            .size = new_size * sizeof(ecs_rgb_t), .usage = SG_USAGE_STREAM });
        geometry->transform_buf = sg_make_buffer(&(sg_buffer_desc){
            .size = new_size * sizeof(mat4), .usage = SG_USAGE_STREAM });
        geometry->instance_count = new_size;
    }

    if (new_size > 0) {
        sg_update_buffer(geometry->color_buf, &(sg_range) {
            ecs_vec_first_t(&geometry->color, ecs_rgb_t), 
                new_size * sizeof(ecs_rgb_t) } );
        sg_update_buffer(geometry->transform_buf, &(sg_range) {
            ecs_vec_first_t(&geometry->transform, mat4), 
                new_size * sizeof(mat4) } );
    }
}

static
void gx_draw2d_geometry_init_rect(
    ecs_world_t *world,
    gx_geometry_2d_t *geometry)
{
    // Query that matches rectangle entities
    geometry->query = ecs_query(world, {
        .filter.terms = {
            { .id = ecs_id(EcsRectangle), .inout = EcsIn },
            { .id = ecs_id(EcsPosition2), .inout = EcsIn, .oper = EcsOptional },
            { .id = ecs_id(EcsRgb), .inout = EcsIn, .oper = EcsOptional }
        }
    });

    // Create vertex & index buffers for rectangle geometry
    geometry->verts = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(gx_draw2d_rect_verts),
        .label = "rect_verts"
    });

    geometry->idx = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(gx_draw2d_rect_idx),
        .label = "rect_idx",
        .type = SG_BUFFERTYPE_INDEXBUFFER
    });

    geometry->index_count = 6;
}

void gx_draw2d_init(
    ecs_world_t *world,
    GxCanvas *canvas)
{
    gx_draw2d_ctx_t *ctx = &canvas->draw2d_ctx;

    gx_draw2d_geometry_init_rect(world, &ctx->rect_data);

    ctx->pip = gx_draw2d_init_pipeline();
}

void gx_draw2d_populate_geometry(
    ecs_world_t *world,
    GxCanvas *canvas)
{
    gx_draw2d_ctx_t *ctx = &canvas->draw2d_ctx;
    gx_draw2d_geometry_collect(world, &ctx->rect_data);
    gx_draw2d_geometry_update(&ctx->rect_data);
}

void gx_draw2d_draw_geometry(
    gx_geometry_2d_t *geometry)
{
    sg_bindings bind = {
        .vertex_buffers = {
            [POSITION_I] =  geometry->verts,
            [COLOR_I] =     geometry->color_buf,
            [TRANSFORM_I] = geometry->transform_buf
        },
        .index_buffer = geometry->idx
    };

    sg_apply_bindings(&bind);
    sg_draw(0, geometry->index_count, geometry->instance_count);
}

void gx_draw2d_draw(
    ecs_world_t *world,
    GxCanvas *canvas)
{
    gx_draw2d_ctx_t *ctx = &canvas->draw2d_ctx;

    gx_draw2d_vs_uniforms_t vs_u;
    gx_draw2d_matvp_make(vs_u.mat_vp, canvas->width, canvas->height);

    sg_apply_pipeline(ctx->pip);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, 
        &(sg_range){&vs_u, sizeof(gx_draw2d_vs_uniforms_t)});


    gx_draw2d_draw_geometry(&ctx->rect_data);
}
