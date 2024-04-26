#include "simple_logger.h"

#include "gfc_config.h"
#include "gf2d_actor.h"

#include "gf3d_draw.h"
#include "gf3d_armature.h"

typedef struct
{
    Uint32 maxArmatures;
    Armature3D *armatureList;
}ArmatureManager3D;

static ArmatureManager3D armature_manager3d = {0};

void gf3d_armature_delete(Armature3D *armature);//permanently clean up

Bone3D *gf3d_armature_bone_new();
void gf3d_armature_bone_free(Bone3D *bone);

Pose3D *gf3d_armature_pose_new();
void gf3d_armature_pose_free(Pose3D *pose);

BonePose3D *gf3d_armature_bone_pose_new();
void gf3d_armature_bone_pose_free(BonePose3D *pose);

Armature3D *gf3d_armature_load(const char *filename)
{
    SJson *config;
    Armature3D *armature;
    if (!filename)return NULL;
    config = sj_load(filename);
    if (!config)
    {
        slog("failed load armature file %s",filename);
        return NULL;
    }
    armature = gf3d_armature_parse(config);
    sj_free(config);
    if (!armature)
    {
        slog("failed to parse armature file %s",filename);
        return NULL;
    }
    return armature;
}

Bone3D *gf3d_armature_get_bone_by_id(Armature3D *armature,Uint32 id)
{
    int i,c;
    Bone3D *bone;
    if (!armature)return NULL;
    c = gfc_list_get_count(armature->bones);
    for (i = 0;i < c; i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        if (bone->nodeId == id)return bone;
    }
    return NULL;
}

Bone3D * gf3d_armature_get_root(Armature3D *arm)
{
    int i,c;
    Bone3D *bone = NULL;
    if (!arm)return NULL;
    c = gfc_list_get_count(arm->bones);
    for (i = 0;i < c; i++)
    {
        bone = gfc_list_get_nth(arm->bones,i);
        if (!bone)continue;
        if (bone->parent == NULL)return bone;
    }
    return NULL;
}

void gf3d_armature_calc_child_bone(Bone3D *parent)
{
    int i,c;
    Bone3D *child;
    if (!parent)return;
    c = gfc_list_get_count(parent->children);
    for (i = 0; i < c; i++)
    {
        child = gfc_list_get_nth(parent->children,i);
        if (!child)continue;
        //update child bone's matrix based on parent's matrix
        gfc_matrix_multiply(
            child->mat,
            child->mat,
            parent->mat
        );
        gf3d_armature_calc_child_bone(child);
    }
}

void gf3d_armature_calc_bone_mats(Armature3D *arm)
{
    if (!arm)return;
    gf3d_armature_calc_child_bone(gf3d_armature_get_root(arm));
}

void gf3d_armature_bone_make_parent(Bone3D *parent, Bone3D *child)
{
    if ((!parent)||(!child))return;
    child->parent = parent;
    if (!parent->children)parent->children = gfc_list_new();
    gfc_list_append(parent->children,child);
    //slog("%s is the parent of %s",parent->name,child->name);
}

