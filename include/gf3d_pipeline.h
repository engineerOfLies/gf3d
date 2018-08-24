#ifndef __GF3D_PIPELINE_H__
#define __GF3D_PIPELINE_H__

#include <vulkan/vulkan.h>

#include "gf3d_types.h"


typedef struct
{
    Bool inUse;
}Pipeline;

/**
 * @brief setup pipeline system
 */
void gf3d_pipeline_init();

/**
 * @brief free a created pipeline
 */
void gf3d_pipeline_free(Pipeline *pipe);
#endif
