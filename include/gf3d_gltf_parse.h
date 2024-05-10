#ifndef __GF3D_GLTF_PARSE_H__
#define __GF3D_GLTF_PARSE_H__

#include "simple_json.h"
#include "gfc_types.h"

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
    GFC_List  *buffers;
    GFC_TextLine filename;
}GLTF;

/**
 * @brief load and decode the binary data from an embedded gltf file
 * @param filename the gltf file to load
 * @return NULL on error, or the gltlf file otherwise.
 * @note must be freed with gf3d_gltf_free when you are done
 */
GLTF *gf3d_gltf_load(const char *filename);

/**
 * @brief get the bufferIndex and count for a given accessor
 * @note you will still need to know what type you are getting.
 * @param gltf the gltf to extract from
 * @param accessorIndex which accessor to ... access
 * @param bufferIndex [output] this will be populated the buffer view index for the accessor
 * @param count [output] this will be populated with the number of items
 * @return the name of the type, or NULL if this fails
 */
const char *gf3d_gltf_accessor_get_details(GLTF* gltf,Uint32 accessorIndex, int *bufferIndex, int *count);

/**
 * @brief get the data from a buffer view in the gltf
 * @param gltf the one to extract from
 * @param viewIndex which buffer view to extract with
 * @param buffer [output] make sure you have enough space.  Get the details from gf3d_gltf_accessor_get_details
 */
void gf3d_gltf_get_buffer_view_data(GLTF *gltf,Uint32 viewIndex,char *buffer);

/**
 * @brief free the decoded gltf file data and the json.
 * @param gltf the data to free
 */
void gf3d_gltf_free(GLTF *gltf);


#endif
