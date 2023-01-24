# Makefile reference
# CC		C COMPILER
# CFLAGS	COMPILER FLAGS
# $@		EVERYTHING BEFORE THE COLON
# $<		FIRST ARGUMENT AFTER THE COLON
# $^		EVERYTHING AFTER THE COLON
# `			EVERYTHING INSIDE `` IS A CONSOLE COMMAND

# Variables
# SDL_FLAGS		ALL NECESSARY FLAGS TO INCLUDE SDL HEADERS AND LINK LIBRARIES
# ODIR			DIRECTORY TO STORE OBJECT FILES
# DEPS			MY HEADER FILES (DEPENDENCIES)
# OBJ			OBJECT FILES

# Rules
# nescle		DEFAULT RULE; BUILD WITH -O2
# nescle_debug	BUILD WITH -O0
# clean			CLEAN


#IDIR=include
CC=gcc
#CFLAGS=-I$(IDIR)
CFLAGS=-Wall -std=c99
#CFLAGS += -Wextra -pedantic
CFLAGS += -pipe -Werror=format-security -Werror=implicit-function-declaration
# Can degrade performance
#CFLAGS +=-fstack-protector -fstack-clash-protection

# Includes the SDL headers and links the libraries
SDL_FLAGS=`sdl2-config --cflags --libs`

ODIR=obj
#LDIR=lib

DEPS= Bus.h Cart.h CPU.h Mapper.h MyTypes.h PPU.h

_OBJ= Bus.o Cart.o CPU.o Main.o Mapper.o PPU.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

# Tells us to recompile each object file based on its 
# C file which depends on the defined dependencies
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

nescle: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -O2 $(SDL_FLAGS)

nescle_debug: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -O0 $(SDL_FLAGS) 

clean:
	rm -f $(OBJ) nescle_debug nescle

