#include "geometry2.h"
#include "rounded_rect.h"

#define POSITION_I 0
#define COLOR_I 1
#define SIZE_I 2
#define CORNER_RADIUS_I 3
#define STROKE_COLOR_I 4
#define STROKE_WIDTH_I 5
#define TRANSFORM_I 6
#define LAYOUT_I_STR(i) #i
#define LAYOUT(loc) "layout(location=" LAYOUT_I_STR(loc) ") "

typedef struct gx_rounded_rect_vs_uniforms_t {
    mat4 mat_vp;
} gx_rounded_rect_vs_uniforms_t;

typedef struct gx_rounded_rect_fs_uniforms_t {
    float aspect;
} gx_rounded_rect_fs_uniforms_t;

static
sg_pipeline gx_rounded_rect_init_pipeline(void) {
    char *vs = gx_shader_from_str(
        SOKOL_SHADER_HEADER
        "uniform mat4 u_mat_vp;\n"
        "uniform float u_aspect;\n"
        LAYOUT(POSITION_I)      "in vec3 v_position;\n"
        LAYOUT(COLOR_I)         "in vec3 i_color;\n"
        LAYOUT(SIZE_I)          "in vec3 i_size;\n"
        LAYOUT(CORNER_RADIUS_I) "in float i_corner_radius;\n"
        LAYOUT(STROKE_COLOR_I)  "in vec3 i_stroke_color;\n"
        LAYOUT(STROKE_WIDTH_I)  "in float i_stroke_width;\n"
        LAYOUT(TRANSFORM_I)     "in mat4 i_mat_m;\n"
        "#include \"etc/flecs-gx/shaders/rounded_rect_vert.glsl\"\n"
    );

    char *fs = gx_shader_from_str(
        SOKOL_SHADER_HEADER
        "#include \"etc/flecs-gx/shaders/rounded_rect_frag.glsl\"\n"
    );

    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .vs.uniform_blocks = {
            [0] = {
                .size = sizeof(gx_rounded_rect_vs_uniforms_t),
                .uniforms = {
                    [0] = { .name="u_mat_vp", .type=SG_UNIFORMTYPE_MAT4 },
                },
            }
        },

        .fs.uniform_blocks = {
            [0] = {
                .size = sizeof(gx_rounded_rect_fs_uniforms_t),
                .uniforms = {
                    [0] = { .name="u_aspect", .type=SG_UNIFORMTYPE_FLOAT }
                }
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
                [COLOR_I]          = { .stride = 12, .step_func = SG_VERTEXSTEP_PER_INSTANCE },
                [SIZE_I]           = { .stride = 8, .step_func = SG_VERTEXSTEP_PER_INSTANCE },
                [CORNER_RADIUS_I]  = { .stride = 4, .step_func = SG_VERTEXSTEP_PER_INSTANCE },
                [STROKE_COLOR_I]   = { .stride = 12, .step_func = SG_VERTEXSTEP_PER_INSTANCE },
                [STROKE_WIDTH_I]   = { .stride = 4, .step_func = SG_VERTEXSTEP_PER_INSTANCE },
                [TRANSFORM_I]      = { .stride = 64, .step_func = SG_VERTEXSTEP_PER_INSTANCE }
            },

            .attrs = {
                /* Static geometry */
                [POSITION_I]      = { .buffer_index = POSITION_I,      .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT3 },

                /* Per instance */
                [COLOR_I]         = { .buffer_index = COLOR_I,         .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT3 },
                [SIZE_I]          = { .buffer_index = SIZE_I,          .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT2 },
                [CORNER_RADIUS_I] = { .buffer_index = CORNER_RADIUS_I, .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT },
                [STROKE_COLOR_I]  = { .buffer_index = STROKE_COLOR_I,  .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT3 },
                [STROKE_WIDTH_I]  = { .buffer_index = STROKE_WIDTH_I,  .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT },

                [TRANSFORM_I]     = { .buffer_index = TRANSFORM_I, .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT4 },
                [TRANSFORM_I + 1] = { .buffer_index = TRANSFORM_I, .offset = 16, .format = SG_VERTEXFORMAT_FLOAT4 },
                [TRANSFORM_I + 2] = { .buffer_index = TRANSFORM_I, .offset = 32, .format = SG_VERTEXFORMAT_FLOAT4 },
                [TRANSFORM_I + 3] = { .buffer_index = TRANSFORM_I, .offset = 48, .format = SG_VERTEXFORMAT_FLOAT4 }
            }
        },

        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },

        .cull_mode = SG_CULLMODE_FRONT,
        .sample_count = 1
    });
}

