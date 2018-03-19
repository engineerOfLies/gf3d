#include <GL/glew.h>

#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "simple_json.h"
#include "simple_logger.h"

typedef struct
{
    Uint32 max_models;
    Model *model_list;
}ModelManager;

static ModelManager model_manager;

void gf3d_model_manager_close()
{
    gf3d_model_clear_all();
    if (model_manager.model_list != NULL)
    {
        free(model_manager.model_list);
    }
    model_manager.model_list = NULL;
    model_manager.max_models = 0;
    slog("model system closed");
}

void gf3d_model_manager_init(Uint32 max)
{
    if (!max)
    {
        slog("cannot intialize a model manager for Zero models!");
        return;
    }
    model_manager.max_models = max;
    model_manager.model_list = (Model *)malloc(sizeof(Model)*max);
    memset (model_manager.model_list,0,sizeof(Model)*max);
    slog("model system initialized");
    atexit(gf3d_model_manager_close);
}

void gf3d_model_delete(Model *model)
{
    if (!model)return;
    // cleanup
    glDeleteBuffers(1,&model->vertex_buffer);
    glDeleteBuffers(1,&model->normal_buffer);
    glDeleteBuffers(1,&model->texel_buffer);
    memset(model,0,sizeof(Model));//clean up all other data
}

void gf3d_model_free(Model *model)
{
    if (!model) return;
    model->ref_count--;
}

void gf3d_model_clear_all()
{
    int i;
    for (i = 0;i < model_manager.max_models;i++)
    {
        gf3d_model_delete(&model_manager.model_list[i]);// clean up the data
    }
}

Model *gf3d_model_new()
{
    int i;
    /*search for an unused model address*/
    for (i = 0;i < model_manager.max_models;i++)
    {
        if (model_manager.model_list[i].ref_count == 0)
        {
            model_manager.model_list[i].ref_count = 1;//set ref count
            return &model_manager.model_list[i];//return address of this array element        }
        }
    }
    slog("error: out of model addresses");
    return NULL;
}

Model *gf3d_model_new_triangle()
{
    Model *model;
    GLuint vertexbuffer;
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
    }; 
    model = gf3d_model_new();
    if (!model)return NULL;

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    model->vertex_buffer = vertexbuffer;
    sprintf(model->filepath,"triangle");
    return model;
}

Model *gf3d_model_get_by_filename(char * filename)
{
    int i;
    for (i = 0;i < model_manager.max_models;i++)
    {
        if (gf3d_line_cmp(model_manager.model_list[i].filepath,filename)==0)
        {
            return &model_manager.model_list[i];
        }
    }
    return NULL;// not found
}

void gf3d_model_render(
    Model *model,
    Matrix4 mat,
    GLuint program
)
{
    Matrix4 identity,mvp,v,p,pv;
    GLuint mvp_id;
    if (!model)return;
    if (!mat)
    {
        gf3d_matrix_identity(identity);
        mat = identity;
    }
//  slog("rendering model %s, vertex buffer %i",model->filepath,model->vertex_buffer);

    // calculate Model View Projection
    
    gf3d_graphics_get_projection(p);
    gf3d_camera_get_view(v);
    
    gf3d_matrix_multiply(pv,p,v);
    gf3d_matrix_multiply(mvp,pv,mat);

    glUseProgram(program);
    mvp_id = glGetUniformLocation(program, "MVP");
    glUniformMatrix4fv(mvp_id, 1, GL_FALSE, &mvp[0][0]);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, model->vertex_buffer);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized
        0,                  // stride
        (void*)0            // array buffer offset
        );
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, model->vertex_count); // Starting from vertex 0; 3 vertices total -> 1 triangle
    glDisableVertexAttribArray(0);
}

GLuint gf3d_model_buffer_new(
    GLenum buffer_type,
    const void *data,
    GLsizei buffer_size
)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(buffer_type, buffer);
    glBufferData(buffer_type, buffer_size, data, GL_DYNAMIC_DRAW);
    return buffer;
}

GLuint gf3d_model_json_parse_array_data(SJson *array,Uint32 *outcount)
{
    SJson *item;
    Uint32 count,i;
    float *list;
    count = sj_array_get_count(array);
    if (count)
    {
        list = (float *)malloc(sizeof(float)*count);
        memset(list,0,sizeof(float)*count);
        for (i = 0; i < count; i++)
        {
            item = sj_array_get_nth(array,i);
            sj_get_float_value(item,&list[i]);
        }
        // data validation
        slog("data gathered from json file array:");
        for (i = 0; i < count;i++)
        {
            slog("value [%i] = %f",i,list[i]);
        }
        if (outcount)*outcount = count;
        return gf3d_model_buffer_new(GL_ARRAY_BUFFER,list,sizeof(float)*count);
    }
    return 0;
}

int gf3d_model_json_parse(Model *model,SJson *data)
{
    SJson *attributes,*positions;
    if ((!model)||(!data))
    {
        slog("gf3d_model_json_parse: missing model or data to parse!");
        return -1;
    }
    attributes = sj_object_get_value(data,"attributes");
    if (attributes)
    {
        positions = sj_object_get_value(attributes,"position");
        model->vertex_buffer = gf3d_model_json_parse_array_data(sj_object_get_value(positions,"array"),&model->vertex_count);
        positions = sj_object_get_value(attributes,"normal");
        model->normal_buffer = gf3d_model_json_parse_array_data(sj_object_get_value(positions,"array"),&model->normal_count);
    }
    return 0;
}

Model *gf3d_model_load_from_json_file(char *filename)
{
    Model *model;
    SJson *json;
    model = gf3d_model_get_by_filename(filename);
    if (model != NULL)return model;
    model = gf3d_model_new();
    if (!model)return NULL; // in case this fails
    // load the json
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load json data: %s",sj_get_error());
        gf3d_model_delete(model);
        return NULL;
    }
    //parse the json
    if (gf3d_model_json_parse(model,sj_object_get_value(json,"data")) != 0)
    {
        gf3d_model_delete(model);
        sj_free(json);
        return NULL;
    }
    //cleanup
    gf3d_line_cpy(model->filepath,filename);
    sj_free(json);
    return model;
}

/*eol@eof*/