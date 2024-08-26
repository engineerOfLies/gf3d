#include "simple_logger.h"

#include "gfc_config.h"
#include "gfc_config_def.h"

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
    gfc_action_next_frame(effect->action,&effect->frame);
    //do physcial updates
    switch (effect->eType)
    {
        case GF3D_ET_Particle:
            gfc_vector3d_add(effect->particle.position,effect->particle.position,effect->velocity);
            gfc_vector3d_add(effect->velocity,effect->velocity,effect->acceleration);
            gfc_color_copy(effect->particle.color,effect->color);
            effect->particle.size += effect->sizeVector;
            effect->sizeVector += effect->sizeAcceleration;
            break;
        case GF3D_ET_Line:
            gf3d_model_mat_move(&effect->mat,effect->mat.positionDelta);
            gf3d_model_mat_rotate(&effect->mat,effect->mat.rotationDelta);
            gf3d_model_mat_scale(&effect->mat,effect->mat.scaleDelta);
            effect->size += effect->sizeVector;
            effect->sizeVector += effect->sizeAcceleration;
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
            if (effect->actor)
            {
                gf3d_particle_draw_sprite(effect->particle,effect->actor->sprite,effect->frame);
            }
            else if (effect->image)
            {
                gf3d_particle_draw_sprite(effect->particle,effect->image,0);
            }
            else gf3d_particle_draw(effect->particle);
            break;
        case GF3D_ET_Line:
            gf3d_draw_edge_3d(effect->edge,effect->mat.position,effect->mat.rotation,effect->mat.scale,effect->size,effect->color);
            break;
        case GF3D_ET_Model:
            gf3d_model_draw(effect->mat.model,effect->mat.mat,effect->color,NULL,effect->frame);
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
        memset(&gf3d_effect_manager.effect_list[i],0,sizeof(GF3DEffect));
        gf3d_effect_manager.effect_list[i]._inuse = 1;
        gf3d_effect_manager.effect_list[i].color = gfc_color8(255,255,255,255);
        gf3d_model_mat_reset(&gf3d_effect_manager.effect_list[i].mat);
        return &gf3d_effect_manager.effect_list[i];
    }
    return NULL;
}

void gf3d_effect_free(GF3DEffect *effect)
{
    if (!effect)return;
    if (effect->actor)gf2d_actor_free(effect->actor);
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
    effect->size = size;
    effect->edge = edge;
    effect->velocity = velocity;
    effect->acceleration = acceleration;
    effect->particle.size = size;
    effect->sizeVector = sizeDelta;
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
    effect->sizeVector = sizeDelta;
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
    effect->sizeVector = sizeDelta;
    gfc_color_copy(effect->particle.color,color);
    gfc_color_copy(effect->color,color);
    gfc_color_copy(effect->colorVector,colorVector);
    gfc_color_copy(effect->colorAcceleration,colorAcceleration);
    gfc_vector3d_sub(velocity,target,position);
    
    gfc_vector3d_scale(velocity,velocity,(1.0 / (float)ttl));
    effect->velocity = velocity;
    return effect;
}


