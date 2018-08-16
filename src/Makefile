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

LIB_PATH = ../libs
STATIC_LIBS = libsj.a
VULKAN_LIB = /home/djkehoe/git/vulkan1.1.82.0/x86_64
LDFLAGS = -L$(VULKAN_LIB)/lib -L$(LIB_PATH) $(foreach d, $(STATIC_LIBS),$(LIB_PATH)/$d)

OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

INC_PATHS = ../include ../libs/include
INC_PARAMS =$(foreach d, $(INC_PATHS), -I$d)

SDL_CFLAGS = `sdl2-config --cflags` $(INC_PARAMS)
SDL_LDFLAGS = `sdl2-config --libs` -lSDL2_image -lpng -ljpeg -lz -lGL -lGLEW -lvulkan
LFLAGS = -g  -o ../$(PROJECT) -lm
CFLAGS = -g  -fPIC -Wall -pedantic -std=gnu99 -fgnu89-inline -Wno-unknown-pragmas -Wno-variadic-macros
# -ffast-math for relase version

DOXYGEN = doxygen

#
# Targets
#

$(PROJECT): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) $(LDFLAGS) $(SDL_LDFLAGS) 

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


