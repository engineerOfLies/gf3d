#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"
#include "gfc_shape.h"
#include "gfc_pak.h"

#include "gf2d_draw.h"
#include "gf2d_actor.h"
#include "gf2d_armature.h"

typedef struct
{
    Uint32 maxArmatures;
    Armature2D *armatureGFC_List;
}ArmatureManager;

void gf2d_armature_delete(Armature2D *armature);
void gf2d_armature_bone_free(Bone *bone);
void gf2d_armature_pose_free(Pose *pose);
BonePose *gf2d_armature_bone_pose_new();
Pose *gf2d_armature_pose_new();
void gf2d_armature_bone_scale(Bone *bone,GFC_Vector2D scale, GFC_Vector2D center);
Bone gf2d_armature_bone_scaled_copy(Bone *bone,GFC_Vector2D scale,GFC_Vector2D center);

static ArmatureManager armature_manager = {0};

void gf2d_armature_system_close()
{
    int i;
    if (armature_manager.armatureGFC_List != NULL)
    {
        for (i = 0;i < armature_manager.maxArmatures;i++)
        {
            gf2d_armature_free(&armature_manager.armatureGFC_List[i]);
        }
        free(armature_manager.armatureGFC_List);
    }
    memset(&armature_manager,0,sizeof(ArmatureManager));
    slog("armature system closed");
}

void gf2d_armature_init(Uint32 maxArmatures)
{
    if (!maxArmatures)
    {
        slog("cannot initialize armature system for zero entities");
        return;
    }
    memset(&armature_manager,0,sizeof(ArmatureManager));
    
    armature_manager.armatureGFC_List = (Armature2D*)malloc(sizeof(Armature2D)*maxArmatures);
    if (!armature_manager.armatureGFC_List)
    {
        slog("failed to allocate armature list");
        gf2d_armature_system_close();
        return;
    }
    memset(armature_manager.armatureGFC_List,0,sizeof(Armature2D)*maxArmatures);
    armature_manager.maxArmatures = maxArmatures;
    atexit(gf2d_armature_system_close);
    slog("armature system initialized");
}

Armature2D *gf2d_armature_new()
{
    int i;
    /*search for an unused armature address*/
    for (i = 0;i < armature_manager.maxArmatures;i++)
    {
        if ((armature_manager.armatureGFC_List[i].refCount == 0)&&(armature_manager.armatureGFC_List[i].bones == NULL))
        {
            memset(&armature_manager.armatureGFC_List[i],0,sizeof(Armature2D));
            armature_manager.armatureGFC_List[i].refCount = 1;//set ref count
            armature_manager.armatureGFC_List[i].bones = gfc_list_new();
            armature_manager.armatureGFC_List[i].poses = gfc_list_new();
            armature_manager.armatureGFC_List[i].actions = gfc_action_list_new();


            return &armature_manager.armatureGFC_List[i];//return address of this array element        }
        }
    }
    /*find an unused armature address and clean up the old data*/
    for (i = 0;i < armature_manager.maxArmatures;i++)
    {
        if (armature_manager.armatureGFC_List[i].refCount <= 0)
        {
            gf2d_armature_delete(&armature_manager.armatureGFC_List[i]);// clean up the old data
            armature_manager.armatureGFC_List[i].refCount = 1;//set ref count
            armature_manager.armatureGFC_List[i].bones = gfc_list_new();
            armature_manager.armatureGFC_List[i].poses = gfc_list_new();
            armature_manager.armatureGFC_List[i].actions = gfc_action_list_new();
            return &armature_manager.armatureGFC_List[i];//return address of this array element
        }
    }
    slog("error: out of armature addresses");
    return NULL;
}

void gf2d_armature_renumber_bones(Armature2D *armature)
{
    Bone *bone;
    int i,c;
    if (!armature)return;
    c = gfc_list_get_count(armature->bones);
    for (i = 0; i < c; i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        bone->index = i;
    }
}

void gf2d_armature_delete_pose_by_index(Armature2D *armature,Uint32 index)
{
    Pose *pose;
    if (!armature)return;
    pose = gfc_list_get_nth(armature->poses,index);
    if (!pose)return;
    gf2d_armature_pose_free(pose);
    gfc_list_delete_data(armature->poses,pose);
}

void gf2d_armature_delete(Armature2D *armature)
{
    int i,c;
    if (!armature)return;
    gfc_action_list_free(armature->actions);
    if (armature->bones != NULL)
    {
        c = gfc_list_get_count(armature->bones);
        for (i = 0; i < c; i++)
        {
            gf2d_armature_bone_free(gfc_list_get_nth(armature->bones,i));
        }
        gfc_list_delete(armature->bones);
    }
    if (armature->poses != NULL)
    {
        c = gfc_list_get_count(armature->poses);
        for (i = 0; i < c; i++)
        {
            gf2d_armature_pose_free(gfc_list_get_nth(armature->poses,i));
        }
        gfc_list_delete(armature->poses);
    }
    memset(armature,0,sizeof(Armature2D));
}

void gf2d_armature_free(Armature2D *armature)
{
    if (!armature) return;
    armature->refCount--;
}

GFC_Action *gf2d_armature_get_action(Armature2D *armature, const char *name,float *frame)
{
    GFC_Action *action;
    if (!armature)return NULL;
    action = gfc_action_list_get_action_by_name(armature->actions,name);
    if (!action)return NULL;
    if (frame)*frame = action->startFrame;
    return action;
}

GFC_Action *gf2d_armature_get_action_by_index(Armature2D *armature,Uint32 index)
{
    if (!armature)return NULL;
    return gfc_action_list_get_action_by_index(armature->actions,index);
}