GF3DEffect *gf3d_effect_from_config(
    SJson *config,
    GFC_Vector3D position,
    GFC_Vector3D position2,
    GFC_Vector3D velocity,
    GFC_Vector3D acceleration,
    GFC_Callback *callback)
{
    const char *effectType;
    const char *filename;
    float variance = 0;
    GF3DEffect *effect;
    if (!config)return NULL;
    effect = gf3d_effect_new();
    if (!effect)return NULL;
    
    effect->velocity = velocity;
    effect->acceleration = acceleration;
    
    if (callback)
    {
        effect->callback.callback = callback->callback;
        effect->callback.data = callback->data;
    }
    
    sj_object_get_int(config,"ttd",(int*)&effect->ttd);
    if (sj_object_get_float(config,"ttdVariance",&variance))effect->ttd += (int)(gfc_crandom() * variance);
    effect->ttd += gf3d_effect_manager.now;
    
    sj_object_get_float(config,"size",&effect->size);
    sj_object_get_float(config,"sizeVector",&effect->sizeVector);
    sj_object_get_float(config,"sizeAcceleration",&effect->sizeAcceleration);
    if (sj_object_get_float(config,"sizeVariance",&variance))effect->size += (gfc_crandom() * variance);

    sj_object_get_uint8(config,"fadein",&effect->fadein);
    sj_object_get_uint8(config,"fadeout",&effect->fadeout);

    sj_object_get_color_value(config,"color",&effect->color);
    sj_object_get_color_value(config,"colorVector",&effect->colorVector);
    sj_object_get_color_value(config,"colorAcceleration",&effect->colorAcceleration);
    
    effectType = sj_object_get_string(config,"type");
    if (effectType)
    {
        if (gfc_strlcmp(effectType,"particle") == 0)
        {
            effect->eType = GF3D_ET_Particle;
            effect->particle.size = effect->size;
            sj_object_get_color_value(config,"color",&effect->particle.color);
            sj_object_get_color_value(config,"color2",&effect->particle.color2);
            effect->particle.position = position;
            filename = sj_object_get_string(config,"actor");
            if (filename)
            {
                effect->actor = gf2d_actor_load(filename);
                filename = sj_object_get_string(config,"action");
                effect->action = gf2d_actor_get_action_by_name(effect->actor,filename);
            }
            filename = sj_object_get_string(config,"image");
            if (filename)
            {
                effect->image = gf2d_sprite_load_image(filename);
            }

        }
        else if (gfc_strlcmp(effectType,"line") == 0)
        {
            effect->eType = GF3D_ET_Line;
            effect->edge.a = position;
            effect->edge.b = position2;
        }
        else if (gfc_strlcmp(effectType,"model") == 0)
        {
            effect->eType = GF3D_ET_Line;
            filename = sj_object_get_string(config,"model");
            effect->mat.model = gf3d_model_load(filename);
            effect->mat.position = position;
            if (effect->mat.model)
            {
                filename = sj_object_get_string(config,"action");
                if (filename)
                {
                    effect->action = gfc_action_list_get_action(effect->mat.model->action_list,filename);
                }
            }
        }
    }
    return effect;
}

void gf3d_effect_square_emitter(GFC_Vector3D centerPosition, GFC_Vector3D direction,float area,Uint8 count,SJson *effect)
{
    int i;
    float gravity = 0;
    GFC_Vector3D angles,right,up;
    GFC_Vector3D position,velocity,rightV,upV;
    float speed = 0,speedVariance = 0;
    if (!effect)return;
    sj_object_get_float(effect,"gravity",&gravity);
    sj_object_get_float(effect,"speed",&speed);
    sj_object_get_float(effect,"speedVariance",&speedVariance);
    
    gfc_vector3d_normalize(&direction);
    gfc_vector3d_angles (direction, &angles);
    gfc_vector3d_angle_vectors(angles, NULL, &right, &up);
    gfc_vector3d_scale(direction,direction,speed);

    gfc_vector3d_scale(right,right,area);
    gfc_vector3d_scale(up,up,area);
    for (i = 0; i < count; i++)
    {
        gfc_vector3d_copy(position,centerPosition);
        gfc_vector3d_randomize(&rightV,right);
        gfc_vector3d_randomize(&upV,up);
        gfc_vector3d_add(position,position,rightV);
        gfc_vector3d_add(position,position,upV);
        
        velocity.x = direction.x + gfc_crandom() * speedVariance;
        velocity.y = direction.y + gfc_crandom() * speedVariance;
        velocity.z = direction.z + gfc_crandom() * speedVariance;
        gf3d_effect_from_config(
            effect,
            position,
            gfc_vector3d(0,0,0),
            velocity,
            gfc_vector3d(0,0,gravity),
            NULL);
    }
}

void gf3d_effect_emit(GFC_Vector3D position, GFC_Vector3D direction,Uint8 count,const char *effectName)
{
    float area = 0;
    const char *emitter;
    SJson *config;
    if (!effectName)return;
    config = gfc_config_def_get_by_name("effects",effectName);
    if (!config)return;
    emitter = sj_object_get_string(config,"emitter");
    if (!emitter)return;
    if (gfc_strlcmp(emitter,"square") == 0)
    {
        sj_object_get_float(config,"radius",&area);
        gf3d_effect_square_emitter(position, direction,area,count,config);
    }
    else if (gfc_strlcmp(emitter,"point") == 0)
    {
        
        gf3d_effect_square_emitter(position, direction,0,count,config);
    }
}

/*eol@eof*/
