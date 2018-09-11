#include "gf3d_types.h"

#include <string.h>
#include <stdio.h>

#include "simple_logger.h"

void *gf3d_allocate_array(size_t typeSize,size_t count)
{
    void *array;
    if (count <= 0)
    {
        slog("cannot allocate zero elements");
        return NULL;
    }
    if (typeSize <= 0)
    {
        slog("cannot initialize an array of elements with zero size");
        return NULL;
    }
    array = malloc(typeSize * count);
    if (!array)
    {
        slog("failed to allocation space for %i elements of size %i",count,typeSize);
        return NULL;
    }
    memset(array,0,typeSize*count);
    return array;
}

/*eol@eof*/
