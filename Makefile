IDIR=include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR=lib

# Probably don't need this
# probably should define a rule that 
# does all of this just like the odir macro
# FIXME: YOU NEED TO CHANGE THIS TO A LINUX COMPAT ARCHIVE
_LIBS=SDL.h SDL_main.h
# wanna prepend -l to each thing
# in here
LIBS=-lSDL -lSDL_main

#_DEPS=
DEPS= Bus.h Cart.h CPU.h Mapper.h MyTypes.h PPU.h

# Might need to add mYtypes here
_OBJ= Bus.o Cart.o CPU.o Main.o Mapper.o PPU.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
   $(CC) -c -o $@ $< $(CFLAGS)


debug $(OBJ):
    #foobar

release $(OBJ):
    #foobar

clean:
    #foobar