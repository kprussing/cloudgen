# This is the makefile for cloudgen
# Type "make" to make it...

# If you use a C compiler other than gcc then probably the following
# three lines will need changing
CC = gcc
CFLAGS = -Wall -g
OPTFLAGS = -O3 --fast-math

# Specify where the header files are located
INCLUDES = -I$(HOME)/include -I/usr/local/include -I/opt/graphics/include

# Specify where the static libraries are located
LIBS = -L$(HOME)/lib -L/usr/local/lib -L/opt/graphics/lib

# FFTW library: note that you need version 2.x not version 3.x, and
# also there is some variation in the library names in different
# installations of FFTW
FFTWLIBS = -lm -lrfftw -lfftw -lc

# NetCDF library
NETCDFLIBS = -lnetcdf

OBJECTS = cloudgen_core.o cloudgen_layers.o \
	readconfig.o random.o nctools.o main.o
HEADERS = cloudgen.h readconfig.h random.h nctools.h version.h
PROGRAM = cloudgen

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJECTS) $(LIBS) $(FFTWLIBS) $(NETCDFLIBS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(PROGRAM)

clean-autosaves:
	rm -f *~