BonePose *gf2d_armature_bone_pose_new()
{
    BonePose *pose;
    pose = gfc_allocate_array(sizeof(BonePose),1);
    if (!pose)return NULL;
    return pose;
}

void gf2d_armature_bone_pose_free(BonePose *pose)
{
    if (!pose)return;
    free(pose);
}

void gf2d_armature_delete_bones_from_pose_by_bone(Pose *pose,Bone *bone)
{
    BonePose *posebone;
    int i,c;
    if ((!pose)||(!bone))return;
    c = gfc_list_get_count(pose->poseBones);
    for (i = 0; i < c; i++)
    {
        posebone = gfc_list_get_nth(pose->poseBones,i);
        if (!posebone)continue;
        if (posebone->bone == bone)
        {//found it
            gf2d_armature_bone_pose_free(posebone);
            gfc_list_delete_data(pose->poseBones,posebone);
            return;//deleted it, no longer safe to use the list this round
        }
    }
}

void gf2d_armature_delete_pose_bones_by_bone(Armature2D *armature,Bone *bone)
{
    Pose *pose;
    int i,c;
    if ((!armature)||(!bone))return;
    c = gfc_list_get_count(armature->poses);
    for (i = 0; i < c; i++)
    {
        pose = gfc_list_get_nth(armature->poses,i);
        if (!pose)return;
        gf2d_armature_delete_bones_from_pose_by_bone(pose,bone);
    }
}

void gf2d_armature_pose_free(Pose *pose)
{
    int i,c;
    if (!pose)return;
    if (pose->poseBones != NULL)
    {
        c = gfc_list_get_count(pose->poseBones);
        for (i = 0; i < c; i++)
        {
            gf2d_armature_bone_pose_free(gfc_list_get_nth(pose->poseBones,i));
        }
        gfc_list_delete(pose->poseBones);
        pose->poseBones = NULL;
    }
    free(pose);
}

Pose *gf2d_armature_pose_new()
{
    Pose *pose;
    pose = gfc_allocate_array(sizeof(Pose),1);
    if (!pose)return NULL;
    pose->poseBones = gfc_list_new();
    return pose;
}

void gf2d_armature_add_bone_to_parent(Bone *parent,Bone *child)
{
    if ((!parent)||(!child))return;
    child->parent = parent;
    gfc_list_append(parent->children,child);
}

Bone *gf2d_armature_duplicate_bone(Armature2D *armature, Bone *bone)
{
    Bone *child;
    Bone *newbone;
    int i,c;
    if ((!armature)||(!bone))return NULL;

    newbone = gf2d_armature_add_bone(armature,NULL);
    if (!newbone)return NULL;
    gfc_line_sprintf(newbone->name,"%s.dup",bone->name);
    newbone->baseAngle = bone->baseAngle;
    newbone->length = bone->length;
    gfc_vector2d_copy(newbone->rootPosition,bone->rootPosition);
    
    c = gfc_list_get_count(bone->children);
    for (i = 0; i < c; i++)
    {
        child = gfc_list_get_nth(bone->children,i);
        if (!child)continue;
        child = gf2d_armature_duplicate_bone(armature, child);
        gf2d_armature_add_bone_to_parent(newbone,child);
    }
    return newbone;
}

void gf2d_armature_bone_parent_remove_child(Bone *parent, Bone *child)
{
    if ((!parent)||(!child))return;
    gfc_list_delete_data(parent->children,child);
}

void gf2d_armature_delete_bone_recurse(Armature2D *armature, Bone *bone);

void gf2d_armature_delete_bone(Armature2D *armature, Bone *bone)
{
    Bone *child;
    int i,c;
    if ((!armature)||(!bone))return;
    c = gfc_list_get_count(bone->children);
    if ((bone->parent)&&(bone->parent->children))
    {
        gfc_list_delete_data(bone->parent->children,bone);
    }
    for (i = 0; i < c; i++)
    {
        child = gfc_list_get_nth(bone->children,i);
        if (!child)continue;
        child->parent = bone->parent;// remove me from the lineage
        if ((bone->parent)&&(bone->parent->children))
        {
            gfc_list_append(bone->parent->children,child);
        }
    }
    gf2d_armature_delete_pose_bones_by_bone(armature,bone);
    gfc_list_delete_data(armature->bones,bone);
    gf2d_armature_bone_free(bone);
    gf2d_armature_renumber_bones(armature);
}

void gf2d_armature_delete_bone_recurse(Armature2D *armature, Bone *bone)
{
    Bone *child;
    int i,c;
    if ((!armature)||(!bone))return;
    c = gfc_list_get_count(bone->children);
    for (i = 0; i < c; i++)
    {
        child = gfc_list_get_nth(bone->children,i);
        if (!child)continue;
        gf2d_armature_delete_bone_recurse(armature, child);
    }
    gf2d_armature_delete_pose_bones_by_bone(armature,bone);// clean up any pose bones referencing me
    gf2d_armature_bone_free(bone);
    gfc_list_delete_data(armature->bones,bone);
}

void gf2d_armature_bone_free(Bone *bone)
{
    if (!bone)return;
    if (bone->children != NULL)
    {
        gfc_list_delete(bone->children);
        bone->children = NULL;
    }
    free(bone);
}

Bone *gf2d_armature_bone_new()
{
    Bone *bone;
    bone = gfc_allocate_array(sizeof(Bone),1);
    if (!bone)return NULL;
    bone->children = gfc_list_new();
    return bone;
}

Bone *gf2d_armature_get_bone_by_name(Armature2D *armature, const char *name)
{
    Bone *bone;
    int i,c;
    if (!armature)return NULL;
    if (!name)return NULL;
    c = gfc_list_get_count(armature->bones);
    for (i = 0; i < c; i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        if (strcmp(name,bone->name) == 0)
        {
            return bone;
        }
    }
    slog("armature %s has no bone with name %s",armature->filepath,name);
    return NULL;
}

