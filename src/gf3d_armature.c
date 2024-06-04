#include "simple_logger.h"

#include "gfc_config.h"
#include "gf2d_actor.h"

#include "gf3d_draw.h"
#include "gf3d_armature.h"

extern int __DEBUG;

typedef struct
{
    Uint32 maxArmatures;
    Armature3D *armatureGFC_List;
}ArmatureManager3D;

static ArmatureManager3D armature_manager3d = {0};

void gf3d_armature_delete(Armature3D *armature);//permanently clean up
void gf3d_armature_parse_poses(Armature3D *armature,GLTF *gltf);

Bone3D *gf3d_armature_bone_new();
void gf3d_armature_bone_free(Bone3D *bone);

Pose3D *gf3d_armature_pose_new(Uint32 boneCount);
void gf3d_armature_pose_free(Pose3D *pose);

BonePose3D *gf3d_armature_bone_pose_new();
void gf3d_armature_bone_pose_free(BonePose3D *pose);

//grab a bone pose from a bone by its frame id.
BonePose3D *gf3d_armature_bone_get_pose_bone(Bone3D *bone,Uint32 frame);
Bone3D *gf3d_armature_get_bone_by_id(Armature3D *armature,Uint32 id);

int gf3d_armature_get_bone_index(Armature3D *armature,Uint32 boneId)
{
    Bone3D *bone;
    if (!armature)return -1;
    bone = gf3d_armature_get_bone_by_id(armature,boneId);
    if (!bone)return -1;
    return gfc_list_get_item_index(armature->bones,bone);
}

/**
 * @brief add a pose for a bone given a frame.
 * @param armature the armature in question
 * @param bone the bone to add to
 * @param translation if provided, updates the translation
 * @param rotation if provided, updates the rotation
 * @param scale if provided, updates the scale (which is defaulted to 1,1,1)
 */
void gf3d_armature_add_bone_pose_data(
    Armature3D *armature,
    Bone3D *bone,
    Uint32 frame,
    GFC_Vector3D *translation,
    GFC_Vector4D *rotation,
    GFC_Vector3D *scale);


Armature3D *gf3d_armature_load(const char *filename)
{
    GLTF *gltf;
    Armature3D *armature;
    if (!filename)return NULL;
    gltf = gf3d_gltf_load(filename);
    if (!gltf)
    {
        slog("failed load armature from file %s",filename);
        return NULL;
    }
    armature = gf3d_armature_parse(gltf);
    gf3d_armature_parse_poses(armature,gltf);
    gf3d_gltf_free(gltf);
    if (!armature)
    {
        slog("failed to parse armature file %s",filename);
        return NULL;
    }
    return armature;
}

GFC_Matrix4 *gf3d_armature_get_bone_matrix_by_name(Armature3D *armature,const char *name)
{
    Bone3D *bone;
    bone = gf3d_armature_get_bone_by_name(armature,name);
    if (!bone)return NULL;
    return &bone->mat;
}

GFC_Matrix4 *gf3d_armature_get_bone_pose_matrix_by_name(Armature3D *armature,Uint32 frame,const char *name)
{
    Bone3D *bone;
    BonePose3D *pose;
    bone = gf3d_armature_get_bone_by_name(armature,name);
    if (!bone)return NULL;
    pose = gfc_list_get_nth(bone->poses,frame);
    if (!pose)return NULL;
    return &pose->globalMat;
}


