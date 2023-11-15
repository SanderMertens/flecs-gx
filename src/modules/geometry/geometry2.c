#include "geometry2.h"

#define POSITION_I 0
#define COLOR_I 1
#define TRANSFORM_I 2
#define LAYOUT_I_STR(i) #i
#define LAYOUT(loc) "layout(location=" LAYOUT_I_STR(loc) ") "

typedef struct gx_geometry_vs_uniforms_t {
    mat4 mat_vp;
} gx_geometry_vs_uniforms_t;

typedef struct gx_geometry_vs_data_t {
    vec3 *colors;
    mat4 *transforms;
} gx_geometry_vs_data_t;

static
vec3 gx_geometry_rect_verts[] = {
    {-0.5,  0.5, 0.0},
    { 0.5,  0.5, 0.0},
    { 0.5, -0.5, 0.0},
    {-0.5, -0.5, 0.0},
};

static
uint16_t gx_geometry_rect_idx[] = {
    0, 1, 2,
    0, 2, 3
};

static
sg_pipeline gx_geometry_init_pipeline(void) {
    char *vs = gx_shader_from_str(
        SOKOL_SHADER_HEADER
        "uniform mat4 u_mat_vp;\n"
        LAYOUT(POSITION_I)  "in vec3 v_position;\n"
        LAYOUT(COLOR_I)     "in vec3 i_color;\n"
        LAYOUT(TRANSFORM_I) "in mat4 i_mat_m;\n"
        "#include \"etc/flecs-gx/shaders/draw2d_vert.glsl\"\n"
    );

    char *fs = gx_shader_from_str(
        SOKOL_SHADER_HEADER
        "#include \"etc/flecs-gx/shaders/draw2d_frag.glsl\"\n"
    );

    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .vs.uniform_blocks = {
            [0] = {
                .size = sizeof(gx_geometry_vs_uniforms_t),
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
                [COLOR_I]         = { .stride = 12, .step_func = SG_VERTEXSTEP_PER_INSTANCE },
                [TRANSFORM_I]     = { .stride = 64, .step_func = SG_VERTEXSTEP_PER_INSTANCE }
            },

            .attrs = {
                /* Static geometry */
                [POSITION_I]      = { .buffer_index = POSITION_I,  .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT3 },

                /* Color buffer (per instance) */
                [COLOR_I]         = { .buffer_index = COLOR_I,     .offset = 0,  .format = SG_VERTEXFORMAT_FLOAT3 },

                /* Matrix (per instance) */
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

static
void gx_geometry2_matvp_make(mat4 mat_vp, gx_viewport_t *vp) {
    glm_ortho(vp->left, vp->right, vp->top, vp->bottom, -1.0, 1.0, mat_vp);
}

static
void gx_geometry_draw_border(
    GxTransform2Computed *t,
    GxStyleComputed *style,
    vec3 *stroke_color,
    mat4 *stroke_transform) 
{
    float w = t->scale[0];
    float h = t->scale[1];
    float sw = style->stroke_width;
    float sw_h = sw / 2;
    float w_h = w / 2.0;
    float h_h = h / 2.0;

    { // Left
        vec3 pos = {-w_h + sw_h, 0, 0};
        vec3 scale = {sw, h, 1};

        glm_translate_to(t->mat, pos, stroke_transform[0]);
        glm_scale(stroke_transform[0], scale);

        stroke_color[0][0] = style->stroke_color[0];
        stroke_color[0][1] = style->stroke_color[1];
        stroke_color[0][2] = style->stroke_color[2];
    }
    { // Right
        vec3 pos = {w_h - sw_h, 0, 0};
        vec3 scale = {sw, h, 1};

        glm_translate_to(t->mat, pos, stroke_transform[1]);
        glm_scale(stroke_transform[1], scale);

        stroke_color[1][0] = style->stroke_color[0];
        stroke_color[1][1] = style->stroke_color[1];
        stroke_color[1][2] = style->stroke_color[2];
    }
    { // Top
        vec3 pos = {0, h_h - sw_h, 0};
        vec3 scale = {w, sw, 1};

        glm_translate_to(t->mat, pos, stroke_transform[2]);
        glm_scale(stroke_transform[2], scale);

        stroke_color[2][0] = style->stroke_color[0];
        stroke_color[2][1] = style->stroke_color[1];
        stroke_color[2][2] = style->stroke_color[2];
    }
    { // Bottom
        vec3 pos = {0, -h_h + sw_h, 0};
        vec3 scale = {w, sw, 1};

        glm_translate_to(t->mat, pos, stroke_transform[3]);
        glm_scale(stroke_transform[3], scale);

        stroke_color[3][0] = style->stroke_color[0];
        stroke_color[3][1] = style->stroke_color[1];
        stroke_color[3][2] = style->stroke_color[2];
    }
}

static
void gx_geometry2_collect(
    ecs_world_t *world,
    gx_geometry2_data_t *geometry,
    GxCanvas *canvas)
{
    ecs_vec_reset_t(NULL, &geometry->color, vec3);
    ecs_vec_reset_t(NULL, &geometry->transform, mat4);

    vec3 axis = {0.0, 0.0, 1.0};

    ecs_iter_t it = ecs_query_iter(world, geometry->query);
    while (ecs_query_next(&it)) {
        GxTransform2Computed *t = ecs_field(&it, GxTransform2Computed, 1);
        GxTransform2Computed *t_parent = ecs_field(&it, GxTransform2Computed, 2);
        GxStyleComputed *s = ecs_field(&it, GxStyleComputed, 3);
        bool is_solid = ecs_field_is_set(&it, 4) || ecs_field_is_set(&it, 5);
        bool aligned = ecs_field_is_set(&it, 6);
        int32_t with_stroke = 0;

        // Calculate model matrix
        if (!t_parent) {
            vec3 center = {canvas->width / 2, canvas->height / 2, 0};
            for (int32_t i = 0; i < it.count; i ++) {
                glm_translate_make(t[i].mat, t[i].position);
                glm_translate(t[i].mat, center);
            }
        } else {
            for (int32_t i = 0; i < it.count; i ++) {
                glm_translate_to(t_parent[i].mat, t[i].position, t[i].mat);
                glm_rotate(t[i].mat, t[i].rotation, axis);
            }
        }

        // Align items
        if (aligned) {
            int32_t width = 0, height = 0;
            if (!t_parent) {
                width = canvas->width / 2;
                height = canvas->height / 2;
            } else {
                width = t_parent[0].scale[0] / 2 - t_parent[0].padding;
                height = t_parent[0].scale[1] / 2 - t_parent[0].padding;
            }

            for (int32_t i = 0; i < it.count; i ++) {
                EcsAlign align = t[i].align;
                int32_t align_x = 0, align_y = 0;
                int32_t w = t[i].scale[0], h = t[i].scale[1];
                if (align & EcsAlignLeft) {
                    align_x = w / 2 - width;
                } else
                if (align & EcsAlignCenter) {
                    align_x = 0;
                }
                if (align & EcsAlignRight) {
                    align_x = width - w / 2;
                }

                if (align & EcsAlignTop) {
                    align_y = h / 2 - height;
                } else
                if (align & EcsAlignMiddle) {
                    align_y = 0;
                } else
                if (align & EcsAlignBottom) {
                    align_y = height - h / 2;
                }

                if (align_x || align_y) {
                    glm_translate(t[i].mat, (vec3){align_x, align_y, 0});
                }
            }        
        }

        // Draw solid objects
        if (s) {
            vec3 *color = NULL;
            mat4 *transform = NULL;
            int32_t cur = 0;

            for (int32_t i = 0; i < it.count; i ++) {
                with_stroke += s[i].stroke_width > 0;
            }

            color = ecs_vec_grow_t(NULL, &geometry->color, vec3, 
                is_solid * it.count + with_stroke * 4);
            transform = ecs_vec_grow_t(NULL, &geometry->transform, mat4, 
                is_solid * it.count + with_stroke * 4);

            // Populate matrix & color
            if (is_solid) {
                for (int32_t i = 0; i < it.count; i ++) {
                    glm_mat4_copy(t[i].mat, transform[cur]);
                    glm_scale(transform[cur], t[i].scale);

                    color[cur][0] = s[i].color[0];
                    color[cur][1] = s[i].color[1];
                    color[cur][2] = s[i].color[2];

                    cur ++;

                    if (s[i].stroke_width != 0) {
                        gx_geometry_draw_border(&t[i], &s[i],
                            &color[cur], &transform[cur]);
                        cur += 4;
                    }
                }
            } else {
                for (int32_t i = 0; i < it.count; i ++) {
                    if (s[i].stroke_width != 0) {
                        gx_geometry_draw_border(&t[i], &s[i],
                            &color[cur], &transform[cur]);
                        cur += 4;
                    }
                }
            }
        }
    }
}

static
void gx_geometry2_update(
    gx_geometry2_data_t *geometry)
{
    int32_t new_size = ecs_vec_count(&geometry->color);
    int32_t old_size = geometry->instance_count;
    if (new_size != old_size) {
        if (old_size) {
            sg_destroy_buffer(geometry->color_buf);
            sg_destroy_buffer(geometry->transform_buf);
        }

        geometry->color_buf = sg_make_buffer(&(sg_buffer_desc){
            .size = new_size * sizeof(vec3), .usage = SG_USAGE_STREAM });
        geometry->transform_buf = sg_make_buffer(&(sg_buffer_desc){
            .size = new_size * sizeof(mat4), .usage = SG_USAGE_STREAM });
        geometry->instance_count = new_size;
    }

    if (new_size > 0) {
        sg_update_buffer(geometry->color_buf, &(sg_range) {
            ecs_vec_first_t(&geometry->color, vec3), 
                new_size * sizeof(vec3) } );
        sg_update_buffer(geometry->transform_buf, &(sg_range) {
            ecs_vec_first_t(&geometry->transform, mat4), 
                new_size * sizeof(mat4) } );
    }
}

static
void gx_geometry2_init_rect(
    ecs_world_t *world,
    gx_geometry2_data_t *geometry)
{
    // Query that matches rectangle entities
    geometry->query = ecs_query(world, {
        .filter.terms = {
            { .id = ecs_id(GxTransform2Computed), .inout = EcsIn },
            { .id = ecs_id(GxTransform2Computed), .inout = EcsIn, .oper = EcsOptional, .src.flags = EcsParent|EcsCascade },
            { .id = ecs_id(GxStyleComputed), .inout = EcsIn, .oper = EcsOptional },
            { .id = ecs_id(EcsRgb),   .inout = EcsInOutNone, .oper = EcsOptional },
            { .id = ecs_id(EcsLine2), .inout = EcsInOutNone, .oper = EcsOptional },
            { .id = ecs_id(EcsAlign), .inout = EcsInOutNone, .oper = EcsOptional }
        }
    });

    // Create vertex & index buffers for rectangle geometry
    geometry->verts = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(gx_geometry_rect_verts),
        .label = "rect_verts"
    });

    geometry->idx = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(gx_geometry_rect_idx),
        .label = "rect_idx",
        .type = SG_BUFFERTYPE_INDEXBUFFER
    });

    geometry->index_count = 6;
}

