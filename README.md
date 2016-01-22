freetypeGlesRpi
===============

Port of freetype-gl for use on Raspberry Pi, GLESv2.

Freetype-gl is an opengl library that handles fonts by setting up a font atlas. The atlas consists of a single texture holding truetype glyphs, 
and a data structure to allow lookup for rendering. See https://code.google.com/p/freetype-gl/

To make this work on the rapberry pi, I took the basic setup code from https://github.com/chriscamacho/gles2framework, 
which sets up the opengles context (it works on standard linux, rpi running X and rpi command line, based on compile time settings)

