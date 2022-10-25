#ifndef __GF3D_PARTICLES_H__
#define __GF3D_PARTICLES_H__

#include <vulkan/vulkan.h>

#include "gfc_vector.h"
#include "gfc_color.h"
#include "gfc_list.h"

#include "gf3d_pipeline.h"

typedef struct
{
    Vector3D position;
    Color color;
    float size;
}Particle;

/**
 * @brief initialize the particle drawing subsystem
 * @param max_particles the limit of concurrent particles to draw
 */
void gf3d_particle_manager_init(Uint32 max_particles);

/**
 * @brief needs to be called once at the beginning of each render frame
 */
void gf3d_particle_reset_pipes();

/**
 * @brief called to submit all draw commands to the particle pipelines
 */
void gf3d_particle_submit_pipe_commands();

/**
 * @brief draw a single particle this frame
 * @param particle the particle to draw
 */
void gf3d_particle_draw(Particle *particle);

/**
 * @brief draw a list of particles this frame
 * @param list list of GF3D_Particle's
 */
void gf3d_particle_draw_list(List *list);

/**
 * @brief get the related pipeline for the particle system
 * @return NULL if not set, or the pipeline
 */
Pipeline *gf3d_particle_get_pipeline();

#endif
