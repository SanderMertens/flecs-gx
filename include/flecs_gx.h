#ifndef FLECS_GX_H
#define FLECS_GX_H

/* This generated file contains includes for project dependencies */
#include "flecs-gx/bake_config.h"

#ifdef __cplusplus
extern "C" {
#endif

FLECS_GX_API
void FlecsGxImport(
    ecs_world_t *world);

#ifdef __cplusplus
}

namespace flecs {

class gx {
public:
    gx(flecs::world& ecs) {
        // Bind dependencies with modules
        ecs.import<flecs::components::transform>();
        ecs.import<flecs::components::geometry>();
        ecs.import<flecs::components::graphics>();
        ecs.import<flecs::components::input>();
        ecs.import<flecs::components::gui>();

        // Load module contents
        FlecsGxImport(ecs);

        // Bind C++ types with module contents
        ecs.module<flecs::gx>();
    }
};

}

#endif

#endif
