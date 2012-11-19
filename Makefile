# Generic Makefile for compiling a simple executable.

# only ONE of these three                                                       

# xorg      - for running in normal xwindows when you can't get to your PI :-o
# rpi       - uses xwindows to provide event handling
# rpi_noX   - get keyboard events from raw input, xwindows not needed

#PLATFORM=xorg
PLATFORM=rpi
#PLATFORM=rpi_noX

####

ifeq ($(PLATFORM),xorg)
    FLAGS= -D__FOR_XORG__ -ggdb -c  `pkg-config libpng --cflags` 
    LIBS=-lX11 -lEGL -lGLESv2 `pkg-config libpng --libs` -lm
endif

ifeq ($(PLATFORM),rpi)
    FLAGS=-D__FOR_RPi__ -c  `pkg-config libpng --cflags` -I/usr/include/freetype2
    FLAGS+= -I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads/
    LIBS=-lX11 -lGLESv2 -lEGL -lm -lbcm_host -lfreetype -L/opt/vc/lib `pkg-config libpng --libs`
endif

ifeq ($(PLATFORM),rpi_noX)
    FLAGS=-D__FOR_RPi_noX__ -c  `pkg-config libpng --cflags` -I/usr/include/freetype2
    FLAGS+= -I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads/
    LIBS=-lX11 -lGLESv2 -lEGL -lm -lbcm_host -lfreetype -L/opt/vc/lib `pkg-config libpng --libs`
endif


CC := gcc
SRCDIR := src
BUILDDIR := build
CFLAGS := $(FLAGS) -g -Wall
TARGET := testFreetypGlesRpi

SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEPS := $(OBJECTS:.o=.deps)

$(TARGET): $(OBJECTS)
	@echo " Linking...";  $(CC) $(LIBS) $^ -o $(TARGET) 

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " CC $<"; $(CC) $(CFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

clean:
	@echo " Cleaning..."; $(RM) -r $(BUILDDIR) $(TARGET)

-include $(DEPS)

.PHONY: clean

