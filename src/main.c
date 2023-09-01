#include <flecs_gx.h>
#include "modules/app/app.h"
#include "modules/canvas/canvas.h"

void FlecsGxImport(
    ecs_world_t *world)
{
    ECS_MODULE(world, FlecsGx);

    ECS_IMPORT(world, FlecsComponentsTransform);
    ECS_IMPORT(world, FlecsComponentsGraphics);
    ECS_IMPORT(world, FlecsComponentsGui);

    ECS_IMPORT(world, FlecsGxApp);
    ECS_IMPORT(world, FlecsGxCanvas);

    ecs_set_name_prefix(world, "Ecs");
}
