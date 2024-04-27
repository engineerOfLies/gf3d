#ifndef __GF3D_GLTF_PARSE_H__
#define __GF3D_GLTF_PARSE_H__

#include "gf3d_model.h"

typedef enum
{
    G_CT_signedByte = 5120,
    G_CT_unsignedByte = 5121,
    G_CT_signedShort = 5122,
    G_CT_unsignedShort = 5123,
    G_CT_unsignedInt = 5125,
    G_CT_float = 5126
}GLTF_componentType;

typedef struct
{
    SJson *json;
    List  *buffers;
    TextLine filename;
}GLTF;

Model *gf3d_gltf_parse_model(const char *filename);


#endif
