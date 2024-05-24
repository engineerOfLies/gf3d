#include "simple_logger.h"
#include "gf3d_effects.h"
#include "gf3d_effects.h"

typedef struct
{
    Uint32      effect_max;
    GF3DEffect *effect_list;
    Uint32      now;
}GF3DEventManager;

static GF3DEventManager gf3d_effect_manager = {0};

void gf3d_effect_manager_close()
{
    int i;
    for (i = 0; i < gf3d_effect_manager.effect_max;i++)
    {
        if (!gf3d_effect_manager.effect_list[i]._inuse)continue;
        gf3d_effect_free(&gf3d_effect_manager.effect_list[i]);
    }
    if (gf3d_effect_manager.effect_list)free(gf3d_effect_manager.effect_list);
    memset(&gf3d_effect_manager,0,sizeof(GF3DEventManager));
}

void gf3d_effect_manager_init(Uint32 maxEffects)
{
    if (!maxEffects)
    {
        slog("cannot initialize effect manager with zero effects");
        return;
    }
    gf3d_effect_manager.effect_list = gfc_allocate_array(sizeof(GF3DEffect),maxEffects);
    if (!gf3d_effect_manager.effect_list)
    {
        slog("effect manager could not allocate memory for effects");
        return;
    }
    gf3d_effect_manager.effect_max = maxEffects;
    atexit(gf3d_effect_manager_close);
}

void gf3d_effect_update(GF3DEffect *effect)
{
    if (!effect)return;
    if ((effect->ttd > 0)&&(effect->ttd < gf3d_effect_manager.now))
    {//if we time out   AND we have timed out
        gfc_callback_call(&effect->callback);
        gf3d_effect_free(effect);
        return;
    }
    gfc_color_add(&effect->color,effect->color,effect->colorVector);
    gfc_color_add(&effect->colorVector,effect->colorVector,effect->colorAcceleration);
    //do fade in/ fade out
    
    //do physcial updates
    switch (effect->eType)
    {
        case GF3D_ET_Particle:
            gfc_vector3d_add(effect->particle.position,effect->particle.position,effect->velocity);
            gfc_vector3d_add(effect->velocity,effect->velocity,effect->acceleration);
            gfc_color_copy(effect->particle.color,effect->color);
            effect->particle.size *= effect->sizeDelta;
            break;
        case GF3D_ET_Line:
            gf3d_model_mat_move(&effect->mat,effect->mat.positionDelta);
            gf3d_model_mat_rotate(&effect->mat,effect->mat.rotationDelta);
            gf3d_model_mat_scale(&effect->mat,effect->mat.scaleDelta);
            effect->radius *= effect->sizeDelta;
            break;
        case GF3D_ET_Model:
            gf3d_model_mat_move(&effect->mat,effect->mat.positionDelta);
            gf3d_model_mat_rotate(&effect->mat,effect->mat.rotationDelta);
            gf3d_model_mat_scale(&effect->mat,effect->mat.scaleDelta);
            gfc_vector3d_add(effect->mat.positionDelta,effect->mat.positionDelta,effect->acceleration);
            gfc_matrix4_from_vectors(
                effect->mat.mat,
                effect->mat.position,
                effect->mat.rotation,
                effect->mat.scale);
            break;
        default:
            break;
    }
}

void gf3d_effect_draw(GF3DEffect *effect)
{
    if (!effect)return;
    switch (effect->eType)
    {
        case GF3D_ET_Particle:
            gf3d_particle_draw(effect->particle);
            break;
        case GF3D_ET_Line:
            gf3d_draw_edge_3d(effect->edge,effect->mat.position,effect->mat.rotation,effect->mat.scale,effect->radius,effect->color);
            break;
        case GF3D_ET_Model:
            gf3d_model_draw(
                effect->mat.model,
                0,
                effect->mat.mat,
                gf3d_mesh_basic_material_from_color(effect->color),
                gf3d_light_basic_ambient_ubo(),
                0);
            break;
        default:
            break;
    }
}

void gf3d_effect_manager_draw_all()
{
    int i;
    gf3d_effect_manager.now++;
    for (i = 0; i < gf3d_effect_manager.effect_max;i++)
    {
        if (!gf3d_effect_manager.effect_list[i]._inuse)continue;
        gf3d_effect_update(&gf3d_effect_manager.effect_list[i]);
        gf3d_effect_draw(&gf3d_effect_manager.effect_list[i]);
    }    
}

GF3DEffect *gf3d_effect_new()
{
    int i;
    for (i = 0; i < gf3d_effect_manager.effect_max;i++)
    {
        if (gf3d_effect_manager.effect_list[i]._inuse)continue;
        gf3d_effect_manager.effect_list[i]._inuse = 1;
        gf3d_effect_manager.effect_list[i].color = gfc_color8(255,255,255,255);
        gf3d_model_mat_reset(&gf3d_effect_manager.effect_list[i].mat);
        gf3d_effect_manager.effect_list[i].sizeDelta = 1;
        return &gf3d_effect_manager.effect_list[i];
    }
    return NULL;
}

