#include <stdlib.h>
#include <string.h>

#include "simple_logger.h"

#include "gf3d_entity.h"

typedef struct
{
    Entity *entity_list;
    Uint32  entity_count;
    Uint8   initialized;
}EntityManager;

static EntityManager entity_manager = {0};

void gf3d_entity_system_close()
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        gf3d_entity_free(&entity_manager.entity_list[i]);        
    }
    free(entity_manager.entity_list);
    memset(&entity_manager,0,sizeof(EntityManager));
}

void gf3d_entity_system_init(Uint32 maxEntities)
{
    entity_manager.entity_list = gfc_allocate_array(sizeof(Entity),maxEntities);
    if (entity_manager.entity_list == NULL)
    {
        slog("failed to allocate entity list, cannot allocate ZERO entities");
        return;
    }
    entity_manager.entity_count = maxEntities;
    entity_manager.initialized = 1;
    atexit(gf3d_entity_system_close);
}

Entity *gf3d_entity_new()
{
    int i;
    if (!entity_manager.initialized)return NULL;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse)// not used yet, so we can!
        {
            entity_manager.entity_list[i]._inuse = 1;
            
            gf3d_model_mat_reset(&entity_manager.entity_list[i].mat);
            
            entity_manager.entity_list[i].color = gfc_color(1,1,1,1);
            entity_manager.entity_list[i].selectedColor = gfc_color(1,1,1,1);
            
            return &entity_manager.entity_list[i];
        }
    }
    slog("gf3d_entity_new: no free space in the entity list");
    return NULL;
}

void gf3d_entity_free(Entity *self)
{
    if (!entity_manager.initialized)return;
    if (!self)return;
    //MUST DESTROY
    if (self->free)
    {
        self->free(self);
    }
    gf3d_model_free(self->mat.model);
    memset(self,0,sizeof(Entity));
}


void gf3d_entity_draw(Entity *self)
{
    if (!entity_manager.initialized)return;
    if (!self)return;
    if (self->hidden)return;
    if (self->draw)self->draw(self);
    if (!self->mat.model)return;
    gf3d_model_draw(self->mat.model,0,self->mat.mat,gfc_color_to_vector4f(self->color),gfc_color_to_vector4(self->detailColor),vector4d(1,1,1,1));
    if (self->selected)
    {
        gf3d_model_draw_highlight(
            self->mat.model,
            0,
            self->mat.mat,
            gfc_color_to_vector4f(self->selectedColor));
    }
}

void gf3d_entity_draw_all()
{
    int i;
    if (!entity_manager.initialized)return;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse)// not used yet
        {
            continue;// skip this iteration of the loop
        }
        gf3d_entity_draw(&entity_manager.entity_list[i]);
    }
}

void gf3d_entity_think(Entity *self)
{
    if (!entity_manager.initialized)return;
    if (!self)return;
    if (self->think)self->think(self);
}

void gf3d_entity_think_all()
{
    int i;
    if (!entity_manager.initialized)return;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse)// not used yet
        {
            continue;// skip this iteration of the loop
        }
        gf3d_entity_think(&entity_manager.entity_list[i]);
    }
}

Entity *gf3d_entity_get_by_name(const char *name)
{
    int i;
    if (!entity_manager.initialized)return NULL;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse)// not used yet
        {
            continue;// skip this iteration of the loop
        }
        if (gfc_strlcmp(entity_manager.entity_list[i].name,name)==0)return &entity_manager.entity_list[i];
    }
    return NULL;
}

void gf3d_entity_update(Entity *self)
{
    if (!entity_manager.initialized)return;
    if (!self)return;
    // HANDLE ALL COMMON UPDATE STUFF
    
    gf3d_model_mat_move(&self->mat,self->velocity);
    vector3d_add(self->velocity,self->acceleration,self->velocity);
    
    gf3d_model_mat_set_matrix(&self->mat);
    
    
    if (self->update)self->update(self);
}

void gf3d_entity_update_all()
{
    int i;
    if (!entity_manager.initialized)return;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse)// not used yet
        {
            continue;// skip this iteration of the loop
        }
        gf3d_entity_update(&entity_manager.entity_list[i]);
    }
}

/*eol@eof*/
