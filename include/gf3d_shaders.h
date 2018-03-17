#ifndef __GF3D_SHADERS_H__
#define __GF3D_SHADERS_H__

/**
 * @brief loads vertex and fragment shaders, links them and returns a handle to the shader program
 * @param vertfile the filepath to the vertex shader to use
 * @param fragfile the filepath to the fragment shader to use
 * @return 0 on error (check logs) or a valid gl handle
 */
GLuint gf3d_shader_program_load(char *vertfile, char *fragfile);

#endif