void gf3d_effect_free(GF3DEffect *effect)
{
    if (!effect)return;
    if (effect->mat.model)gf3d_model_free(effect->mat.model);
    memset(effect,0,sizeof(GF3DEffect));
}

GF3DEffect *gf3d_effect_new_line(
    GFC_Edge3D edge,
    GFC_Vector3D velocity,
    GFC_Vector3D acceleration,
    float size,
    float sizeDelta,
    GFC_Color color,
    GFC_Color colorVector,
    GFC_Color colorAcceleration,
    Sint32 ttl)
{
    GF3DEffect *effect;
    if (size <= 0)return NULL;
    effect = gf3d_effect_new();
    if (!effect)return NULL;
    effect->eType = GF3D_ET_Line;
    if (ttl < 0)effect->ttd = -1;
    else effect->ttd = gf3d_effect_manager.now + ttl;
    effect->radius = size;
    effect->edge = edge;
    effect->velocity = velocity;
    effect->acceleration = acceleration;
    effect->particle.size = size;
    effect->sizeDelta = sizeDelta;
    gfc_color_copy(effect->color,color);
    gfc_color_copy(effect->colorVector,colorVector);
    gfc_color_copy(effect->colorAcceleration,colorAcceleration);
    return effect;
}

GF3DEffect *gf3d_effect_new_particle(
    GFC_Vector3D position,
    GFC_Vector3D velocity,
    GFC_Vector3D acceleration,
    float size,
    float sizeDelta,
    GFC_Color color,
    GFC_Color colorVector,
    GFC_Color colorAcceleration,
    Sint32 ttl)
{
    GF3DEffect *effect;
    effect = gf3d_effect_new();
    if (!effect)return NULL;
    effect->eType = GF3D_ET_Particle;
    if (ttl < 0)effect->ttd = -1;
    else effect->ttd = gf3d_effect_manager.now + ttl;
    effect->particle.position = position;
    effect->velocity = velocity;
    effect->acceleration = acceleration;
    effect->particle.size = size;
    effect->sizeDelta = sizeDelta;
    gfc_color_copy(effect->particle.color,color);
    gfc_color_copy(effect->color,color);
    gfc_color_copy(effect->colorVector,colorVector);
    gfc_color_copy(effect->colorAcceleration,colorAcceleration);
    return effect;
}

void gf3d_effect_set_callback(GF3DEffect *effect,void (*callback)(void *data),void *data)
{
    if (!effect)return;
    effect->callback.callback = callback;
    effect->callback.data = data;
}


void gf3d_effect_make_particle_explosion(
    GFC_Vector3D position,
    float size,
    float sizeDelta,
    GFC_Color color,
    GFC_Color colorVariation,
    int count,
    float speed,
    Uint32 ttl)
{
    int i;
    GFC_Color pColor,vGFC_Color;
    GFC_Vector3D velocity;
    for (i = 0; i < count; i++)
    {
        velocity.x = gfc_crandom();
        velocity.y = gfc_crandom();
        velocity.z = gfc_crandom();
        gfc_vector3d_set_magnitude(&velocity,speed);
        gfc_color_copy(pColor,color);
        gfc_color_copy(vGFC_Color,colorVariation);
        vGFC_Color.r *= gfc_crandom();
        vGFC_Color.g *= gfc_crandom();
        vGFC_Color.b *= gfc_crandom();
        vGFC_Color.a *= gfc_crandom();
        gfc_color_add(&pColor,pColor,vGFC_Color);
        
        gf3d_effect_new_particle(
            position,
            velocity,
            gfc_vector3d(0,0,0),
            size,
            sizeDelta,
            pColor,
            gfc_color(-0.01,-0.01,-0.01,0),
            gfc_color(0,0,0,0),
            ttl);
    }
}

GF3DEffect *gf3d_effect_new_particle_target(
    GFC_Vector3D position,
    GFC_Vector3D target,
    float size,
    float sizeDelta,
    GFC_Color color,
    GFC_Color colorVector,
    GFC_Color colorAcceleration,
    Sint32 ttl)
{
    GFC_Vector3D velocity;
    GF3DEffect *effect;
    if (!ttl)
    {
        return NULL;
    }
    effect = gf3d_effect_new();
    if (!effect)return NULL;
    effect->ttd = gf3d_effect_manager.now + ttl;
    effect->eType = GF3D_ET_Particle;
    effect->particle.position = position;
    effect->particle.size = size;
    effect->sizeDelta = sizeDelta;
    gfc_color_copy(effect->particle.color,color);
    gfc_color_copy(effect->color,color);
    gfc_color_copy(effect->colorVector,colorVector);
    gfc_color_copy(effect->colorAcceleration,colorAcceleration);
    gfc_vector3d_sub(velocity,target,position);
    
    gfc_vector3d_scale(velocity,velocity,(1.0 / (float)ttl));
    effect->velocity = velocity;
    return effect;
}


GF3DEffect *gf3d_effect_from_config(SJson *config)
{
    GF3DEffect *effect;
    if (!config)return NULL;
    effect = gf3d_effect_new();
    if (!effect)return NULL;
    
    return effect;
}

/*eol@eof*/
