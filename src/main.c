#include <flecs_gx.h>
#include "modules/app/app.h"
#include "modules/canvas/canvas.h"
#include "modules/event/event.h"

void FlecsGxImport(
    ecs_world_t *world)
{
    ECS_MODULE(world, FlecsGx);

    ECS_IMPORT(world, FlecsComponentsTransform);
    ECS_IMPORT(world, FlecsComponentsGraphics);
    ECS_IMPORT(world, FlecsComponentsGeometry);
    ECS_IMPORT(world, FlecsComponentsInput);
    ECS_IMPORT(world, FlecsComponentsGui);

    ECS_IMPORT(world, FlecsGxApp);
    ECS_IMPORT(world, FlecsGxCanvas);
    ECS_IMPORT(world, FlecsGxEvent);

    ecs_set_name_prefix(world, "Ecs");
}