static
void gx_geometry2_data_draw(
    gx_geometry2_data_t *geometry)
{
    if (!geometry->instance_count) {
        return;
    }

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

static
void GxGeometryPopulateDof(ecs_iter_t *it) {
    EcsPosition2 *p = ecs_field(it, EcsPosition2, 1);
    EcsRotation2 *r = ecs_field(it, EcsRotation2, 2);
    GxTransform2Computed *t = ecs_field(it, GxTransform2Computed, 3);

    if (p) {
        for (int32_t i = 0; i < it->count; i ++) {
            t[i].position[0] = p[i].x;
            t[i].position[1] = p[i].y;
            t[i].position[2] = 0;
        }
    } else {
        for (int32_t i = 0; i < it->count; i ++) {
            t[i].position[0] = 0;
            t[i].position[1] = 0;
            t[i].position[2] = 0;
        }
    }

    if (r) {
        for (int32_t i = 0; i < it->count; i ++) {
            t[i].rotation = r[i].angle;
        }
    } else {
        for (int32_t i = 0; i < it->count; i ++) {
            t[i].rotation = 0;
        }
    }
}

static
void GxGeometryPopulateRect(ecs_iter_t *it) {
    EcsRectangle *g = ecs_field(it, EcsRectangle, 1);
    EcsPosition2 *p = ecs_field(it, EcsPosition2, 2);
    EcsRotation2 *r = ecs_field(it, EcsRotation2, 3);
    EcsRgb *c = ecs_field(it, EcsRgb, 4);
    EcsStroke *st = ecs_field(it, EcsStroke, 5);
    EcsPadding *padding = ecs_field(it, EcsPadding, 6);
    GxTransform2Computed *t = ecs_field(it, GxTransform2Computed, 7);
    GxStyleComputed *cc = ecs_field(it, GxStyleComputed, 8);

    if (p) {
        for (int32_t i = 0; i < it->count; i ++) {
            t[i].position[0] = p[i].x;
            t[i].position[1] = p[i].y;
            t[i].position[2] = 0;
        }
    } else {
        for (int32_t i = 0; i < it->count; i ++) {
            t[i].position[0] = 0;
            t[i].position[1] = 0;
            t[i].position[2] = 0;
        }
    }

    if (r) {
        for (int32_t i = 0; i < it->count; i ++) {
            t[i].rotation = r[i].angle;
        }
    } else {
        for (int32_t i = 0; i < it->count; i ++) {
            t[i].rotation = 0;
        }
    }

    for (int32_t i = 0; i < it->count; i ++) {
        t[i].scale[0] = g[i].width;
        t[i].scale[1] = g[i].height;
        t[i].scale[2] = 1;
    }

    if (c) {
        for (int32_t i = 0; i < it->count; i ++) {
            cc[i].color[0] = c[i].r;
            cc[i].color[1] = c[i].g;
            cc[i].color[2] = c[i].b;
        }
    } else {
        for (int32_t i = 0; i < it->count; i ++) {
            cc[i].color[0] = 0;
            cc[i].color[1] = 0;
            cc[i].color[2] = 0;
        }
    }

    if (st) {
        for (int32_t i = 0; i < it->count; i ++) {
            cc[i].stroke_color[0] = st[i].color.r;
            cc[i].stroke_color[1] = st[i].color.g;
            cc[i].stroke_color[2] = st[i].color.b;
            cc[i].stroke_width = st[i].width;
        }
    } else {
        for (int32_t i = 0; i < it->count; i ++) {
            cc[i].stroke_color[0] = 0;
            cc[i].stroke_color[1] = 0;
            cc[i].stroke_color[2] = 0;
            cc[i].stroke_width = 0;
        }
    }

    if (padding) {
        for (int32_t i = 0; i < it->count; i ++) {
            t[i].padding = padding[i].value;
        }
    }
}

static
void GxGeometryPopulateLine2(ecs_iter_t *it) {
    EcsLine2 *g = ecs_field(it, EcsLine2, 1);
    EcsStroke *s = ecs_field(it, EcsStroke, 2);
    GxTransform2Computed *t = ecs_field(it, GxTransform2Computed, 3);
    GxStyleComputed *cc = ecs_field(it, GxStyleComputed, 4);

    for (int32_t i = 0; i < it->count; i ++) {
        vec3 start = {g[i].start[0], g[i].start[1], 0};
        vec3 stop = {g[i].stop[0], g[i].stop[1], 0};
        vec3 dir;
        glm_vec3_sub(stop, start, dir);
        float len = glm_vec3_norm(dir);
        glm_vec3_normalize(dir);

        t[i].position[0] = (g[i].start[0] + g[i].stop[0]) / 2;
        t[i].position[1] = (g[i].start[1] + g[i].stop[1]) / 2;
        t[i].position[2] = 0;

        t[i].rotation = atan2(dir[1], dir[0]);

        t[i].scale[0] = len;
        t[i].scale[1] = s ? s[i].width : 1;
        t[i].scale[2] = 1;
    }

    if (s) {
        for (int32_t i = 0; i < it->count; i ++) {
            cc[i].color[0] = s[i].color.r;
            cc[i].color[1] = s[i].color.g;
            cc[i].color[2] = s[i].color.b;
        }
    } else {
        for (int32_t i = 0; i < it->count; i ++) {
            cc[i].color[0] = 0;
            cc[i].color[1] = 0;
            cc[i].color[2] = 0;
        }
    }
}

static
void GxGeometryPopulateAlign(ecs_iter_t *it) {
    EcsAlign *a = ecs_field(it, EcsAlign, 1);
    GxTransform2Computed *t = ecs_field(it, GxTransform2Computed, 2);

    if (a) {
        for (int32_t i = 0; i < it->count; i ++) {
            t[i].align = a[i];
        }
    } else {
        for (int32_t i = 0; i < it->count; i ++) {
            t[i].align = 0;
        }
    }
}

void gx_geometry2_init(
    ecs_world_t *world,
    gx_geometry2_t *ctx)
{
    gx_geometry2_init_rect(world, &ctx->rect_data);
    ctx->pip = gx_geometry_init_pipeline();
}

void gx_geometry2_populate(
    ecs_world_t *world,
    gx_geometry2_t*ctx,
    GxCanvas *canvas)
{
    gx_geometry2_collect(world, &ctx->rect_data, canvas);
    gx_geometry2_update(&ctx->rect_data);
}

void gx_geometry2_draw(
    gx_geometry2_t *geometry,
    GxCanvas *canvas)
{
    gx_geometry_vs_uniforms_t vs_u;
    gx_geometry2_matvp_make(vs_u.mat_vp, &canvas->viewport);

    sg_apply_pipeline(geometry->pip);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, 
        &(sg_range){&vs_u, sizeof(gx_geometry_vs_uniforms_t)});

    gx_geometry2_data_draw(&geometry->rect_data);
}

