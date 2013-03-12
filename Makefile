# Generic Makefile for compiling a simple executable.

# only ONE of these three                                                       

# xorg      - for running in normal xwindows when you can't get to your PI :-o
# rpi       - uses xwindows to provide event handling
# rpi_noX   - get keyboard events from raw input, xwindows not needed

#PLATFORM=xorg
#PLATFORM=rpi
PLATFORM=rpi_noX

####

ifeq ($(PLATFORM),xorg)
    FLAGS= -D__FOR_XORG__ -ggdb -c  `pkg-config libpng --cflags` `pkg-config freetype2 --cflags` -Isrc -Itest
    LIBS=-lX11 -lEGL -lGLESv2 `pkg-config libpng --libs` `pkg-config freetype2 --libs` -lm
endif

ifeq ($(PLATFORM),rpi)
    FLAGS=-D__FOR_RPi__ -c  `pkg-config libpng --cflags` -I/usr/include/freetype2 -Isrc -Itest
    FLAGS+= -I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads/
    LIBS=-lX11 -lGLESv2 -lEGL -lm -lbcm_host -lfreetype -L/opt/vc/lib `pkg-config libpng --libs`
endif

ifeq ($(PLATFORM),rpi_noX)
    FLAGS=-D__FOR_RPi_noX__ -c  `pkg-config libpng --cflags` -I/usr/include/freetype2 -Isrc -Itest
    FLAGS+= -I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads/
    LIBS=-lX11 -lGLESv2 -lEGL -lm -lbcm_host -lfreetype -L/opt/vc/lib `pkg-config libpng --libs`
endif

TARGET=testFreetypeGlesPi

CFLAGS += $(FLAGS)

OBJ=$(shell find src/*.c | sed 's/\(.*\.\)c/\1o/g' | sed 's/src\//build\//g')  
TESTOBJ=$(shell find test/*.c | sed 's/\(.*\.\)c/\1o/g' | sed 's/test\//build\//g')  

$(TARGET): $(OBJ) $(TESTOBJ)
	@echo " Linking...";  $(CC) $(LIBS) $^ -o $(TARGET) 

build/%.o: src/%.c
	@mkdir -p build
	@echo " CC $<"; $(CC) $(CFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

build/%.o: test/%.c
	@mkdir -p build
	@echo " CC $<"; $(CC) $(CFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

clean:
	@echo " Cleaning..."; $(RM) -r build $(TARGET)

-include $(DEPS)

.PHONY: clean