GFC_Matrix4 *gf3d_armature_get_pose_matrices(Armature3D *armature,Uint32 frame,Uint32 *boneCount)
{
    Pose3D *pose;
    if (!armature)
    {
        slog("null armature");
        return NULL;
    }
    pose = gfc_list_get_nth(armature->poses,frame);
    if (!pose)
    {
        slog("failed to find armature pose %i",frame);
        return NULL;
    }
    if (boneCount)*boneCount = pose->boneCount;
    return pose->bones;
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


Bone3D *gf3d_armature_get_bone_by_name(Armature3D *armature,const char *name)
{
    int i,c;
    Bone3D *bone;
    if ((!armature)||(!name))return NULL;
    c = gfc_list_get_count(armature->bones);
    for (i = 0;i < c; i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        if (gfc_line_cmp(name,bone->name) == 0)return bone;
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
        gfc_matrix4_multiply(
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

void gf3d_armature_bone_calc_joint_matrices(Bone3D *bone,GFC_Matrix4 inverseBindGFC_Matrix)
{
    BonePose3D *pose;
    int i,c;
    if ((!bone)||(!inverseBindGFC_Matrix))return;
    c = gfc_list_get_count(bone->poses);
    for (i = 0;i < c; i++)
    {
        pose = gfc_list_get_nth(bone->poses,i);
        if (!pose)continue;
        gfc_matrix4_multiply(
            pose->jointMat,
            inverseBindGFC_Matrix,
            pose->globalMat);
    }
}

void gf3d_armature_calc_joint_matrices(Armature3D *armature)
{
    Bone3D *bone;
    int i,c;
    if (!armature)return;
    c = gfc_list_get_count(armature->bones);
    for (i = 0; (i < c)&&(i < armature->bindCount);i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        gf3d_armature_bone_calc_joint_matrices(bone,armature->inverseBindMatrices[i]);
    }
}

void gf3d_armature_calc_child_posebone(Bone3D *parent,Uint32 frame)
{
    int i,c;
    Bone3D *child;
    BonePose3D *childPose;
    BonePose3D *myPose;
    if (!parent)return;
    c = gfc_list_get_count(parent->children);
    myPose = gfc_list_get_nth(parent->poses,frame);
    if (!myPose)return;//bad frame
    for (i = 0; i < c; i++)
    {
        child = gfc_list_get_nth(parent->children,i);
        if (!child)continue;
        //update child psoe bone's matrix based on parent's matrix
        childPose = gfc_list_get_nth(child->poses,frame);
        if (!childPose)continue;//bad frame
        gfc_matrix4_multiply(
            childPose->globalMat,
            childPose->globalMat,
            myPose->globalMat
        );
        gf3d_armature_calc_child_posebone(child,frame);
    }    
}

void gf3d_armature_calculate_poses(Armature3D *armature)
{
    int i,c,j;
    Bone3D *bone;
    BonePose3D *bonePose;
    Pose3D *pose;
    if (!armature)return;
    c = gfc_list_get_count(armature->bones);
    for (i = 0; i < armature->maxFrames;i++)
    {
        gf3d_armature_calc_child_posebone(gf3d_armature_get_root(armature),i);
    }
    gf3d_armature_calc_joint_matrices(armature);
    for (i = 0; i < armature->maxFrames;i++)
    {
        //for each frame...
        pose = gf3d_armature_pose_new(c);
        if (!pose)continue;
        //for reach bone
        for (j = 0;j < c;j++)
        {
            //copy it to the pose
            bone = gfc_list_get_nth(armature->bones,j);
            if (!bone)continue;
            bonePose = gfc_list_get_nth(bone->poses,i);//for this frame
            if (!bonePose)continue;
            gfc_matrix4_copy(
                pose->bones[j],
                bonePose->jointMat);
        }
        gfc_list_append(armature->poses,pose);
    }
}

void gf3d_armature_parse_joints(Armature3D *armature,SJson *nodes, SJson *joints)
{
    Bone3D *bone;
    SJson *node;
    GFC_Vector4D quaternion;
    GFC_Vector3D position,scale;
    const char *name;
    int nodeId,childId;
    int i,c,j,d;
    SJson *item,*children,*child;
    if ((!armature)||(!nodes)||(!joints))return;
    c = sj_array_get_count(joints);
   // slog("skin has %i joints",c);
    for (i = 0;i < c; i++)
    {
        item = sj_array_get_nth(joints,i);//    int i,c;

        if (!item)continue;
        if (!sj_get_integer_value(item,&nodeId))continue;
        node = sj_array_get_nth(nodes,nodeId);
        if (!node)continue;
        name = sj_object_get_value_as_string(node,"name");
        if (!name)continue;
        bone = gf3d_armature_bone_new();
        if (!bone)continue;
        bone->nodeId = nodeId;
        bone->index = i;
        gfc_line_cpy(bone->name,name);
        
        position = gfc_vector3d(0,0,0);
        scale = gfc_vector3d(1,1,1);
        quaternion = gfc_vector4d(0,0,0,0);
        
        sj_object_get_vector3d(node,"translation",&position);
        sj_object_get_vector3d(node,"scale",&scale);
        sj_object_get_vector4d(node,"rotation",&quaternion);

        gfc_matrix4_from_vectors_q(
                    bone->mat,
                    position,
                    quaternion,
                    scale);
        gfc_list_append(armature->bones,bone);
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
    gf3d_armature_calc_bone_mats(armature);
}

/**
 * @brief get the interpolations for the bone poses for a given timestamp
 * @param bone the bone in question
 * @param timestamp the search criteria
 * @param translation [output] if provided, this will be populated with the translation at timestamp.  defaults to 0,0,0
 * @param rotation [output] if provided, this will be populated with the rotation at timestamp.  defaults to 0,0,0,0
 * @param scale [output] if provided, this will be populated with the scale at timestamp.  defaults to 1,1,1
 */
void gf3d_armature_bone_get_interpolation_for_timestamp(
    Bone3D *bone,
    float timestamp,
    GFC_Vector3D *translation,
    GFC_Vector4D *rotation,
    GFC_Vector3D *scale)
{
    int i,done;
    int last,next;
    float time,delta;
    GFC_Vector3D a,b;
    GFC_Vector4D A,B;
    if (!bone)return;
    if ((translation)&&(bone->translations)&&(bone->translationTimestamps))
    {
        //first check if this is already a keyframe
        done = 0;
        for (i = 0; i < bone->translationCount;i++)
        {
            if (bone->translationTimestamps[i] == timestamp)
            {
                gfc_vector3d_copy((*translation),bone->translations[i]);
                done = 1;
                break;
            }
        }
        //now find the previous and next keyframes
        if (!done)
        {
            for (i = 0,last = -1,next = -1; i < bone->translationCount;i++)
            {
                if (bone->translationTimestamps[i] < timestamp)
                {
                    last = i;
                }
                else if (bone->translationTimestamps[i] > timestamp)
                {
                    next = i;
                    break;
                }
            }
            if ((last == -1)&&(next != -1))
            {
                //there is no keyframe before this, so just use the next keyframe without interpolation
                gfc_vector3d_copy((*translation),bone->translations[next]);
            }
            else if ((next == -1)&&(last != -1))
            {
                //there is no keyframe after this, so just use the last keyframe without interpolation
                gfc_vector3d_copy((*translation),bone->translations[last]);
            }
            else if ((next != -1)&&(last != -1))
            {
                //we have keyframes before and after.  lets interpolate
                time = bone->translationTimestamps[next] - bone->translationTimestamps[last];//time span
                delta = timestamp - bone->translationTimestamps[last];
                if (!time)
                {
                    //something is wrong
                    *translation = gfc_vector3d(0,0,0);
                }
                else
                {
                    time = delta / time;//now a percentage of the way from Last to Next;
                    gfc_vector3d_scale(a,bone->translations[last],time);
                    gfc_vector3d_scale(b,bone->translations[next],1.0 - time);
                    gfc_vector3d_add((*translation),a,b);
                }
            }
            else
            {
                //there are no keyframes this channel, set it to defaults
                *translation = gfc_vector3d(0,0,0);
            }
        }
    }
    
    //rotation
    if ((rotation)&&(bone->rotations)&&(bone->rotationTimestamps))
    {
        //first check if this is already a keyframe
        done = 0;
        for (i = 0; i < bone->rotationCount;i++)
        {
            if (bone->rotationTimestamps[i] == timestamp)
            {
                gfc_vector4d_copy((*rotation),bone->rotations[i]);
                done = 1;
                break;
            }
        }
        //now find the previous and next keyframes
        if (!done)
        {
            for (i = 0,last = -1,next = -1; i < bone->rotationCount;i++)
            {
                if (bone->rotationTimestamps[i] < timestamp)
                {
                    last = i;
                }
                else if (bone->rotationTimestamps[i] > timestamp)
                {
                    next = i;
                    break;
                }
            }
            if ((last == -1)&&(next != -1))
            {
                //there is no keyframe before this, so just use the next keyframe without interpolation
                gfc_vector4d_copy((*rotation),bone->rotations[next]);
            }
            else if ((next == -1)&&(last != -1))
            {
                //there is no keyframe after this, so just use the last keyframe without interpolation
                gfc_vector4d_copy((*rotation),bone->rotations[last]);
            }
            else if ((next != -1)&&(last != -1))
            {
                //we have keyframes before and after.  lets interpolate
                time = bone->rotationTimestamps[next] - bone->rotationTimestamps[last];//time span
                delta = timestamp - bone->rotationTimestamps[last];
                if (!time)
                {
                    //something is wrong
                    *rotation = gfc_vector4d(0,0,0,0);
                }
                else
                {
                    time = delta / time;//now a percentage of the way from Last to Next;
                    gfc_vector4d_scale(A,bone->rotations[last],time);
                    gfc_vector4d_scale(B,bone->rotations[next],1.0 - time);
                    gfc_vector4d_add((*rotation),A,B);
                }
            }
            else
            {
                //there are no keyframes this channel, set it to defaults
                *rotation = gfc_vector4d(0,0,0,0);
            }
        }
    }

    
    //scale
    if ((scale)&&(bone->scales)&&(bone->scaleTimestamps))
    {
        //first check if this is already a keyframe
        done = 0;
        for (i = 0; i < bone->scaleCount;i++)
        {
            if (bone->scaleTimestamps[i] == timestamp)
            {
                gfc_vector3d_copy((*scale),bone->scales[i]);
                done = 1;
                break;
            }
        }
        //now find the previous and next keyframes
        if (!done)
        {
            for (i = 0,last = -1,next = -1; i < bone->scaleCount;i++)
            {
                if (bone->scaleTimestamps[i] < timestamp)
                {
                    last = i;
                }
                else if (bone->scaleTimestamps[i] > timestamp)
                {
                    next = i;
                    break;
                }
            }
            if ((last == -1)&&(next != -1))
            {
                //there is no keyframe before this, so just use the next keyframe without interpolation
                gfc_vector3d_copy((*scale),bone->scales[next]);
            }
            else if ((next == -1)&&(last != -1))
            {
                //there is no keyframe after this, so just use the last keyframe without interpolation
                gfc_vector3d_copy((*scale),bone->scales[last]);
            }
            else if ((next != -1)&&(last != -1))
            {
                //we have keyframes before and after.  lets interpolate
                time = bone->scaleTimestamps[next] - bone->scaleTimestamps[last];//time span
                delta = timestamp - bone->scaleTimestamps[last];
                if (!time)
                {
                    //something is wrong
                    *scale = gfc_vector3d(1,1,1);
                }
                else
                {
                    time = delta / time;//now a percentage of the way from Last to Next;
                    gfc_vector3d_scale(a,bone->scales[last],time);
                    gfc_vector3d_scale(b,bone->scales[next],1.0 - time);
                    gfc_vector3d_add((*scale),a,b);
                }
            }
            else
            {
                //there are no keyframes this channel, set it to defaults
                *scale = gfc_vector3d(1,1,1);
            }
        }
    }
}

void gf3d_armature_bone_build_poses(Bone3D *bone,Uint32 maxFrames, float maxTimestamp,float timeStep)
{
    int i;
    BonePose3D *pose;
    GFC_Vector3D translation;
    GFC_Vector4D rotation;
    GFC_Vector3D scale;
    if (!bone)return;
    if (!bone->poses)bone->poses = gfc_list_new();

    for (i = 0; i < maxFrames; i++)
    {
        pose = gf3d_armature_bone_pose_new();
        if (!pose)continue;
        pose->bone = bone;
        pose->timestamp = i*timeStep;
        //now determine the interpolation for the three metrics
        //defaults:
        translation = gfc_vector3d(0,0,0);
        rotation = gfc_vector4d(0,0,0,0);
        scale = gfc_vector3d(1,1,1);
        gf3d_armature_bone_get_interpolation_for_timestamp(
            bone,
            pose->timestamp,
            &translation,
            &rotation,
            &scale);
        
        gfc_matrix4_from_vectors_q(
            pose->localMat,
            translation,
            rotation,
            scale);
        
        gfc_matrix4_copy(
            pose->globalMat,
            pose->localMat);//for later, this will be updated to global
        //slog("pose matrix for bone %s frame %i timestamp %f",bone->name,i,pose->timestamp);
        //gfc_matrix4_slog(pose->localMat);
        gfc_list_append(bone->poses,pose);
    }
}

void gf3d_armature_bone_get_maximums(Bone3D *bone,Uint32 *maxFrames,float *maxTimestamp)
{
    if (!bone)return;
    if (maxFrames)
    {
        *maxFrames = MAX(*maxFrames,MAX(bone->translationCount,MAX(bone->rotationCount,bone->scaleCount)));
    }
    if (maxTimestamp)
    {
        if (bone->translationTimestamps)
        {
            *maxTimestamp = MAX(*maxTimestamp,bone->translationTimestamps[bone->translationCount - 1]);
        }
        if (bone->rotationTimestamps)
        {
            *maxTimestamp = MAX(*maxTimestamp,bone->rotationTimestamps[bone->rotationCount - 1]);
        }
        if (bone->scaleTimestamps)
        {
            *maxTimestamp = MAX(*maxTimestamp,bone->scaleTimestamps[bone->scaleCount - 1]);
        }
    }
}

void gf3d_armature_parse_build_poses(Armature3D *armature)
{
    int i,c;
    float timeStep = 0;
    Uint32 maxFrames = 0;
    float  maxTimestamp = 0;
    Bone3D *bone;
    if (!armature)return;
    c = gfc_list_get_count(armature->bones);
    //first get the maximum timestamp and frame
    for (i = 0; i < c; i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        gf3d_armature_bone_get_maximums(bone,&maxFrames,&maxTimestamp);
    }
    if (maxFrames <= 1)
    {
        slog("Armature has not enough key frames");
        return;
    }
    armature->maxFrames = maxFrames;
    timeStep = maxTimestamp / (maxFrames -1);
//    slog("for the whole armature, there are %i frames taking %f seconds, with a step of %f",maxFrames,maxTimestamp,timeStep);
    for (i = 0; i < c; i++)
    {
        bone = gfc_list_get_nth(armature->bones,i);
        if (!bone)continue;
        gf3d_armature_bone_build_poses(bone,maxFrames, maxTimestamp,timeStep);
    }
    //calculate all bone global matrices, put them in index order and populate the armature->poses
    gf3d_armature_calculate_poses(armature);
}

void gf3d_armature_parse_animation(Armature3D *armature,GLTF *gltf,SJson *animation)
{
    int bufferIndex;
    int itemCount;
    int i,c;
    float *times = NULL;
    GFC_Vector3D *translations = NULL;
    GFC_Vector4D *rotations = NULL;
    GFC_Vector3D *scales = NULL;
    Bone3D *bone;
    const char *path;
    int timeAccessor,valuesAccessor;
    int samplerId,targetNode;
    SJson *channels,*channel,*samplers, *sampler;
    if ((!armature)||(!gltf)||(!animation))return;
    channels = sj_object_get_value(animation,"channels");
    samplers = sj_object_get_value(animation,"samplers");
    if ((!channels)||(!samplers))
    {
        slog("animation missing channels or samplers for %s",gltf->filename);
    }
    c = sj_array_get_count(channels);
    if (!c)return;
    for (i = 0; i < c;i++)
    {
        channel = sj_array_get_nth(channels,i);
        if (!channel)continue;
        if (!sj_object_get_value_as_int(channel,"sampler",&samplerId))continue;
        if (!sj_object_get_value_as_int(sj_object_get_value(channel,"target"),"node",&targetNode))continue;
        path = sj_object_get_value_as_string(sj_object_get_value(channel,"target"),"path");
        if (!path)continue;
        bone = gf3d_armature_get_bone_by_id(armature,targetNode);
        if (!bone)continue;
        sampler = sj_array_get_nth(samplers,samplerId);
        if (!sampler)continue;
        if (!sj_object_get_value_as_int(sampler,"input",&timeAccessor))continue;
        if (!sj_object_get_value_as_int(sampler,"output",&valuesAccessor))continue;
        
        if (gf3d_gltf_accessor_get_details(gltf,timeAccessor, &bufferIndex, (int *)&itemCount))
        {
            times = (float *)gfc_allocate_array(sizeof(float),itemCount);
            gf3d_gltf_get_buffer_view_data(gltf,bufferIndex,(char *)times);            
        }
        else slog("failed to get accessor detials");
        
        if (strcmp(path,"translation")==0)
        {
            if (gf3d_gltf_accessor_get_details(gltf,valuesAccessor, &bufferIndex, (int *)&itemCount))
            {
                translations = (GFC_Vector3D *)gfc_allocate_array(sizeof(GFC_Vector3D),itemCount);                
                gf3d_gltf_get_buffer_view_data(gltf,bufferIndex,(char *)translations); 
                
                bone->translations = translations;
                bone->translationTimestamps = times;
                bone->translationCount = itemCount;
            }
            else slog("failed to get accessor detials");
        }
        else if (strcmp(path,"rotation")==0)
        {
            if (gf3d_gltf_accessor_get_details(gltf,valuesAccessor, &bufferIndex, (int *)&itemCount))
            {
                rotations = (GFC_Vector4D *)gfc_allocate_array(sizeof(GFC_Vector4D),itemCount);                
                gf3d_gltf_get_buffer_view_data(gltf,bufferIndex,(char *)rotations);            
                
                bone->rotations = rotations;
                bone->rotationTimestamps = times;
                bone->rotationCount = itemCount;

            }
            else slog("failed to get accessor detials");
        }
        else if (strcmp(path,"scale")==0)
        {
            if (gf3d_gltf_accessor_get_details(gltf,valuesAccessor, &bufferIndex, (int *)&itemCount))
            {
                scales = (GFC_Vector3D *)gfc_allocate_array(sizeof(GFC_Vector3D),itemCount);
                gf3d_gltf_get_buffer_view_data(gltf,bufferIndex,(char *)scales);    
                
                bone->scales = scales;
                bone->scaleTimestamps = times;
                bone->scaleCount = itemCount;
            }
            else slog("failed to get accessor detials");
        }
    }
    gf3d_armature_parse_build_poses(armature);
}

void gf3d_armature_parse_poses(Armature3D *armature,GLTF *gltf)
{
    int i,c;
    SJson *animations,*animation;
    if ((!armature)||(!gltf))return;
    animations = sj_object_get_value(gltf->json,"animations");
    c = sj_array_get_count(animations);
    for (i = 0;i < c;i++)
    {
        animation = sj_array_get_nth(animations,i);
        if (!animation)continue;
        gf3d_armature_parse_animation(armature,gltf,animation);
    }
}


void gf3d_armature_parse_inverseBindMatrices(Armature3D *armature,GLTF *gltf,SJson *skin)
{
    int valuesAccessor;
    int bufferIndex;
    if ((!armature)||(!gltf)||(!skin))return;
    if (!sj_object_get_value_as_int(skin,"inverseBindMatrices",&valuesAccessor))return;
    if (gf3d_gltf_accessor_get_details(gltf,valuesAccessor, &bufferIndex, (int *)&armature->bindCount))
    {
        armature->inverseBindMatrices = (GFC_Matrix4 *)gfc_allocate_array(sizeof(GFC_Matrix4),armature->bindCount);
        gf3d_gltf_get_buffer_view_data(gltf,bufferIndex,(char *)armature->inverseBindMatrices);
    }
}

//TODO handle more than one armature / skin 
Armature3D *gf3d_armature_parse(GLTF *gltf)
{
    int i,c;
    SJson *array,*item;
    Armature3D *armature;
    if (!gltf)return NULL;
    array = sj_object_get_value(gltf->json,"skins");
    if (!array)
    {
        slog("does not contain armature data!");
        return NULL;
    }
    c = sj_array_get_count(array);
    for (i = 0;i < c;i++)
    {
        item = sj_array_get_nth(array,i);
        if (!item)continue;
        armature = gf3d_armature_new();
        if (!armature)continue;
        gf3d_armature_parse_inverseBindMatrices(armature,gltf,item);
        gf3d_armature_parse_joints(armature,sj_object_get_value(gltf->json,"nodes"),sj_object_get_value(item,"joints"));
        break;//only supporting one for now
    }
    return armature;
}

BonePose3D *gf3d_armature_bone_pose_get_parent(BonePose3D *pose)
{
    Uint32 frame;
    if ((!pose)||(!pose->bone)||(!pose->bone->parent))return NULL;
    frame = gfc_list_get_item_index(pose->bone->poses,pose);
    if (frame == -1)return NULL;//something is bad
    return gfc_list_get_nth(pose->bone->parent->poses,frame);
}

void gf3d_armature_posebone_draw(BonePose3D *pose)
{
    GFC_Edge3D edge;
    BonePose3D *parent;
    GFC_Vector3D position = {0},rotation = {0}, scale = {1,1,1};
    if (!pose)return;
    parent = gf3d_armature_bone_pose_get_parent(pose);
    if (!parent)return;//skip the root
    
    gfc_matrix4_to_vectors(
        parent->globalMat,
        &edge.a,
        NULL,
        NULL);
    gfc_matrix4_to_vectors(
        pose->globalMat,
        &edge.b,
        NULL,
        NULL);
    gfc_vector3d_scale(edge.a,edge.a,100);
    gfc_vector3d_scale(edge.b,edge.b,100);
    
    gf3d_draw_edge_3d(edge,position,rotation,scale,1,GFC_COLOR_LIGHTCYAN);
}

void gf3d_armature_draw_bone_poses(Armature3D *arm,Uint32 frame)
{
    Bone3D *bone;
    BonePose3D *pose;
    int i,c;
    if ((!arm)||(!arm->bones))return;
    c = gfc_list_get_count(arm->bones);
    for (i = 0;i < c;i++)
    {
        bone = gfc_list_get_nth(arm->bones,i);
        if (!bone)continue;
        pose = gfc_list_get_nth(bone->poses,frame);
        if (!pose)continue;
        gf3d_armature_posebone_draw(pose);
    }
}


void gf3d_armature_bone_draw(Bone3D *bone)
{
    GFC_Edge3D edge;
    GFC_Vector3D position = {0},rotation = {0}, scale = {1,1,1};
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
    
    gfc_vector3d_scale(edge.a,edge.a,100);
    gfc_vector3d_scale(edge.b,edge.b,100);
    
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
    if (armature_manager3d.armatureGFC_List != NULL)
    {
        for (i = 0;i < armature_manager3d.maxArmatures;i++)
        {
            gf3d_armature_free(&armature_manager3d.armatureGFC_List[i]);
        }
        free(armature_manager3d.armatureGFC_List);
    }
    memset(&armature_manager3d,0,sizeof(ArmatureManager3D));
    if(__DEBUG)slog("armature system closed");
}

void gf3d_armature_system_init(Uint32 maxArmatures)
{
    if (!maxArmatures)
    {
        slog("cannot initialize armature system for zero entities");
        return;
    }
    
    armature_manager3d.armatureGFC_List = (Armature3D*)malloc(sizeof(Armature3D)*maxArmatures);
    if (!armature_manager3d.armatureGFC_List)
    {
        slog("failed to allocate armature list");
        gf3d_armature_system_close();
        return;
    }
    memset(armature_manager3d.armatureGFC_List,0,sizeof(Armature3D)*maxArmatures);
    armature_manager3d.maxArmatures = maxArmatures;
    atexit(gf3d_armature_system_close);
    if(__DEBUG)slog("3d armature system initialized");
}

Armature3D *gf3d_armature_new()
{
    int i;
    /*search for an unused armature address*/
    for (i = 0;i < armature_manager3d.maxArmatures;i++)
    {
        if ((armature_manager3d.armatureGFC_List[i].refCount == 0)&&(armature_manager3d.armatureGFC_List[i].bones == NULL))
        {
            memset(&armature_manager3d.armatureGFC_List[i],0,sizeof(Armature3D));
            armature_manager3d.armatureGFC_List[i].refCount = 1;//set ref count
            armature_manager3d.armatureGFC_List[i].bones = gfc_list_new();
            armature_manager3d.armatureGFC_List[i].poses = gfc_list_new();
            armature_manager3d.armatureGFC_List[i].actions = gfc_action_list_new();


            return &armature_manager3d.armatureGFC_List[i];//return address of this array element        }
        }
    }
    /*find an unused armature address and clean up the old data*/
    for (i = 0;i < armature_manager3d.maxArmatures;i++)
    {
        if (armature_manager3d.armatureGFC_List[i].refCount <= 0)
        {
            gf3d_armature_delete(&armature_manager3d.armatureGFC_List[i]);// clean up the old data
            armature_manager3d.armatureGFC_List[i].refCount = 1;//set ref count
            armature_manager3d.armatureGFC_List[i].bones = gfc_list_new();
            armature_manager3d.armatureGFC_List[i].poses = gfc_list_new();
            armature_manager3d.armatureGFC_List[i].actions = gfc_action_list_new();
            return &armature_manager3d.armatureGFC_List[i];//return address of this array element
        }
    }
    slog("error: out of armature addresses");
    return NULL;
}

void gf3d_armature_delete(Armature3D *armature)
{
    int i,c;
    if (!armature)return;
    gfc_action_list_free(armature->actions);
    if (armature->inverseBindMatrices)
    {
        free(armature->inverseBindMatrices);
    }
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
    if (bone->poses != NULL)
    {
        //delete the list, but not the bones, bones owned elsewhere
        gfc_list_delete(bone->poses);
        bone->poses = NULL;
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
    gfc_matrix4_identity(pose->localMat);
    gfc_matrix4_identity(pose->globalMat);
    gfc_matrix4_identity(pose->jointMat);
    return pose;
}

void gf3d_armature_bone_pose_free(BonePose3D *pose)
{
    if (!pose)return;
    free(pose);
}

void gf3d_armature_pose_free(Pose3D *pose)
{
    if (!pose)return;
    if (pose->bones)free(pose->bones);
    free(pose);
}

Pose3D *gf3d_armature_pose_new(Uint32 boneCount)
{
    Pose3D *pose;
    pose = gfc_allocate_array(sizeof(Pose3D),1);
    if (!pose)return NULL;
    if (boneCount)
    {
        pose->bones = (GFC_Matrix4*)gfc_allocate_array(sizeof(GFC_Matrix4),boneCount);
        pose->boneCount = boneCount;
    }
    return pose;
}

ArmatureUBO gf3d_armature_get_ubo(
    Armature3D *armature,
    Uint32      frame)
{
    GFC_Matrix4 *bones;
    Uint32 boneCount = 0;
    ArmatureUBO boneUbo = {0};
    
    if (!armature)return boneUbo;
    
    bones = gf3d_armature_get_pose_matrices(armature,frame,&boneCount);
    if ((bones)&&(boneCount <= MAX_SHADER_BONES))
    {
        memcpy(boneUbo.bones,bones,sizeof(GFC_Matrix4)*boneCount);
    }
    else if (__DEBUG)slog("no bones for armature %s at frame %i",armature->filepath,frame);

    return boneUbo;
}

/*eol@eof*/