void gx_rounded_rect_data_draw(
    ecs_world_t *world,
    gx_rounded_rect_t *geometry,
    GxCanvas *canvas)
{
    if (!geometry->instance_count) {
        return;
    }

    sg_bindings bind = {
        .vertex_buffers = {
            [POSITION_I] =       geometry->mesh.verts,
            [COLOR_I] =          geometry->color_buf,
            [SIZE_I] =           geometry->size_buf,
            [CORNER_RADIUS_I] =  geometry->corner_radius_buf,
            [STROKE_COLOR_I] =   geometry->stroke_color_buf,
            [STROKE_WIDTH_I] =   geometry->stroke_width_buf,
            [TRANSFORM_I] =      geometry->transform_buf
        },
        .index_buffer = geometry->mesh.idx
    };

    sg_apply_bindings(&bind);
    sg_draw(0, geometry->mesh.index_count, geometry->instance_count);

    // for (int32_t i = 0; i < geometry->instance_count; i ++) {
        
    // }
}

static
void gx_rounded_rect_update(
    gx_rounded_rect_t *geometry)
{
    int32_t new_size = ecs_vec_count(&geometry->color);
    int32_t old_size = geometry->instance_count;

    if (new_size != old_size) {
        if (old_size) {
            sg_destroy_buffer(geometry->size_buf);
            sg_destroy_buffer(geometry->color_buf);
            sg_destroy_buffer(geometry->corner_radius_buf);
            sg_destroy_buffer(geometry->stroke_color_buf);
            sg_destroy_buffer(geometry->stroke_width_buf);
            sg_destroy_buffer(geometry->transform_buf);
        }

        geometry->size_buf = sg_make_buffer(&(sg_buffer_desc){
            .size = new_size * sizeof(vec2), .usage = SG_USAGE_STREAM });
        geometry->color_buf = sg_make_buffer(&(sg_buffer_desc){
            .size = new_size * sizeof(vec3), .usage = SG_USAGE_STREAM });
        geometry->corner_radius_buf = sg_make_buffer(&(sg_buffer_desc){
            .size = new_size * sizeof(float), .usage = SG_USAGE_STREAM });
        geometry->stroke_color_buf = sg_make_buffer(&(sg_buffer_desc){
            .size = new_size * sizeof(vec3), .usage = SG_USAGE_STREAM });
        geometry->stroke_width_buf = sg_make_buffer(&(sg_buffer_desc){
            .size = new_size * sizeof(float), .usage = SG_USAGE_STREAM });
        geometry->transform_buf = sg_make_buffer(&(sg_buffer_desc){
            .size = new_size * sizeof(mat4), .usage = SG_USAGE_STREAM });

        geometry->instance_count = new_size;
    }

    if (new_size > 0) {
        sg_update_buffer(geometry->size_buf, &(sg_range){
            ecs_vec_first_t(&geometry->size, vec2),
                new_size * sizeof(vec2) } );
        sg_update_buffer(geometry->color_buf, &(sg_range){
            ecs_vec_first_t(&geometry->color, vec3),
                new_size * sizeof(vec3) } );
        sg_update_buffer(geometry->corner_radius_buf, &(sg_range){
            ecs_vec_first_t(&geometry->corner_radius, float),
                new_size * sizeof(float) } );
        sg_update_buffer(geometry->stroke_color_buf, &(sg_range){
            ecs_vec_first_t(&geometry->stroke_color, vec3),
                new_size * sizeof(vec3) } );
        sg_update_buffer(geometry->stroke_width_buf, &(sg_range){
            ecs_vec_first_t(&geometry->stroke_width, float),
                new_size * sizeof(float) } );
        sg_update_buffer(geometry->transform_buf, &(sg_range){
            ecs_vec_first_t(&geometry->transform, mat4),
                new_size * sizeof(mat4) } );
    }
}