Bone *gf2d_armature_get_bone_by_index(Armature2D *armature, Uint32 index)
{
    Bone *bone;
    if (!armature)return NULL;
    bone = gfc_list_get_nth(armature->bones,index);
    if (!bone)
    {
        slog("armature %s has no bone with index %i",armature->filepath,index);
    }
    return bone;
}

BonePose *gf2d_armature_get_bone_pose_by_name(Armature2D *armature,Uint32 pose, const char *name)
{
    Bone *bone;
    bone = gf2d_armature_get_bone_by_name(armature, name);
    if (!bone)return NULL;
    return gf2d_armature_get_bone_pose(armature,pose, bone->index);
}


BonePose *gf2d_armature_get_bone_pose(Armature2D *armature,Uint32 poseIndex, Uint32 index)
{
    Pose *pose;
    BonePose *bonepose;
    if (!armature)return NULL;
    pose = gfc_list_get_nth(armature->poses,poseIndex);
    if (!pose)
    {
        slog("armature %s has no pose %i",armature->filepath,poseIndex);
        return NULL;
    }
    bonepose = gfc_list_get_nth(pose->poseBones,index);
    if (!bonepose)
    {
        slog("armature %s has no bonepose %i",armature->filepath,index);
    }
    return bonepose;
}

GFC_Vector2D gf2d_armature_get_pose_bone_position(BonePose *posebone)
{
    GFC_Vector2D position = {0};
    if ((!posebone)||(!posebone->bone))return position;
    gfc_vector2d_add(position,posebone->position,posebone->bone->rootPosition);
    return position;
}

GFC_Vector2D gf2d_armature_get_pose_bone_draw_position(BonePose *posebone,GFC_Vector2D scale,float rotation)
{
    GFC_Vector2D posePoint;
    Bone drawbone;
    GFC_Vector2D drawPosition = {0};
    if ((!posebone)||(!posebone->bone))return drawPosition;
    drawbone = gf2d_armature_bone_scaled_copy(posebone->bone,scale, gfc_vector2d(0,0));

    gfc_vector2d_scale_by(posePoint,posebone->position,scale);
    
    gfc_vector2d_add(drawPosition,drawbone.rootPosition,posePoint);
    
    return drawPosition;
}

BonePose gf2d_armature_get_tweened_pose_bone(Armature2D *armature,Uint32 poseA, Uint32 poseB,Uint32 index, float weight)
{
    BonePose *A,*B, result = {0};
    float d;
    A = gf2d_armature_get_bone_pose(armature,poseA, index);
    if (!A)return result;
    memcpy(&result,A,sizeof(BonePose));
    B = gf2d_armature_get_bone_pose(armature,poseB, index);
    if (!B)return result;
    d = gfc_angle_between_radians(A->angle, B->angle) * weight;
    

    result.angle = A->angle + d;
    result.position.x = (B->position.x * weight) + (A->position.x * (1 - weight));;
    result.position.y = (B->position.y * weight) + (A->position.y * (1 - weight));;
    return result;
}

void gf2d_armature_draw_tweened_pose(
    Armature2D *armature,
    Uint32 poseA,
    Uint32 poseB,
    float fraction,
    GFC_Vector2D position,
    GFC_Vector2D scale,
    float rotation,
    GFC_Color color)
{
    BonePose drawPose;
    int i,c;
    if (!armature)return;
    c = gfc_list_get_count(armature->bones);
    for (i = 0;i < c; i++)
    {
        drawPose = gf2d_armature_get_tweened_pose_bone(armature,poseA, poseB,i, fraction);
        if (!drawPose.bone)continue;//no bone, no draw
        gf2d_armature_draw_pose_bone(&drawPose,position,scale, rotation, color);
    }
}


void gf2d_armature_draw_sprite_to_named_bone_pose(
    Armature2D *armature,
    Sprite *sprite,
    Uint32 frame,
    GFC_Vector2D position,
    GFC_Vector2D * scale,
    GFC_Vector2D * center,
    float    * rotation,
    GFC_Vector2D * flip,
    GFC_Color    * colorShift,
    Uint32 pose,
    const char *name)
{
    Bone *bone;
    bone = gf2d_armature_get_bone_by_name(armature, name);
    if (!bone)return;
    gf2d_armature_draw_sprite_to_bone_pose(
        armature,
        sprite,
        frame,
        position,
        scale,
        center,
        rotation,
        flip,
        colorShift,
        pose,
        bone->index);
}

void gf2d_armature_draw_sprite_to_bone_pose(
    Armature2D *armature,
    Sprite *sprite,
    Uint32 frame,
    GFC_Vector2D position,
    GFC_Vector2D * scale,
    GFC_Vector2D * center,
    float    * rotation,
    GFC_Vector2D * flip,
    GFC_Color    * colorShift,
    Uint32 pose,
    Uint32 bone)
{
    float drawRotation = 0;
    GFC_Vector2D drawPosition;
    BonePose *bonePose = NULL;
    if ((!armature)||(!sprite))return;// nothing to do
    
    if (rotation)
    {
        drawRotation = *rotation;
    }
    bonePose = gf2d_armature_get_bone_pose(armature,pose, bone);
    if ((!bonePose)||(!bonePose->bone))return;
    
    // apply bonepose delta to base bone and requested information
    drawRotation += bonePose->bone->baseAngle + bonePose->angle;
    drawRotation *= GFC_RADTODEG;
    gfc_vector2d_add(drawPosition,bonePose->bone->rootPosition,bonePose->position);
    gfc_vector2d_add(drawPosition,drawPosition,position);
    
    gf2d_sprite_draw(
        sprite,
        drawPosition,
        scale,
        center,
        &drawRotation,
        flip,
        colorShift,
        NULL,
        frame);
}

