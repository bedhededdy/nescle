IDIR=include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR=lib

# Probably don't need this
#LIBS=-lm

#_DEPS=
#DEPS=

#_OBJ=
#OBJ=

#$(ODIR)/%.o: %.c $(DEPS)
#   $(CC) -c -o $@ $< $(CFLAGS)


debug:
    #foobar

release:
    #foobar

clean:
    #foobar