void gf3d_armature_parse_joints(Armature3D *armature,SJson *nodes, SJson *joints)
{
    Matrix4 rotation,scaleM,transM;
    Bone3D *bone;
    SJson *node;
    Vector4D quaternion;
    Vector3D position,scale;
    const char *name;
    int nodeId,childId;
    int i,c,j,d;
    SJson *item,*children,*child;
    if ((!armature)||(!nodes)||(!joints))return;
    c = sj_array_get_count(joints);
   // slog("skin has %i joints",c);
    for (i = 0;i < c; i++)
    {
        item = sj_array_get_nth(joints,i);
        if (!item)continue;
        if (!sj_get_integer_value(item,&nodeId))continue;
        node = sj_array_get_nth(nodes,nodeId);
        if (!node)continue;
        name = sj_object_get_value_as_string(node,"name");
        if (!name)continue;
        bone = gf3d_armature_bone_new();
        if (!bone)continue;
        bone->nodeId = nodeId;
        gfc_line_cpy(bone->name,name);
        
        position = vector3d(0,0,0);
        scale = vector3d(1,1,1);
        quaternion = vector4d(0,0,0,0);
        
        sj_object_get_vector3d(node,"translation",&position);
        sj_object_get_vector3d(node,"scale",&scale);
        sj_object_get_vector4d(node,"rotation",&quaternion);

        gfc_matrix4_from_vectors(
            transM,
            position,
            vector3d(0,0,0),
            vector3d(1,1,1)
        );
        
        gfc_matrix_from_quaternion(rotation,quaternion);
        gfc_matrix_scale(scaleM,scale);
        
        //M = T * R * S
        
        gfc_matrix_multiply(
            bone->mat,
            rotation,
            transM
        );
        
        gfc_list_append(armature->bones,bone);
        slog("rotation: %f,%f,%f,%f",quaternion.x,quaternion.y,quaternion.z,quaternion.w);
        slog("parsed bone %s:",bone->name);
        gfc_matrix4_slog(rotation);
    }
    //second pass for parenting
    for (i = 0;i < c; i++)
    {
        item = sj_array_get_nth(joints,i);
        if (!item)continue;
        if (!sj_get_integer_value(item,&nodeId))continue;
        node = sj_array_get_nth(nodes,nodeId);
        if (!node)continue;
        name = sj_object_get_value_as_string(node,"name");
        if (!name)continue;//in case any were skipped, we skip em too
        children = sj_object_get_value(node,"children");
        if (!children)continue;
        bone = gf3d_armature_get_bone_by_id(armature,nodeId);
        if (!bone)
        {
            slog("failed to find bone with id %i",nodeId);
            continue;
        }
        d = sj_array_get_count(children);
        for (j = 0; j < d;j++)
        {
            child = sj_array_get_nth(children,j);
            if (!child)continue;
            if (!sj_get_integer_value(child,&childId))continue;
            gf3d_armature_bone_make_parent(bone, gf3d_armature_get_bone_by_id(armature,childId));
        }
    }
    //now figure out the matrices in global space
    gf3d_armature_calc_bone_mats(armature);
}

Armature3D *gf3d_armature_parse(SJson *config)
{
    int i,c;
    SJson *array,*item;
    Armature3D *armature;
    if (!config)return NULL;
    array = sj_object_get_value(config,"skins");
    if (!array)
    {
        slog("does not contain armature data!");
        return NULL;
    }
    armature = gf3d_armature_new();
    if (!armature)
    {
        return NULL;
    }
    c = sj_array_get_count(array);
    slog("armature has %i skins",c);
    for (i = 0;i < c;i++)
    {
        item = sj_array_get_nth(array,i);
        if (!item)continue;
        gf3d_armature_parse_joints(armature,sj_object_get_value(config,"nodes"),sj_object_get_value(item,"joints"));
    }
    slog("parsed %i bones for the armature",gfc_list_get_count(armature->bones));
    return armature;
}

void gf3d_armature_bone_draw(Bone3D *bone)
{
    Edge3D edge;
    Vector3D position = {0},rotation = {0}, scale = {1,1,1};
    if (!bone)return;
    if (!bone->parent)return;//skip the root bone
    gfc_matrix4_to_vectors(
        bone->parent->mat,
        &edge.a,
        NULL,
        NULL);
    gfc_matrix4_to_vectors(
        bone->mat,
        &edge.b,
        NULL,
        NULL);
    
    vector3d_scale(edge.a,edge.a,100);
    vector3d_scale(edge.b,edge.b,100);
    
    gf3d_draw_edge_3d(edge,position,rotation,scale,1,GFC_COLOR_LIGHTORANGE);
}

void gf3d_armature_draw_bones(Armature3D *arm)
{
    Bone3D *bone;
    int i,c;
    if ((!arm)||(!arm->bones))return;
    c = gfc_list_get_count(arm->bones);
    for (i = 0;i < c;i++)
    {
        bone = gfc_list_get_nth(arm->bones,i);
        if (!bone)continue;
        gf3d_armature_bone_draw(bone);
    }
}

void gf3d_armature_system_close()
{
    int i;
    if (armature_manager3d.armatureList != NULL)
    {
        for (i = 0;i < armature_manager3d.maxArmatures;i++)
        {
            gf3d_armature_free(&armature_manager3d.armatureList[i]);
        }
        free(armature_manager3d.armatureList);
    }
    memset(&armature_manager3d,0,sizeof(ArmatureManager3D));
    slog("armature system closed");
}