void gf2d_armature_bone_draw(GFC_Vector2D position, float angle, float length, GFC_Color color)
{
    GFC_Vector2D dir,p2;
    dir = gfc_vector2d_from_angle(angle);
    gfc_vector2d_scale(dir,dir,length);
    gfc_vector2d_add(p2,position,dir);
    gf2d_draw_line(position,p2, color);
    gf2d_draw_circle(position, 5, color);
    gf2d_draw_circle(p2, 3, color);
}

void gf2d_armature_draw_pose_bone(BonePose *bonePose,GFC_Vector2D position,GFC_Vector2D scale, float rotation, GFC_Color color)
{
    GFC_Vector2D drawPosition;
    GFC_Vector2D drawPosePosition;
    float angle,length;
    Bone drawbone;
    if ((!bonePose)||(!bonePose->bone))return;
    
    drawbone = gf2d_armature_bone_scaled_copy(bonePose->bone,scale, gfc_vector2d(0,0));
    
    gfc_vector2d_scale_by(drawPosePosition,bonePose->position,scale);
    
    gfc_vector2d_add(drawPosition,drawbone.rootPosition,drawPosePosition);
    
    gfc_vector2d_add(drawPosition,drawPosition,position);//draw offset
    angle = drawbone.baseAngle + (bonePose->angle + rotation) * gfc_vector2d_scale_flip_rotation(scale);;
    length = drawbone.length + (bonePose->length * gfc_vector2d_magnitude(scale));
    gf2d_armature_bone_draw(drawPosition, angle, length, color);    
}

void gf2d_armature_draw_pose(
    Armature2D *armature,
    Uint32 poseindex,
    GFC_Vector2D position,
    GFC_Vector2D scale,
    float rotation,
    GFC_Color color)
{
    Pose *pose;
    BonePose *bonePose;
    int i,c;
    if (!armature)return;
    pose = gfc_list_get_nth(armature->poses,poseindex);
    if (!pose)return;// no pose with this index
    c = gfc_list_get_count(pose->poseBones);
    for (i = 0;i < c; i++)
    {
        bonePose = gfc_list_get_nth(pose->poseBones,i);
        if (!bonePose)continue;
        if (!bonePose->bone)continue;
        gf2d_armature_draw_pose_bone(bonePose,position,scale, rotation, color);
    }
}

void gf2d_armature_draw_bone(Bone *bone,GFC_Vector2D position, GFC_Vector2D scale, float rotation, GFC_Color color)
{
    Bone drawbone;
    GFC_Vector2D drawPosition;
    drawbone = gf2d_armature_bone_scaled_copy(bone,scale,gfc_vector2d(0,0));
    gfc_vector2d_add(drawPosition,position,drawbone.rootPosition);
    gf2d_armature_bone_draw(drawPosition, drawbone.baseAngle + rotation, drawbone.length, color);
}

void gf2d_armature_draw_bones(Armature2D *armature,GFC_Vector2D position, GFC_Vector2D scale, float rotation, GFC_Color color)
{
    GFC_Vector2D drawPosition;
    Bone *bone;
    Bone drawbone;
    int i,c;
    if (!armature)return;
    c = gfc_list_get_count(armature->bones);
    for (i = 0;i < c; i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        drawbone = gf2d_armature_bone_scaled_copy(bone,scale,gfc_vector2d(0,0));
        gfc_vector2d_add(drawPosition,position,drawbone.rootPosition);
        gf2d_armature_bone_draw(drawPosition, drawbone.baseAngle + rotation, drawbone.length, color);
    }
}

SJson *gf2d_armature_bone_to_json(Bone *bone)
{
    int i,c;
    Bone *child;
    SJson *save,*children;
    if (!bone)return NULL;
    save = sj_object_new();
    if (!save)return NULL;
    
    sj_object_insert(save,"name",sj_new_str(bone->name));
    sj_object_insert(save,"index",sj_new_int(bone->index));
    if (bone->parent)
    {
        sj_object_insert(save,"parent",sj_new_str(bone->parent->name));
    }
    
    c = gfc_list_get_count(bone->children);
    children = sj_array_new();
    for (i = 0;i < c;i++)
    {
        child = gfc_list_get_nth(bone->children,i);
        if (!child)continue;
        sj_array_append(children,sj_new_str(child->name));
    }
    sj_object_insert(save,"children",children);

    sj_object_insert(save,"baseAngle",sj_new_float(bone->baseAngle));
    sj_object_insert(save,"length",sj_new_float(bone->length));
    sj_object_insert(save,"rootPosition",sj_vector2d_new(bone->rootPosition));
    return save;
}

SJson *gf2d_armature_bones_to_json(Armature2D *armature)
{
    int i,c;
    SJson *bones;
    Bone *bone;
    if (!armature)return NULL;
    bones = sj_array_new();
    if (!bones)return NULL;
    c = gfc_list_get_count(armature->bones);
    for (i = 0;i < c;i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        sj_array_append(bones,gf2d_armature_bone_to_json(bone));
    }
    return bones;
}

SJson *gf2d_armature_bonepose_to_json(BonePose *bonepose)
{
    SJson *save;
    if (!bonepose)return NULL;
    save = sj_object_new();
    if (!save)return NULL;
    
    if (bonepose->bone)
    {
        sj_object_insert(save,"bone",sj_new_str(bonepose->bone->name));
    }
    sj_object_insert(save,"angle",sj_new_float(bonepose->angle));
    sj_object_insert(save,"length",sj_new_float(bonepose->length));
    sj_object_insert(save,"position",sj_vector2d_new(bonepose->position));
    return save;
}

