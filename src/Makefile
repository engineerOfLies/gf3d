##############################################################################
#
# The Linux-GCC Makefile
#
##############################################################################


#
# Compiler stuff -- adjust to your system.
#

# Linux
PROJECT = gf3d
CC      = gcc
#CC      = clang


LIB_LIST = ../gfc/libs/libgfc.a ../gfc/simple_json/libs/libsj.a ../gfc/simple_logger/libs/libsl.a 
DLIB_LIST = -L../../vulkan/1.1.108.0/x86_64/lib
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

INC_PATHS = ../include ../gfc/include ../gfc/simple_logger/include ../gfc/simple_json/include
INC_PARAMS =$(foreach d, $(INC_PATHS), -I$d)

SDL_CFLAGS = `sdl2-config --cflags` $(INC_PARAMS)
SDL_LDFLAGS = `sdl2-config --libs` -lSDL2_image -lpng -ljpeg -lz -lvulkan -lm
LFLAGS = -g  -o ../$(PROJECT) 
CFLAGS = -g  -fPIC -Wall -pedantic -std=gnu99 -fgnu89-inline -Wno-unknown-pragmas -Wno-variadic-macros
# -ffast-math for relase version

DOXYGEN = doxygen

#
# Targets
#

$(PROJECT): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) $(LDFLAGS) $(LIB_LIST) $(SDL_LDFLAGS) 

docs:
	$(DOXYGEN) doxygen.cfg

sources:
	echo (patsubst %.c,%.o,$(wildcard *.c)) > makefile.sources


makefile.dep: depend

depend:
	@touch makefile.dep
	@-rm makefile.dep
	@echo Creating dependencies.
	@for i in *.c; do $(CC) $(INC) -MM $(INC_PARAMS) $$i; done > makefile.dep
	@echo Done.

clean:
	rm *.o

count:
	wc -l *.c $(foreach d, $(INC_PATHS), $d/*.h) makefile

#
# Dependencies.
#

include makefile.dep

#
# The default rule.
#

.c.o:
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $<
