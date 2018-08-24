#include "gf3d_pipeline.h"

#include "simple_logger.h"

typedef struct
{
    Uint32      maxPipelines;
    Pipeline   *pipelineList;
}PipelineManager;

static PipelineManager gf3d_pipeline = {0};

void gf3d_pipeline_close();

void gf3d_pipeline_init(Uint32 max_pipelines)
{
    if (max_pipelines == 0)
    {
        slog("cannot initialize zero pipelines");
        return;
    }
    gf3d_pipeline.pipelineList = (Pipeline *)gf3d_allocate_array(sizeof(Pipeline),max_pipelines);
    if (!gf3d_pipeline.pipelineList)
    {
        slog("failed to allocate pipeline manager");
        return;
    }
    gf3d_pipeline.maxPipelines = max_pipelines;
    atexit(gf3d_pipeline_close);
}

void gf3d_pipeline_close()
{
    int i;
    if (gf3d_pipeline.pipelineList != 0)
    {
        for (i = 0; i < gf3d_pipeline.maxPipelines; i++)
        {
            gf3d_pipeline_free(&gf3d_pipeline.pipelineList[i]);
        }
        free(gf3d_pipeline.pipelineList);
    }
    memset(&gf3d_pipeline,0,sizeof(PipelineManager));
}

Pipeline *gf3d_pipeline_new()
{
    int i;
    for (i = 0; i < gf3d_pipeline.maxPipelines; i++)
    {
        if (gf3d_pipeline.pipelineList[i].inUse)continue;
        gf3d_pipeline.pipelineList[i].inUse = true;
        return &gf3d_pipeline.pipelineList[i];
    }
    slog("no free pipelines");
    return NULL;
}

void gf3d_pipeline_free(Pipeline *pipe)
{
    if (!pipe)return;
    if (!pipe->inUse)return;
    
    memset(pipe,0,sizeof(Pipeline));
}

/*eol@eof*/