SJson *gf2d_armature_pose_to_json(Pose *pose)
{
    int i,c;
    SJson *save;
    BonePose *bonepose;
    if (!pose)return NULL;
    save = sj_array_new();
    if (!save)return NULL;
    c = gfc_list_get_count(pose->poseBones);
    for (i = 0;i < c;i++)
    {
        bonepose = gfc_list_get_nth(pose->poseBones,i);
        if (!bonepose)continue;
        sj_array_append(save,gf2d_armature_bonepose_to_json(bonepose));
    }
    return save;
}

SJson *gf2d_armature_poses_to_json(Armature2D *armature)
{
    int i,c;
    SJson *poses;
    Pose *pose;
    if (!armature)return NULL;
    poses = sj_array_new();
    if (!poses)return NULL;
    c = gfc_list_get_count(armature->poses);
    for (i = 0;i < c;i++)
    {
        pose = gfc_list_get_nth(armature->poses,i);
        if (!pose)continue;
        sj_array_append(poses,gf2d_armature_pose_to_json(pose));
    }
    return poses;
}

void gf2d_armature_save(Armature2D *armature, const char *filepath)
{
    SJson *save;
    if ((!armature)||(!filepath))return;
    save = sj_object_new();
    if (!save)
    {
        slog("cannot save file %s: bad json",filepath);
        return;
    }
    gfc_line_cpy(armature->filepath,filepath);
    sj_object_insert(save,"filepath",sj_new_str(filepath));
    sj_object_insert(save,"name",sj_new_str(armature->name));
    sj_object_insert(save,"actionList",gfc_action_list_to_json(armature->actions));
    sj_object_insert(save,"bones",gf2d_armature_bones_to_json(armature));
    sj_object_insert(save,"poses",gf2d_armature_poses_to_json(armature));
    sj_save(save,filepath);
}

Bone *gf2d_armature_bone_parse(SJson *jBone)
{
    const char *name;
    Bone *bone;
    if (!jBone)return NULL;
    bone = gf2d_armature_bone_new();
    if (!bone)return NULL;
    name = sj_get_string_value(sj_object_get_value(jBone,"name"));
    gfc_line_cpy(bone->name,name);
    sj_get_float_value(sj_object_get_value(jBone,"baseAngle"),&bone->baseAngle);
    sj_get_float_value(sj_object_get_value(jBone,"length"),&bone->length);
    sj_value_as_vector2d(sj_object_get_value(jBone,"rootPosition"),&bone->rootPosition);
    return bone;
}

void gf2d_armature_bone_parse_link(Armature2D *armature,SJson *jBone,Bone *bone)
{
    SJson *children,*jChild;
    Bone *child;
    int i,c;
    const char *name;
    if ((!jBone)||(!bone))return;
    name = sj_get_string_value(sj_object_get_value(jBone,"parent"));
    if (name)
    {
        bone->parent = gf2d_armature_get_bone_by_name(armature, name);
    }
    children = sj_object_get_value(jBone,"children");
    c = sj_array_get_count(children);
    for (i = 0; i < c; i++)
    {
        jChild = sj_array_get_nth(children,i);
        if (!jChild)continue;
        name = sj_get_string_value(jChild);
        if (!name)continue;
        child = gf2d_armature_get_bone_by_name(armature, name);
        if (!child)
        {
            continue;
        }
        gfc_list_append(bone->children,child);
    }
}

BonePose *gf2d_armature_bone_pose_parse(Armature2D *armature,SJson *jPoseBone)
{
    const char *name;
    BonePose *posebone;
    if (!jPoseBone)return NULL;
    posebone = gf2d_armature_bone_pose_new();
    if (!posebone)return NULL;
    name = sj_get_string_value(sj_object_get_value(jPoseBone,"bone"));
    if (name)
    {
        posebone->bone = gf2d_armature_get_bone_by_name(armature, name);
    }
    sj_get_float_value(sj_object_get_value(jPoseBone,"angle"),&posebone->angle);
    sj_get_float_value(sj_object_get_value(jPoseBone,"length"),&posebone->length);
    sj_value_as_vector2d(sj_object_get_value(jPoseBone,"position"),&posebone->position);
    return posebone;
}

Pose *gf2d_armature_pose_parse(Armature2D *armature, SJson *jPose)
{
    int i,c;
    Pose *pose;
    SJson *jPoseBone;
    BonePose *posebone;
    if (!jPose)return NULL;
    pose = gf2d_armature_pose_new();
    if (!pose)return NULL;
    c = sj_array_get_count(jPose);
    for (i = 0;i < c;i++)
    {
        jPoseBone = sj_array_get_nth(jPose,i);
        if (!jPoseBone)continue;
        posebone = gf2d_armature_bone_pose_parse(armature,jPoseBone);
        if (!posebone)continue;
        gfc_list_append(pose->poseBones,posebone);
    }
    return pose;
}

Armature2D *gf2d_armature_get_by_filename(const char *filepath)
{
    int i;
    if (!filepath)return NULL;
    for (i = 0;i < armature_manager.maxArmatures;i++)
    {
        if (strcmp(armature_manager.armatureGFC_List[i].filepath,filepath) == 0)
        {
            return &armature_manager.armatureGFC_List[i];//found it
        }
    }
    return NULL;
}