static
void gx_rounded_rect_collect(
    ecs_world_t *world,
    gx_rounded_rect_t *geometry,
    GxCanvas *canvas)
{
    ecs_vec_reset_t(NULL, &geometry->size, vec2);
    ecs_vec_reset_t(NULL, &geometry->color, vec3);
    ecs_vec_reset_t(NULL, &geometry->corner_radius, float);
    ecs_vec_reset_t(NULL, &geometry->stroke_color, vec3);
    ecs_vec_reset_t(NULL, &geometry->stroke_width, float);
    ecs_vec_reset_t(NULL, &geometry->transform, mat4);

    ecs_iter_t it = ecs_query_iter(world, geometry->query);
    while (ecs_query_next(&it)) {
        GxTransform2Computed *t = ecs_field(&it, GxTransform2Computed, 1);
        GxTransform2Computed *t_parent = ecs_field(&it, GxTransform2Computed, 2);
        GxStyleComputed *style = ecs_field(&it, GxStyleComputed, 4);
        bool aligned = ecs_field_is_set(&it, 6);

        gx_transform2(t, t_parent, it.count, canvas);

        if (aligned) {
            gx_align2(t, t_parent, it.count, canvas);
        }

        gx_scale2(t, it.count);

        vec2 *i_size = ecs_vec_grow_t(NULL, &geometry->size, vec2, it.count);
        vec3 *i_color = ecs_vec_grow_t(NULL, &geometry->color, vec3, it.count);
        float *i_corner_radius = ecs_vec_grow_t(NULL, &geometry->corner_radius, float, it.count);
        vec3 *i_stroke_color = ecs_vec_grow_t(NULL, &geometry->stroke_color, vec3, it.count);
        float *i_stroke_width = ecs_vec_grow_t(NULL, &geometry->stroke_width, float, it.count);
        mat4 *i_transform = ecs_vec_grow_t(NULL, &geometry->transform, mat4, it.count);

        for (int32_t i = 0; i < it.count; i ++) {
            ecs_os_memcpy_t(i_size[i], t[i].scale, vec2);
            ecs_os_memcpy_t(i_color[i], style[i].color, vec3);
            i_corner_radius[i] = style[i].corner_radius;
            ecs_os_memcpy_t(i_stroke_color[i], style[i].stroke_color, vec3);
            i_stroke_width[i] = style[i].stroke_width;
            ecs_os_memcpy_t(i_transform[i], t[i].mat, mat4);
        }
    }
}

void gx_rounded_rect_init(
    ecs_world_t *world,
    gx_rounded_rect_t *ctx)
{
    ctx->query = ecs_query(world, {
        .filter.terms = {
            { .id = ecs_id(GxTransform2Computed), .inout = EcsIn },
            { .id = ecs_id(GxTransform2Computed), .inout = EcsIn, .oper = EcsOptional, .src.flags = EcsParent|EcsCascade },
            { .id = ecs_id(EcsCornerRadius), .inout = EcsIn },
            { .id = ecs_id(GxStyleComputed), .inout = EcsIn, .oper = EcsOptional },
            { .id = ecs_id(EcsRgb),   .inout = EcsInOutNone, .oper = EcsOptional },
            { .id = ecs_id(EcsAlign), .inout = EcsInOutNone, .oper = EcsOptional },
        }
    });

    ctx->pip = gx_rounded_rect_init_pipeline();
    ctx->mesh = gx_make_rect();
    ctx->instance_count = 0;
}

void gx_rounded_rect_populate(
    ecs_world_t *world,
    gx_rounded_rect_t *ctx,
    GxCanvas *canvas)
{
    gx_rounded_rect_collect(world, ctx, canvas);
    gx_rounded_rect_update(ctx);
}

void gx_rounded_rect_draw(
    ecs_world_t *world,
    gx_rounded_rect_t *geometry,
    GxCanvas *canvas)
{
    gx_rounded_rect_vs_uniforms_t vs_u;
    gx_geometry_matvp_make(vs_u.mat_vp, &canvas->viewport);
    gx_rounded_rect_fs_uniforms_t fs_u;
    fs_u.aspect = (float)canvas->actual_width / (float)canvas->actual_height;

    sg_apply_pipeline(geometry->pip);

    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, 
        &(sg_range){&vs_u, sizeof(gx_rounded_rect_vs_uniforms_t)});
    sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, 
        &(sg_range){&fs_u, sizeof(gx_rounded_rect_fs_uniforms_t)});

    gx_rounded_rect_data_draw(world, geometry, canvas);
}

#undef POSITION_I
#undef COLOR_I
#undef STROKE_COLOR_I
#undef TRANSFORM_I
#undef LAYOUT_I_STR
#undef LAYOUT