void gx_geometry2_import(
    ecs_world_t *world)
{
    ECS_SYSTEM(world, GxGeometryPopulateDof, EcsOnStore, 
        [in]  ?flecs.components.transform.Position2, 
        [in]  ?flecs.components.transform.Rotation2,
        [out] GxTransform2Computed,        
              !GxStyleComputed);

    ECS_SYSTEM(world, GxGeometryPopulateRect, EcsOnStore, 
        [in]  flecs.components.geometry.Rectangle, 
        [in]  ?flecs.components.transform.Position2, 
        [in]  ?flecs.components.transform.Rotation2,
        [in]  ?flecs.components.graphics.Rgb,
        [in]  ?flecs.components.geometry.Stroke,
        [in]  ?flecs.components.gui.Padding,
        [out] GxTransform2Computed,
        [out] GxStyleComputed);

    ECS_SYSTEM(world, GxGeometryPopulateLine2, EcsOnStore, 
        [in]  flecs.components.geometry.Line2, 
        [in]  ?flecs.components.geometry.Stroke,
        [out] GxTransform2Computed,
        [out] GxStyleComputed);

    ECS_SYSTEM(world, GxGeometryPopulateAlign, EcsOnStore, 
        [in]  ?flecs.components.gui.Align,
        [out] GxTransform2Computed);
}