Armature2D *gf2d_armature_load(const char *filepath)
{
    int i,c;
    const char *name = NULL;
    Armature2D *armature;
    Bone *bone;
    Pose *pose;
    SJson *json,*jBones,*jBone,*jPoses,*jPose;
    
    armature = gf2d_armature_get_by_filename(filepath);
    if (armature)
    {
        armature->refCount++;
        return armature;
    }
    
    json = gfc_pak_load_json(filepath);
    if (!json)return NULL;
    armature = gf2d_armature_new();
    if (!armature)
    {
        sj_free(json);
        return NULL;
    }
    gfc_line_cpy(armature->filepath,filepath);
    name = sj_get_string_value(sj_object_get_value(json,"name"));
    if (name)
    {
        gfc_line_cpy(armature->name,name);
    }
    //base bones pass
    armature->actions = gfc_action_list_parse(sj_object_get_value(json,"actionList"));

    jBones = sj_object_get_value(json,"bones");
    c = sj_array_get_count(jBones);
    for (i = 0;i < c; i++)
    {
        jBone = sj_array_get_nth(jBones,i);
        if (!jBone)continue;
        bone = gf2d_armature_bone_parse(jBone);
        if (!bone)continue;
        bone->index = i;
        gfc_list_append(armature->bones,bone);
    }
    //bone link pass
    for (i = 0;i < c; i++)
    {
        jBone = sj_array_get_nth(jBones,i);
        if (!jBone)continue;
        bone = gfc_list_get_nth(armature->bones,i);
        gf2d_armature_bone_parse_link(armature,jBone,bone);
    }    
    //bone poses
    jPoses = sj_object_get_value(json,"poses");
    c = sj_array_get_count(jPoses);
    for (i = 0;i < c;i++)
    {
        jPose = sj_array_get_nth(jPoses,i);
        if (!jPose)continue;
        pose = gf2d_armature_pose_parse(armature,jPose);
        if (!pose)continue;
        gfc_list_append(armature->poses,pose);
    }
    sj_free(json);
    return armature;
}

void gf2d_armature_bone_rotate_by_name(Armature2D *armature,const char *name, float angle)
{
    Bone *bone;
    bone = gf2d_armature_get_bone_by_name(armature, name);
    gf2d_armature_bone_rotate(bone, bone->rootPosition, angle);
}

GFC_Vector2D gf2d_armature_get_bone_tip(Bone *bone)
{
    GFC_Vector2D dir;
    GFC_Vector2D tip = {0,0};
    if (!bone)return tip;
    dir = gfc_vector2d_from_angle(bone->baseAngle);
    gfc_vector2d_scale(dir,dir,bone->length);
    gfc_vector2d_add(tip,bone->rootPosition,dir);
    return tip;
}

GFC_Vector2D gf2d_armature_get_bonepose_tip_by_name(
    Armature2D *armature,
    const char *bonename,
    Uint32 pose,
    GFC_Vector2D scale,
    float rotation)
{
    BonePose *posebone;
    posebone = gf2d_armature_get_bone_pose_by_name(armature,pose, bonename);
    return gf2d_armature_get_bonepose_tip(posebone,scale, rotation);
}

GFC_Vector2D gf2d_armature_get_bonepose_tip(BonePose *posebone,GFC_Vector2D scale, float rotation)
{
    Bone bonecopy;
    GFC_Vector2D dir;
    GFC_Vector2D tip = {0,0};
    if ((!posebone)||(!posebone->bone))return tip;
    bonecopy = gf2d_armature_bone_scaled_copy(posebone->bone,scale,gfc_vector2d(0,0));
    dir = gfc_vector2d_from_angle(bonecopy.baseAngle + posebone->angle + rotation);
    gfc_vector2d_scale(dir,dir,(bonecopy.length + (posebone->length * gfc_vector2d_magnitude(scale))));
    gfc_vector2d_add(tip,bonecopy.rootPosition,gfc_vector2d(posebone->position.x*scale.x,posebone->position.y*scale.y));
    gfc_vector2d_add(tip,tip,dir);
    return tip;
}


BonePose *gf2d_armature_get_bonepose_by_position(
    Armature2D *armature,
    Uint32 poseindex,
    GFC_Vector2D position,
    GFC_Vector2D scale,
    float rotation,
    BonePose *ignore)
{
    int i,c;
    GFC_Vector2D tip;
    Pose *pose;
    BonePose *bonepose;
    if (!armature)return NULL;
    pose = gfc_list_get_nth(armature->poses,poseindex);
    if (!pose)return NULL;
    c = gfc_list_get_count(pose->poseBones);
    for (i = 0; i < c; i++)
    {
        bonepose = gfc_list_get_nth(pose->poseBones,i);
        if (!bonepose)continue;
        if ((ignore)&&(ignore == bonepose))continue;
        tip = gf2d_armature_get_bonepose_tip(bonepose,scale,rotation);
        if (gfc_point_in_cicle(position,gfc_circle(tip.x,tip.y,10)))
        {
            return bonepose;
        }
    }
    return NULL;
}

Bone *gf2d_armature_get_bone_by_position(Armature2D *armature,GFC_Vector2D position,GFC_Vector2D scale,Bone *ignore)
{
    Bone *bone;
    Bone bonescale;
    GFC_Vector2D tip;
    int i,c;
    if (!armature)return NULL;
    c = gfc_list_get_count(armature->bones);
    for (i = 0;i < c;i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        if ((ignore)&&(ignore == bone))continue;
        bonescale = gf2d_armature_bone_scaled_copy(bone,scale,gfc_vector2d(0,0));
        tip = gf2d_armature_get_bone_tip(&bonescale);
        if (gfc_point_in_cicle(position,gfc_circle(tip.x,tip.y,10)))
        {
            return bone;
        }
    }
    return NULL;
}



void gf2d_armature_bone_calculate_from_tip(Bone *bone,GFC_Vector2D tip)
{
    GFC_Vector2D delta;
    if (!bone)return;
    gfc_vector2d_sub(delta,tip,bone->rootPosition);
    bone->length = gfc_vector2d_magnitude(delta);
    bone->baseAngle = gfc_vector2d_angle(delta);
}