void gf3d_armature_system_init(Uint32 maxArmatures)
{
    if (!maxArmatures)
    {
        slog("cannot initialize armature system for zero entities");
        return;
    }
    
    armature_manager3d.armatureList = (Armature3D*)malloc(sizeof(Armature3D)*maxArmatures);
    if (!armature_manager3d.armatureList)
    {
        slog("failed to allocate armature list");
        gf3d_armature_system_close();
        return;
    }
    memset(armature_manager3d.armatureList,0,sizeof(Armature3D)*maxArmatures);
    armature_manager3d.maxArmatures = maxArmatures;
    atexit(gf3d_armature_system_close);
    slog("3d armature system initialized");
}

Armature3D *gf3d_armature_new()
{
    int i;
    /*search for an unused armature address*/
    for (i = 0;i < armature_manager3d.maxArmatures;i++)
    {
        if ((armature_manager3d.armatureList[i].refCount == 0)&&(armature_manager3d.armatureList[i].bones == NULL))
        {
            memset(&armature_manager3d.armatureList[i],0,sizeof(Armature3D));
            armature_manager3d.armatureList[i].refCount = 1;//set ref count
            armature_manager3d.armatureList[i].bones = gfc_list_new();
            armature_manager3d.armatureList[i].poses = gfc_list_new();
            armature_manager3d.armatureList[i].actions = gfc_list_new();


            return &armature_manager3d.armatureList[i];//return address of this array element        }
        }
    }
    /*find an unused armature address and clean up the old data*/
    for (i = 0;i < armature_manager3d.maxArmatures;i++)
    {
        if (armature_manager3d.armatureList[i].refCount <= 0)
        {
            gf3d_armature_delete(&armature_manager3d.armatureList[i]);// clean up the old data
            armature_manager3d.armatureList[i].refCount = 1;//set ref count
            armature_manager3d.armatureList[i].bones = gfc_list_new();
            armature_manager3d.armatureList[i].poses = gfc_list_new();
            armature_manager3d.armatureList[i].actions = gfc_list_new();
            return &armature_manager3d.armatureList[i];//return address of this array element
        }
    }
    slog("error: out of armature addresses");
    return NULL;
}

void gf3d_armature_delete(Armature3D *armature)
{
    int i,c;
    if (!armature)return;
    gf2d_action_list_delete(armature->actions);
    if (armature->bones != NULL)
    {
        c = gfc_list_get_count(armature->bones);
        for (i = 0; i < c; i++)
        {
            gf3d_armature_bone_free(gfc_list_get_nth(armature->bones,i));
        }
        gfc_list_delete(armature->bones);
    }
    if (armature->poses != NULL)
    {
        c = gfc_list_get_count(armature->poses);
        for (i = 0; i < c; i++)
        {
            gf3d_armature_pose_free(gfc_list_get_nth(armature->poses,i));
        }
        gfc_list_delete(armature->poses);
    }
    memset(armature,0,sizeof(Armature3D));
}

void gf3d_armature_free(Armature3D *armature)
{
    if (!armature) return;
    armature->refCount--;
}

void gf3d_armature_bone_free(Bone3D *bone)
{
    if (!bone)return;
    if (bone->children != NULL)
    {
        //delete the list, but not the bones, bones owned elsewhere
        gfc_list_delete(bone->children);
        bone->children = NULL;
    }
    free(bone);
}

Bone3D *gf3d_armature_bone_new()
{
    Bone3D *bone;
    bone = gfc_allocate_array(sizeof(Bone3D),1);
    if (!bone)return NULL;
    bone->children = gfc_list_new();
    return bone;
}

BonePose3D *gf3d_armature_bone_pose_new()
{
    BonePose3D *pose;
    pose = gfc_allocate_array(sizeof(BonePose3D),1);
    if (!pose)return NULL;
    return pose;
}

void gf3d_armature_bone_pose_free(BonePose3D *pose)
{
    if (!pose)return;
    free(pose);
}

void gf3d_armature_pose_free(Pose3D *pose)
{
    int i,c;
    if (!pose)return;
    if (pose->poseBones != NULL)
    {
        c = gfc_list_get_count(pose->poseBones);
        for (i = 0; i < c; i++)
        {
            gf3d_armature_bone_pose_free(gfc_list_get_nth(pose->poseBones,i));
        }
        gfc_list_delete(pose->poseBones);
        pose->poseBones = NULL;
    }
    free(pose);
}

Pose3D *gf3d_armature_pose_new()
{
    Pose3D *pose;
    pose = gfc_allocate_array(sizeof(Pose3D),1);
    if (!pose)return NULL;
    pose->poseBones = gfc_list_new();
    return pose;
}

/*eol@eof*/
