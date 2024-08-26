#ifndef __GF3D_PARTICLES_H__
#define __GF3D_PARTICLES_H__

#include <stdalign.h>
#include <vulkan/vulkan.h>

#include "gfc_vector.h"
#include "gfc_color.h"
#include "gfc_list.h"
#include "gfc_primitives.h"

#include "gf2d_sprite.h"
#include "gf3d_pipeline.h"

typedef struct
{
    GFC_Vector3D position;
    GFC_Color color;
    GFC_Color color2;   //for color blending
    float size;
}Particle;

/**
 * @brief initialize the particle drawing subsystem
 * @param max_particles the limit of concurrent particles to draw
 */
void gf3d_particle_manager_init(Uint32 max_particles);

/**
 * @brief set a particle
 * @return a set particle
 */
Particle gf3d_particle(GFC_Vector3D position, GFC_Color color, float size);

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
void gf3d_particle_draw(Particle particle);

/**
 * @brief draw a single particle this frame with a texture
 * @param particle the particle to draw
 * @param texture the texture to use for the particle
 */
void gf3d_particle_draw_textured(Particle particle,Texture *texture);

/**
 * @brief draw a particle as an animated sprite
 * @param particle the particle to draw
 * @param sprite the sprite to draw with
 * @param frame the frame of animation to draw
 */
void gf3d_particle_draw_sprite(Particle particle,Sprite *sprite,int frame);

/**
 * @brief draw a trail particles from tail.a to tail.b
 * @param color the color of the particles to draw
 * @param size the size of the particles to draw
 * @param count how many particles to fill in
 * @param trail the path of particles to draw
 */
void gf3d_particle_trail_draw(GFC_Color color, float size, Uint8 count, GFC_Edge3D trail);

/**
 * @brief draw an array of particles
 */
void gf3d_particle_draw_array(Particle *particle,Uint32 count);

/**
 * @brief draw a list of particles this frame
 * @param list list of GF3D_Particle's
 */
void gf3d_particle_draw_list(GFC_List *list);

/**
 * @brief get the related pipeline for the particle system
 * @return NULL if not set, or the pipeline
 */
Pipeline *gf3d_particle_get_pipeline();

/**
 * @brief draw a line of red and green particles in parallel from the position, based on the rotation provided.
 * @param postion where to center the lines on
 * @param rotation used to determine rotation of the lines
 * @param width how far apart the lines will be
 * @param length how long the lines will be
 */
void draw_guiding_lights(GFC_Vector3D position,GFC_Vector3D rotation,float width, float length);

#endif