void gf2d_armature_bone_tip_move(Bone *bone,GFC_Vector2D offset)
{
    GFC_Vector2D tip;
    if (!bone)return;
    tip = gf2d_armature_get_bone_tip(bone);
    gfc_vector2d_add(tip,tip,offset);
    gf2d_armature_bone_calculate_from_tip(bone,tip);
}


void gf2d_armature_bone_move(Bone *bone,GFC_Vector2D offset)
{
    Bone *child;
    int i,c;
    if (!bone)return;
    c = gfc_list_get_count(bone->children);
    gfc_vector2d_add(bone->rootPosition,bone->rootPosition,offset);
    for (i = 0;i < c;i++)
    {
        child = gfc_list_get_nth(bone->children,i);
        if (!child)continue;
        gf2d_armature_bone_move(child, offset);
    }
}

void gf2d_armature_bone_move_to(Bone *bone,GFC_Vector2D newPosition)
{
    GFC_Vector2D offset;
    if (!bone)return;
    gfc_vector2d_sub(offset,newPosition,bone->rootPosition);
    gf2d_armature_bone_move(bone,offset);
}

void gf2d_armature_bonepose_move(Armature2D *armature, BonePose *bonepose,Uint32 poseindex, GFC_Vector2D delta)
{
    Bone *bone;
    Bone *child;
    int i,c;
    if (!bonepose)return;
    bone = bonepose->bone;
    if (!bone)return;
    gfc_vector2d_add(bonepose->position,delta,bonepose->position);
    
    c = gfc_list_get_count(bone->children);
    for (i = 0;i < c;i++)
    {
        child = gfc_list_get_nth(bone->children,i);
        if (!child)continue;
        gf2d_armature_bonepose_move(
            armature,
            gf2d_armature_get_bone_pose(armature,poseindex, child->index),
            poseindex,
            delta);
    }    
}

void gf2d_armature_bonepose_rotate(Armature2D *armature, BonePose *bonepose,Uint32 poseindex, GFC_Vector2D center, float angle)
{
    Bone *bone;
    Bone *child;
    GFC_Vector2D posePosition;
    int i,c;
    if (!bonepose)return;
    bone = bonepose->bone;
    if (!bone)return;
    gfc_vector2d_add(posePosition,bone->rootPosition,bonepose->position);
    posePosition = gfc_vector2d_rotate_around_center(posePosition,angle, center);
    gfc_vector2d_sub(bonepose->position,posePosition,bone->rootPosition);
    bonepose->angle += angle;
    c = gfc_list_get_count(bone->children);
    for (i = 0;i < c;i++)
    {
        child = gfc_list_get_nth(bone->children,i);
        if (!child)continue;
        gf2d_armature_bonepose_rotate(
            armature,
            gf2d_armature_get_bone_pose(armature,poseindex, child->index),
            poseindex,
            center,
            angle);
    }    
}

void gf2d_armature_bone_rotate(Bone *bone, GFC_Vector2D center, float angle)
{
    Bone *child;
    int i,c;
    if (!bone)return;
    bone->rootPosition = gfc_vector2d_rotate_around_center(bone->rootPosition,angle, center);
    bone->baseAngle += angle;
    c = gfc_list_get_count(bone->children);
    for (i = 0;i < c;i++)
    {
        child = gfc_list_get_nth(bone->children,i);
        if (!child)continue;
        gf2d_armature_bone_rotate(child, center, angle);
    }
}

Bone *gf2d_armature_add_bone(Armature2D * armature,Bone *parent)
{
    int i,c;
    Pose *pose;
    BonePose *newbonepose;
    Bone *newbone;
    if (!armature)return NULL;
    newbone = gf2d_armature_bone_new();
    if (!newbone)return NULL;
    newbone->length = 10;
    newbone->index = gfc_list_get_count(armature->bones);
    gfc_list_append(armature->bones,newbone);
    if (parent)
    {
        newbone->parent = parent;
        gfc_list_append(parent->children,newbone);
        newbone->rootPosition = gf2d_armature_get_bone_tip(parent);
        gfc_line_sprintf(newbone->name,"%s.child",parent->name);
    }
    else
    {
        gfc_line_sprintf(newbone->name,"Bone");
    }
    c = gfc_list_get_count(armature->poses);
    for (i = 0; i < c;i++)
    {
        pose = gfc_list_get_nth(armature->poses,i);
        if (!pose)continue;
//        slog("adding new pose bone");
        newbonepose = gf2d_armature_bone_pose_new();
        newbonepose->bone = newbone;
        gfc_list_append(pose->poseBones,newbonepose);
    }
    return newbone;
}


void gf2d_armature_bone_scale(Bone *bone,GFC_Vector2D scale, GFC_Vector2D center)
{
    GFC_Vector2D point,tip,delta;
    if (!bone)return;
    gfc_vector2d_sub(point,bone->rootPosition,center);
    gfc_vector2d_scale_by(point,point,scale);
    tip = gf2d_armature_get_bone_tip(bone);
    gfc_vector2d_sub(tip,tip,center);
    gfc_vector2d_scale_by(tip,tip,scale);
    gfc_vector2d_sub(delta,tip,point);
    bone->length = gfc_vector2d_magnitude(delta);
    gfc_vector2d_add(bone->rootPosition,point,center);
    bone->baseAngle *= gfc_vector2d_scale_flip_rotation(scale);
}

void gf2d_armature_bone_scale_children(Bone *bone,GFC_Vector2D scale, GFC_Vector2D center)
{
    Bone *child;
    int i,c;
    if (!bone)return;
    gf2d_armature_bone_scale(bone,scale, center);
    c = gfc_list_get_count(bone->children);
    for (i = 0;i < c;i++)
    {
        child = gfc_list_get_nth(bone->children,i);
        if (!child)continue;
        gf2d_armature_bone_scale_children(child,scale, center);
    }
}

