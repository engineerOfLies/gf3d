#include "gf3d_model.h"
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
    return model;
}

/*eol@eof*/