Pose *gf2d_armature_pose_create_for(Armature2D *armature)
{
    BonePose *newposebone;
    Bone *bone;
    int i,c;
    Pose *pose;
    if (!armature)return NULL;
    pose = gf2d_armature_pose_new();
    if (!pose)return NULL;
    c = gfc_list_get_count(armature->bones);
    for (i = 0;i < c;i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        newposebone = gf2d_armature_bone_pose_new();
        if (!newposebone)continue;
        newposebone->bone = bone;
        gfc_list_append(pose->poseBones,newposebone);
    }
    return pose;
}

Pose *gf2d_armature_pose_add_at_index(Armature2D *armature,Uint32 index)
{
    Pose *pose;
    if (!armature)return NULL;
    if (index > gfc_list_get_count(armature->poses))return NULL;
    pose = gf2d_armature_pose_create_for(armature);
    gfc_list_insert(armature->poses,pose,index);
    return pose;
}

Pose *gf2d_armature_pose_add(Armature2D *armature)
{
    Pose *pose;
    if (!armature)return NULL;
    pose = gf2d_armature_pose_create_for(armature);
    gfc_list_append(armature->poses,pose);
    return pose;
}

Pose *gf2d_armature_pose_get_by_index(Armature2D *armature,Uint32 index)
{
    if (!armature)return NULL;
    return gfc_list_get_nth(armature->poses,index);
}

Uint32 gf2d_armature_get_pose_count(Armature2D *armature)
{
    if (!armature)return 0;
    return gfc_list_get_count(armature->poses);
}

Bone gf2d_armature_bone_scaled_copy(Bone *bone,GFC_Vector2D scale,GFC_Vector2D center)
{
    Bone copy = {0};
    if (!bone)return copy;
    memcpy(&copy,bone,sizeof(Bone));
    gf2d_armature_bone_scale(&copy,scale,center);
    return copy;
}

Bone gf2d_armature_bone_copy(Bone *bone)
{
    Bone copy = {0};
    if (!bone)return copy;
    memcpy(&copy,bone,sizeof(Bone));
    return copy;
}

Pose *gf2d_armature_pose_duplicate_at_index(Armature2D *armature,Pose *pose,Uint32 index)
{
    int i,c;
    Pose *newpose;
    BonePose *oldbone,*newbone;
    if (!pose)return NULL;
    newpose = gf2d_armature_pose_add_at_index(armature,index);
    if (!newpose)return NULL;
    c = gfc_list_get_count(pose->poseBones);
    for (i = 0;i < c; i++)
    {
        oldbone = gfc_list_get_nth(pose->poseBones,i);
        newbone = gfc_list_get_nth(newpose->poseBones,i);
        if ((!oldbone)||(!newbone))continue;
        memcpy(newbone,oldbone,sizeof(BonePose));
    }
    return newpose;
}

void gf2d_armature_pose_copy(Pose *dst,Pose *src)
{
    int i,c,dc;
    BonePose *srcbone,*dstbone;
    if ((!dst)||(!src))return;
    
    c = gfc_list_get_count(src->poseBones);
    dc= gfc_list_get_count(dst->poseBones);
    if (dc != c)
    {
        slog("cannot copy pose, src and dst have different bone counts");
        return;
    }
    for (i = 0;i < c; i++)
    {
        srcbone = gfc_list_get_nth(src->poseBones,i);
        dstbone = gfc_list_get_nth(dst->poseBones,i);
        if ((!dstbone)||(!srcbone))continue;
        memcpy(dstbone,srcbone,sizeof(BonePose));
    }
}

Pose *gf2d_armature_pose_duplicate(Armature2D *armature,Pose *pose)
{
    int i,c;
    Pose *newpose;
    BonePose *oldbone,*newbone;
    if (!pose)return NULL;
    newpose = gf2d_armature_pose_add(armature);
    if (!newpose)return NULL;
    c = gfc_list_get_count(pose->poseBones);
    for (i = 0;i < c; i++)
    {
        oldbone = gfc_list_get_nth(pose->poseBones,i);
        newbone = gfc_list_get_nth(newpose->poseBones,i);
        if ((!oldbone)||(!newbone))continue;
        memcpy(newbone,oldbone,sizeof(BonePose));
    }
    return newpose;
}

void gf2d_armature_scale(Armature2D *armature,GFC_Vector2D scale,GFC_Vector2D center)
{
    Bone *bone;
    int i,c;
    if (!armature)return;
    slog("scaling armature by a factor of (%f,%f)",scale,scale);
    c = gfc_list_get_count(armature->bones);
    for (i = 0;i < c;i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        gf2d_armature_bone_scale(bone,scale,center);
    }
}

int gf2d_armature_bone_name_lr_flip(char *newname,const char *oldname)
{
    if ((!newname)||(!oldname))return 0;
    if (gfc_str_suffix(oldname, ".l"))
    {// get the .r one
        strcpy(newname, oldname);
        newname[strlen(oldname) - 1] = 'r';
        return 1;
    }
    else if (gfc_str_suffix(oldname, ".r"))
    {// get the .l one
        strcpy(newname, oldname);
        newname[strlen(oldname) - 1] = 'l';
        return 1;
    }
    else if (gfc_str_suffix(oldname, ".left"))
    {// get the .r one
        strncpy(newname, oldname,strlen(oldname) - 4);
        strcat(newname, "right");
        return 1;
    }
    else if (gfc_str_suffix(oldname, ".right"))
    {// get the .l one
        strncpy(newname, oldname,strlen(oldname) - 5);
        strcat(newname, "left");
        return 1;
    }
    return 0;
}


/*eol@eof*/
